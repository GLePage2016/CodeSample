#ifndef PROCESS_COMMUNICATOR_H_
#define PROCESS_COMMUNICATOR_H_

#include "UTF8String.h"
#include "Reactor.h"
#include <map>
#include "FileReader.h"
#include "FileWriter.h"
#include "ReactorThread.h"

namespace rct
{
    class REACTOR_API ProcessCommunicator
    {
    public:
        typedef enum ProcessState
        {
            PCPROCESS_DEFAULT,
            PCPROCESS_INIT,
            PCPROCESS_RUNNING,
            PCPROCESS_FINISHED
        };

        typedef enum ProcessType
        {
            PCPROCESS_NOINPUT_NOOUTPUT,
            PCPROCESS_HASINPUT_NOOUTPUT,
            PCPROCESS_NOINPUT_HASOUTPUT,
            PCPROCESS_HASINPUT_HASOUTPUT
        };
    protected:

    protected:
#ifdef WIN32
        STARTUPINFO procStartupInfo_;
        PROCESS_INFORMATION procInformation_;
        SECURITY_ATTRIBUTES procSecurityAttributes_;
        HANDLE procOutputRead_;
        HANDLE procOutputWrite_;
        HANDLE procInputRead_;
        HANDLE procInputWrite_;
#else
        int procId_;
        int procStatus_;
#endif
        rct::FileReader procFileReader_;
        rct::FileWriter procFileWriter_;
        ProcessCommunicator::ProcessState procState_;
        ProcessCommunicator::ProcessType  procType_;
        void* procHandle_;
        rct::UTF8String procName_;
        rct::UTF8String procFileName_;
        rct::UTF8String procDirectory_;
        //! Proc output data or output filename
        rct::UTF8String procOutput_;
        //! Proc input data or input filename
        rct::UTF8String procInput_;
        std::vector<std::pair<std::wstring, std::wstring>> procArgs_;
        //! Threads for I/O processing
        boost::thread* procInputThread_;
        boost::thread* procOutputThread_;
        //! Time to wait for a synchronized process
        unsigned int procSyncWaitTime_;
        unsigned long procExitCode_;
        bool runAsync_;
        bool procInputFromFile_;
        bool procOutputToFile_;
    private:        
        bool createProcPipes(rct::UTF8String& errMsg);
        void createProcIOThreads();
        void setDirectoryAndFileName(const rct::UTF8String& procFileNameAbs);
        std::wstring generateArgList();
        friend void injectProcessInputData(ProcessCommunicator* pComm);
        friend void consumeProcessOutputData(ProcessCommunicator* pComm);
    public:
        ProcessCommunicator();
        virtual ~ProcessCommunicator();

        bool Init(
            const rct::UTF8String& procName,
            const rct::UTF8String& procFileNameAbs,
            unsigned int procSyncWaitTime,
            bool runAsync,
            bool forceInit);
        bool SetSync(bool syncFlag, unsigned int syncWait);
        bool SetProcessIO(
            const ProcessCommunicator::ProcessType& pType,
            const rct::UTF8String& pInput,
            const rct::UTF8String& pOutput,
            bool fileIn, bool fileOut,
            rct::UTF8String& errMsg);
        bool ClearArguments();
        bool AddArgument(
            const rct::UTF8String& argName, 
            const rct::UTF8String& argVal);
        bool Run(rct::UTF8String& errorMsg);
        bool Shutdown();
        bool Destroy();

        const rct::UTF8String& GetProcName() const
        {
            return(procName_);
        }

        const rct::UTF8String& GetProcFileName() const
        {
            return(procFileName_);
        }

        const rct::UTF8String& GetProcDirectory() const
        {
            return(procDirectory_);
        }

        const rct::UTF8String& GetProcOutput() const
        {
            return(procOutput_);
        }

        const rct::ProcessCommunicator::ProcessState& GetProcState() const
        {
            return(this->procState_);
        }

        unsigned int GetProcExitCode() const
        {
            return(this->procExitCode_);
        }
    };
}

#endif //PROCESS_COMMUNICATOR_H_