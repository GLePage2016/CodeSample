#include "stdafx.h"
//#include <process.h>
#include "UTF8String.h"
#include "Reactor.h"
#include "ProcessCommunicator.h"
#include "FileUtilities.h"
#include <boost/asio.hpp>

namespace rct
{
bool ProcessCommunicator::createProcPipes(rct::UTF8String& errMsg)
{
    //Create the pipe that will consume output from the child  process
#ifdef WIN32

    //If the process produces output, create a pipe for the output
    if (this->procType_ == ProcessCommunicator::PCPROCESS_NOINPUT_HASOUTPUT ||
        this->procType_ == ProcessCommunicator::PCPROCESS_HASINPUT_HASOUTPUT)
    {
        if (!CreatePipe(&(this->procOutputRead_), &(this->procOutputWrite_), &(this->procSecurityAttributes_), 0))
        {
            errMsg = rct::Base::GenerateExitErrorMessage("CreatePipe - read/write of child process output");
            return(false);
        }

        //Ensure the output read handle is not inherited
        if (!SetHandleInformation(&(this->procOutputRead_), HANDLE_FLAG_INHERIT, 0))
        {
            errMsg = rct::Base::GenerateExitErrorMessage("SetHandleInformation - ensure child process output handle not inherited");
            return(false);
        }
    }

    //If the process consumes input, create a pipe for the input
    if (this->procType_ == ProcessCommunicator::PCPROCESS_HASINPUT_NOOUTPUT ||
        this->procType_ == ProcessCommunicator::PCPROCESS_HASINPUT_HASOUTPUT)
    {
        //Create the pipe that will inject input into the child process. 
        if (!CreatePipe(&(this->procInputRead_), &(this->procInputWrite_), &(this->procSecurityAttributes_), 0)) 
        {
            errMsg = rct::Base::GenerateExitErrorMessage("CreatePipe - read/write of child process input");
            return(false);
        }

        //Ensure the input write handle is not inherited
        if (!SetHandleInformation(&(this->procInputWrite_), HANDLE_FLAG_INHERIT, 0))
        {
            errMsg = rct::Base::GenerateExitErrorMessage("SetHandleInformation - ensure child process input handle not inherited");
            return(false);
        }
    }
#else
    //Other impl
#endif
    return(true);
}

void ProcessCommunicator::setDirectoryAndFileName(const UTF8String& procFileNameAbs)
{
    if (this->procState_ != ProcessCommunicator::PCPROCESS_DEFAULT)
    {
        return;
    }
    if (!rct::FileUtilities::ExtractFileNameAndPath(procFileNameAbs, this->procDirectory_, this->procFileName_))
    {
        this->procFileName_ = procFileNameAbs;
        this->procDirectory_ = rct::EmptyString;
    }
}

std::wstring ProcessCommunicator::generateArgList()
{
    std::wstringstream wSBuff;
    wSBuff << this->procDirectory_.str() << L"\\" << this->procFileName_.str() << L" ";
    if (!this->procArgs_.empty())
    {
        size_t len = procArgs_.size();
        for (size_t i = 0; i < len; ++i)
        {
            std::wstring argNm = this->procArgs_[i].first;
            std::wstring argVl = this->procArgs_[i].second;
            wSBuff << ((argNm.empty())?L"":argNm.c_str());
            wSBuff << ((argVl.empty())?L"":argVl.c_str());
            if (i < len-1)
            {
                wSBuff << L" ";
            }
        }
    }
    return(wSBuff.str());
}

void injectProcessInputData(ProcessCommunicator* pComm)
{
    //If the incoming object is null, or the communicator is in the wrong state, return immediately
    if (pComm == nullptr || pComm->procState_ != ProcessCommunicator::PCPROCESS_RUNNING)return;
    bool rt = false;
    if (pComm->procType_ == ProcessCommunicator::PCPROCESS_HASINPUT_NOOUTPUT ||
        pComm->procType_ == ProcessCommunicator::PCPROCESS_HASINPUT_HASOUTPUT)
    {
        //If input is file based, read from the file directly into the stdin of the child process
        if (pComm->procInputFromFile_)
        {
            rct::FileReader& fileReader = pComm->procFileReader_;
            if (fileReader.IsOpen())
            {
                HANDLE h = pComm->procInputWrite_;
                boost::iostreams::wfile_descriptor_sink fSink(h, boost::iostreams::never_close_handle);
                //Read the file input directly into the child input
                if (fileReader.Read(fSink, 0))
                {
                    rt = true;
                }
                fSink.close();
                fileReader.CloseFile();
            }
        }
        //If the input is a string, push the string data into the stdin of the child process
        else
        {
            //Take the procInput data and write it to the child process std in
            //Feed the child stdin write stream the proc input data
            HANDLE h = pComm->procInputWrite_;
            boost::iostreams::wfile_descriptor_sink sOut(h, boost::iostreams::never_close_handle);
            sOut.write(pComm->procInput_.c_str(), pComm->procInput_.length());
            sOut.close();
            rt = true;
        }
    }
}

void consumeProcessOutputData(ProcessCommunicator* pComm)
{
    //If the incoming object is null, return immediately
    if (pComm == nullptr || pComm->procState_ != ProcessCommunicator::PCPROCESS_RUNNING)return;
    bool rt = false;
    //Determine if we need to consume the output of the child process
    if (pComm->procType_ == ProcessCommunicator::PCPROCESS_NOINPUT_HASOUTPUT ||
        pComm->procType_ == ProcessCommunicator::PCPROCESS_HASINPUT_HASOUTPUT)
    {
        if (pComm->procOutputToFile_)
        {
            //If output is file based, read the output stream of the child process and write it directly into a file
            rct::FileWriter& fWriter = pComm->procFileWriter_;
            if (fWriter.IsOpen())
            {
                HANDLE h = pComm->procOutputRead_;
                boost::iostreams::wfile_descriptor_source fSource(h, boost::iostreams::never_close_handle);
                if (fWriter.Write(fSource, 0))
                {
                    rt = true;
                }
                fSource.close();
            }
            fWriter.CloseFile();
        }
        else
        {
            //Output is dumped into a string
            HANDLE h = pComm->procOutputRead_;
            boost::iostreams::wfile_descriptor_source sSource(h, boost::iostreams::never_close_handle);
            wchar_t readBuffer[256];
            memset(readBuffer, 0, sizeof(wchar_t) * 256);
            std::streamsize readSz = 0;
            std::streamsize totalAmt = 0;
            do
            {
                readSz = sSource.read(readBuffer, 256);
                if (readSz > 0)
                {
                    pComm->procOutput_ += readBuffer;
                    totalAmt += readSz;
                }
            }
            while(readSz > 0 && totalAmt < rct::MAX_WRITE_SIZE);

            //Close the stream source
            sSource.close();           
        }
    }
}

ProcessCommunicator::ProcessCommunicator() :
        procState_(ProcessCommunicator::PCPROCESS_DEFAULT),
        procType_(ProcessCommunicator::PCPROCESS_NOINPUT_NOOUTPUT),
        procName_(),
        procFileName_(),
        procDirectory_(),
        procOutput_(),
        procInput_(),
        procArgs_(),
        procInputThread_(nullptr),
        procOutputThread_(nullptr),
        procSyncWaitTime_(0),
        procExitCode_(0),
        runAsync_(false),
        procInputFromFile_(false),
        procOutputToFile_(false)
{
}

ProcessCommunicator::~ProcessCommunicator()
{
    this->Destroy();
}

bool ProcessCommunicator::Init(            
    const rct::UTF8String& procName,
    const rct::UTF8String& procFileNameAbs,
    unsigned int procSyncWaitTime,
    bool runAsync,
    bool forceInit)
{    
    if (this->procState_ != ProcessCommunicator::PCPROCESS_DEFAULT &&
        this->procState_ != ProcessCommunicator::PCPROCESS_FINISHED)
    {
        if (forceInit)
        {
            if (!this->Shutdown())
            {
                return(false);
            }
        }
        else
        {
            return(false);
        }
    }
    this->procName_ = procName;
    this->SetSync(!runAsync, procSyncWaitTime);
    this->setDirectoryAndFileName(procFileNameAbs);
    bool rt = false;
#ifdef WIN32
    this->procSecurityAttributes_.nLength = sizeof(SECURITY_ATTRIBUTES);
    this->procSecurityAttributes_.bInheritHandle = TRUE;
    this->procSecurityAttributes_.lpSecurityDescriptor = NULL;
#else
    //Other impl
#endif

    if (this->procFileName_.isValid() && !this->procFileName_.isEmpty())
    {
#ifdef WIN32
        memset(&(this->procStartupInfo_), 0, sizeof(this->procStartupInfo_));
        this->procStartupInfo_.cb = sizeof(this->procStartupInfo_);
#else
        this->procId_ = 1;
        this->procStatus_ = -1;
#endif
        this->procState_ = ProcessCommunicator::PCPROCESS_INIT;
        rt = true;
    }
    
    return(rt);
}

bool ProcessCommunicator::SetSync(bool syncFlag, unsigned int syncWait)
{
    if (procState_ == ProcessCommunicator::PCPROCESS_DEFAULT ||
        procState_ == ProcessCommunicator::PCPROCESS_INIT)
    {
        this->runAsync_ = !syncFlag;
        this->procSyncWaitTime_ = syncWait;
        if (!this->runAsync_ && this->procSyncWaitTime_ <= 0)
        {
            this->procSyncWaitTime_ = 0xffffffff;
        }
        return(true);
    }
    return(false);
}

bool ProcessCommunicator::SetProcessIO(
    const ProcessCommunicator::ProcessType& pType,
    const rct::UTF8String& pInput,
    const rct::UTF8String& pOutput,
    bool fileIn, bool fileOut,
    rct::UTF8String& errMsg)
{
    bool rt = false;
    if (procState_ == ProcessCommunicator::PCPROCESS_DEFAULT ||
        procState_ == ProcessCommunicator::PCPROCESS_INIT)
    {
        this->procType_ = pType;
        this->procInput_ = pInput;
        this->procOutput_ = pOutput;
        this->procInputFromFile_ = fileIn;
        this->procOutputToFile_ = fileOut;
        if (!this->createProcPipes(errMsg))
        {
            return(false);
        }

        //Create base directory buffer
        std::wstringstream sBuff;
        sBuff << this->procDirectory_.c_str();
        sBuff << L"\\";
        switch(this->procType_)
        {
        case ProcessCommunicator::PCPROCESS_NOINPUT_NOOUTPUT:  
            //No pipes needed
            rt = true;
            break;
        case ProcessCommunicator::PCPROCESS_NOINPUT_HASOUTPUT: 
            //If output file, setup file writer
            if (this->procOutputToFile_)
            {
                sBuff << this->procOutput_.c_str();
                rt = this->procFileWriter_.OpenFile(sBuff.str(), false, true);
            }
            break;
        case ProcessCommunicator::PCPROCESS_HASINPUT_NOOUTPUT: 
            //If input file, setup file reader
            if (this->procInputFromFile_)
            {
                sBuff << this->procInput_.c_str();
                rt = this->procFileReader_.OpenFile(sBuff.str());
            }
            break;
        case ProcessCommunicator::PCPROCESS_HASINPUT_HASOUTPUT:
            //If input file, setup file reader      
            rct::UTF8String baseName = sBuff.str();
            if (this->procInputFromFile_)
            {
                rct::UTF8String inName = baseName + this->procInput_;
                rt = this->procFileReader_.OpenFile(inName);
            }
            //If output file, setup file writer
            if (this->procOutputToFile_)
            {
                rct::UTF8String outName = baseName + this->procOutput_;
                rt = this->procFileWriter_.OpenFile(outName, false, true);
            }
            break;
        }
        return(rt);
    }
    return(false);
}

void ProcessCommunicator::createProcIOThreads()
{
    //Short-circuit the method if the process has not started
    if (this->procState_ != ProcessCommunicator::PCPROCESS_RUNNING)return;

    //Create thread to push input
    if (this->procType_ == ProcessCommunicator::PCPROCESS_HASINPUT_NOOUTPUT ||
        this->procType_ == ProcessCommunicator::PCPROCESS_HASINPUT_HASOUTPUT)
    {
        this->procInputThread_ = new boost::thread(&injectProcessInputData, this);
    }
    //Create thread to pull output
    if (this->procType_ == ProcessCommunicator::PCPROCESS_NOINPUT_HASOUTPUT ||
        this->procType_ == ProcessCommunicator::PCPROCESS_HASINPUT_HASOUTPUT)
    {
        this->procOutputThread_ = new boost::thread(&consumeProcessOutputData, this);
    }
}

bool ProcessCommunicator::ClearArguments()
{
    bool rt = false;
    if (procState_ == ProcessCommunicator::PCPROCESS_INIT ||
        procState_ == ProcessCommunicator::PCPROCESS_DEFAULT)
    {
        this->procArgs_.clear();
        rt = true;
    }
    return(rt);
}

bool ProcessCommunicator::AddArgument(
    const rct::UTF8String& argName, 
    const rct::UTF8String& argVal)
{
    bool rt = false;
    if (procState_ == ProcessCommunicator::PCPROCESS_INIT ||
        procState_ == ProcessCommunicator::PCPROCESS_DEFAULT)
    {
        if (argName.isValid() && argVal.isValid() && (!argName.isEmpty() || !argVal.isEmpty()))
        {
            this->procArgs_.push_back(
                std::pair<std::wstring, std::wstring>(argName.c_str(), argVal.c_str()));
            rt = true;
        }
    }
    return(rt);
}

bool ProcessCommunicator::Run(rct::UTF8String& errMessage)
{
    if (procState_ != ProcessCommunicator::PCPROCESS_INIT)
    {
        errMessage = "Cannot invoke run, process is not in the Init state";
        return(false);
    }
    bool rt = false;
    std::wstring errDetails;
    long errMsg = 0;
    this->procExitCode_ = 0;
    try
    {
#ifdef WIN32
        DWORD errCode=0;
        std::wstring genArgs = generateArgList();
        BOOL inheritHandles = FALSE;

        //Set the primary flag instructing the process to use the handles passed
        //in via the startup info structure
        if (this->procType_ != ProcessCommunicator::PCPROCESS_NOINPUT_NOOUTPUT)
        {
            this->procStartupInfo_.dwFlags |= STARTF_USESTDHANDLES;
            inheritHandles = TRUE;
        }

        //Ensure process I/O redirection handles are set properly into the proc startup info
        if (this->procType_ == ProcessCommunicator::PCPROCESS_HASINPUT_NOOUTPUT)
        {
            this->procStartupInfo_.hStdInput = this->procInputWrite_;
            this->procStartupInfo_.hStdError = GetStdHandle(STD_ERROR_HANDLE);
            this->procStartupInfo_.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
        }
        else if (this->procType_ == ProcessCommunicator::PCPROCESS_NOINPUT_HASOUTPUT)
        {
            this->procStartupInfo_.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
            this->procStartupInfo_.hStdError = this->procOutputRead_;
            this->procStartupInfo_.hStdOutput = this->procOutputRead_;
        }
        else if (this->procType_ == ProcessCommunicator::PCPROCESS_HASINPUT_HASOUTPUT)
        {
            this->procStartupInfo_.hStdInput = this->procInputWrite_;
            this->procStartupInfo_.hStdError = this->procOutputRead_;
            this->procStartupInfo_.hStdOutput = this->procOutputRead_;
        }

        this->procStartupInfo_.dwX = 0;
        this->procStartupInfo_.dwY = 0;       
        this->procStartupInfo_.wShowWindow = FALSE;
        BOOL ret = CreateProcess(
            NULL,
            &genArgs[0],
            0, 
            0, 
            inheritHandles,
            0, 
            0, 
            0,
            &procStartupInfo_,
            &procInformation_);
        if (ret == TRUE)
        {
            rt = true;
            //Acquire the process handle
            this->procHandle_ = static_cast<void*>(procInformation_.hProcess);
            procState_ = ProcessCommunicator::PCPROCESS_RUNNING;

            //Create input/output threads if necessary
            this->createProcIOThreads();
                
            if (!this->runAsync_)
            {
                //Running the child process in a synchronous fashion, block until completion or timeout
                DWORD res = WaitForSingleObject(procInformation_.hProcess, this->procSyncWaitTime_);
                procState_ = ProcessCommunicator::PCPROCESS_FINISHED;
                GetExitCodeProcess(procInformation_.hProcess, &procExitCode_);
                if (res == WAIT_ABANDONED)
                {
                    errDetails = L"Wait Abandoned While During Process";
                    errCode = 1;
                    rt = false;
                }
                else if (res == WAIT_OBJECT_0)
                {
                    errDetails = L"";
                    errCode = 0;
                    rt = true;
                }
                else if (res == WAIT_TIMEOUT)
                {
                    errDetails = L"Time-out interval has been exceeded";
                    errCode = 2;
                    rt = false;
                }
                else if (res == WAIT_FAILED)
                {
                    errDetails = L"Wait Has Failed";
                    errCode = GetLastError();
                    rt = false;
                }
            }
        }
        else
        {
            rt = false;
            errCode = GetLastError();
            errMsg = errCode;
        }
#else
    //Other platform impl
#endif
    }
    catch(std::runtime_error& rErr)
    {
        
    }
    catch(std::exception& rEx)
    {
        //TODO: Log and exception handler
    }

    if (!rt)
    {
        std::wstringstream sBuff;
        sBuff << errDetails << ": Internal error code: " << errMsg;
        errMessage = sBuff.str();
    }
    return(rt);
}

bool ProcessCommunicator::Shutdown()
{
    if (procState_ == ProcessCommunicator::PCPROCESS_DEFAULT)
    {
        return(false);
    }
    if (procState_ != ProcessCommunicator::PCPROCESS_FINISHED)
    {
        try
        {
            //Must kill the process
#ifdef WIN32
            if (!TerminateProcess(this->procInformation_.hProcess, 1))
            {
                return(false);
            }
#else
            //Other platform impl
#endif
            procState_ = ProcessCommunicator::PCPROCESS_FINISHED;        
            return(true);
        }
        catch(std::runtime_error& rErr)
        {
            //TODO: log and exception handler
        }
        catch(std::exception& rEx)
        {
            //TODO: log and exception handler
        }
        return(false);
    }

    return(true);
}

bool ProcessCommunicator::Destroy()
{
    try
    {
        if (procState_ == ProcessCommunicator::PCPROCESS_RUNNING)
        {
            this->Shutdown();
        }

#ifdef WIN32
        if (this->procInformation_.hProcess != NULL)
        {
            CloseHandle(this->procInformation_.hProcess);
        }
        if (this->procInformation_.hThread != NULL)
        {
            CloseHandle(this->procInformation_.hThread);
        }
#else
        //Other platform imp;
#endif
        procState_ = ProcessCommunicator::PCPROCESS_DEFAULT;
        return(true);
    }
    catch(std::runtime_error& rErr)
    {
        //TODO: log and exception handler
    }
    catch(std::exception& rEx)
    {
        //TODO: log and exception handler
    }
    return(false);
}

}