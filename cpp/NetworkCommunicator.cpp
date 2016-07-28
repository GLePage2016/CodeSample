#include "stdafx.h"
#include "Reactor.h"
#include "ReactorError.h"
#include "ReactorException.h"
#include "ReactorCriticalException.h"
#include "ExceptionHandler.h"
#include "NetworkCommunicator.h"
#include "Logger.h"
#include <boost/format.hpp>

namespace rct
{
    //!Raw function to escape hex pair values in a URL contained within a "C" string for purposes of URL encode/decode
    /*!Source: From libjingle's talk library
     * Formatted for readability
     */
    static int HexPairValue(const char * code) 
    {
        int value = 0;
        const char * pch = code;
        for (;;) 
        {
            int digit = *pch++;
            if (digit >= '0' && digit <= '9') 
            {
                value += digit - '0';
            }
            else if (digit >= 'A' && digit <= 'F') 
            {
                value += digit - 'A' + 10;
            }
            else if (digit >= 'a' && digit <= 'f') 
            {
                value += digit - 'a' + 10;
            }
            else 
            {
                return -1;
            }
            if (pch == code + 2)
                return value;
            value <<= 4;
        }
    }

    //!Raw function to decode a URL contained within a "C" string
    /*!Source: From libjingle's talk library
     * Formatted for readability
     */
    int UrlDecode(const char *source, char *dest)
    {
        char * start = dest;

        while (*source) 
        {
            switch (*source) 
            {
            case '+':
                *(dest++) = ' ';
                break;
            case '%':
                if (source[1] && source[2]) 
                {
                    int value = HexPairValue(source + 1);
                    if (value >= 0) 
                    {
                        *(dest++) = value;
                        source += 2;
                    }
                    else 
                    {
                        *dest++ = '?';
                    }
                }
                else 
                {
                    *dest++ = '?';
                }
                break;
            default:
                *dest++ = *source;
            }
            source++;
        }
  
        *dest = 0;
        return dest - start;
    }  

    //!Raw function to encode a URL contained within a "C" string
    /*!Source: From libjingle's talk library
     */
    int UrlEncode(const char *source, char *dest, unsigned max)  
    {
        static const char *digits = "0123456789ABCDEF";
        unsigned char ch;
        unsigned len = 0;
        char *start = dest;

        while (len < max - 4 && *source)
        {
            ch = (unsigned char)*source;
            if (*source == ' ') 
            {
                *dest++ = '+';
            }
            else if (isalnum(ch) || strchr("-_.!~*'()", ch)) 
            {
                *dest++ = *source;
            }
            else 
            {
                *dest++ = '%';
                *dest++ = digits[(ch >> 4) & 0x0F];
                *dest++ = digits[       ch & 0x0F];
            }  
            source++;
        }
        *dest = 0;
        return start - dest;
    }

    //!Static function to encode a URL string for NetworkCommunicator
    /*!Source: From libjingle's talk library
     * Formatted for readability and inclusion into NetworkCommunicator class
     */
    static rct::UTF8String URLEncodeString(const rct::UTF8String& toEncode)
    {
        std::string decoded = toEncode.nstr();
        const char * sz_decoded = decoded.c_str();
        size_t needed_length = decoded.length() * 3 + 3;
        char stackalloc[64];
        char * buf = needed_length > sizeof(stackalloc)/sizeof(*stackalloc) ?
        (char *)malloc(needed_length) : stackalloc;
        UrlEncode(decoded.c_str(), buf, needed_length);
        std::string result(buf);
        if (buf != stackalloc) 
        {
            free(buf);
        }
        return result;
    }

    //!Static function to decode a URL string
    /*!Source: From libjingle's talk library
     * Formatted for readability and inclusion into NetworkCommunicator class
     */
    static rct::UTF8String URLDecodeString(const rct::UTF8String& toDecode)
    {
        std::string encoded = toDecode.nstr();
        const char * sz_encoded = encoded.c_str();
        size_t needed_length = encoded.length();
        for (const char * pch = sz_encoded; *pch; pch++) 
        {
            if (*pch == '%')
                needed_length += 2;
        }
        needed_length += 10;
        char stackalloc[64];
        char * buf = needed_length > sizeof(stackalloc)/sizeof(*stackalloc) ?
            (char *)malloc(needed_length) : stackalloc;
        UrlDecode(encoded.c_str(), buf);
        std::string result(buf);
        if (buf != stackalloc) 
        {
            free(buf);
        }
        return result;
    }


    bool NetworkCommunicator::constructGetRequest(
        boost::asio::streambuf& data, 
        const rct::UTF8String& resourceName,
        const rct::UTF8String& acceptStr,
        const rct::UTF8String& connection,
        const rct::UTF8String& authToken)
    {
        if (!initialized_ || !connected_)return(false);
        if (!resourceName.isValid() || resourceName.isEmpty())return(false);
        if (!acceptStr.isValid() || acceptStr.isEmpty())return(false);
        if (!connection.isValid() || connection.isEmpty())return(false);

        //Construct GET request string
        std::ostream requestStream(&data);
        requestStream << rct::GetOp.nstr() << " ";
        requestStream << resourceName.nstr() << " ";
        requestStream << rct::HttpVersion.nstr() << "\r\n";
        //Append the Host section
        requestStream << "Host: " << tcpQuery_->host_name() << "\r\n";
        //Append the Accept section
        requestStream << "Accept: " << acceptStr.nstr() << "\r\n";
        if (authToken.isValid() && !authToken.isEmpty())
        {
            requestStream << "HTTP_AUTHORIZATION: " << authToken.nstr() << "\r\n";
        }
        requestStream << "Connection: " << connection.nstr() << "\r\n\r\n";
        return(true);
    }

    bool NetworkCommunicator::constructPostRequest(
        boost::asio::streambuf& data,
        const rct::UTF8String& path,
        size_t contentLength,
        const rct::UTF8String& contentType,
        const rct::UTF8String& authToken)
    {
        if (!initialized_ || !connected_)return(false);
        if (!path.isValid() || path.isEmpty())return(false);
        if (!contentType.isValid() || contentType.isEmpty())return(false);

        //Construct POST request string
        std::ostream requestStream(&data);
        requestStream << rct::PostOp.nstr() << " ";
        requestStream << path.nstr() << " ";
        requestStream << rct::HttpVersion.nstr() << "\r\n";
        requestStream << "Accept: */*\r\n";
        requestStream << "User-Agent: Mozilla/4.0\r\n";
        //Append content settings
        requestStream << "Content-Length: " << contentLength << "\r\n";
        if (authToken.isValid() && !authToken.isEmpty())
        {
            requestStream << "HTTP_AUTHORIZATION: " << authToken.nstr() << "\r\n";
        }
        requestStream << "Accept-Language: en-us\r\n";
        requestStream << "Accept-Encoding: gzip, deflate\r\n";
        requestStream << "Host: " << tcpQuery_->host_name() << "\r\n";
        requestStream << "Content-Type: " << contentType.nstr() << "\r\n\r\n\r\n";
        return(true);
    }



    bool NetworkCommunicator::Write(boost::asio::streambuf& data)
    {
        if (!initialized_ || !connected_)return(false);
        boost::system::error_code socketError;
        try
        {
            //Prepare the write stream
            std::ostream writeStream(&data);

            //Ensure the socket is not null
            if (tcpSocket_)
            {
                //Write the data into the socket
                size_t sizeWritten = boost::asio::write(*tcpSocket_, data, socketError);
                if (sizeWritten > 0)
                {
                    return(true);
                }
                else
                {
                    throw boost::system::system_error(socketError);
                }
            }
        }
        catch(boost::system::system_error& sysErr)
        {
            ExceptionHandler& exHandler = ExceptionHandlerSingleton::Instance();
            ReactorCriticalException rCex(ReactorError::ReactorErrorCode::RCT_ERR_NETWORK, sysErr);
            ReactorError& err = rCex.GetErrorData();
            err.SetClassName("NetworkCommunicator::Write");
            err.SetErrorDetails(__FILE__, __LINE__);
            exHandler.addCriticalException("Network client error occurred during write", rCex);
        }
        catch(std::runtime_error& rErr)
        {
            ExceptionHandler& exHandler = ExceptionHandlerSingleton::Instance();
            ReactorCriticalException rCex(ReactorError::ReactorErrorCode::RCT_ERR_NETWORK, rErr);
            ReactorError& err = rCex.GetErrorData();
            err.SetClassName("NetworkCommunicator::Write");
            err.SetErrorDetails(__FILE__, __LINE__);
            exHandler.addCriticalException("Network client error occurred during write", rCex);
        }
        catch(std::exception& rEx)
        {
            ExceptionHandler& exHandler = ExceptionHandlerSingleton::Instance();
            ReactorException rCex(ReactorError::ReactorErrorCode::RCT_ERR_NETWORK, rEx);
            ReactorError& err = rCex.GetErrorData();
            err.SetClassName("NetworkCommunicator::Write");
            err.SetErrorDetails(__FILE__, __LINE__);
            exHandler.addException("Network client error occurred during write", rCex);
        }
        return(false);
    }


    bool NetworkCommunicator::Read(boost::asio::streambuf& dataOut, const UTF8String& untilStr)
    {
        if (!initialized_ || !connected_)return(false);
        boost::system::error_code socketError;
        size_t readLength = 0, totalRead = 0;
        bool rt = false;
        std::stringstream strBuff;
        try
        {
            if (untilStr.isValid() && !untilStr.isEmpty())
            {
                readLength = boost::asio::read_until(*tcpSocket_, dataOut, untilStr.nstr(), socketError);
                totalRead = readLength;                
            }
            else
            {                
                do
                {
                    boost::asio::streambuf internalBuffer;
                    readLength = boost::asio::read(*tcpSocket_, internalBuffer, boost::asio::transfer_at_least(1), socketError);
                    totalRead += readLength;
                    strBuff << &internalBuffer;
                }
                while(readLength > 0 && socketError != boost::asio::error::eof);
            }
            if (totalRead > 0)
            {
                rt = true;
                std::ostream dataOutStream(&dataOut);
                dataOutStream << strBuff.str();                
            }
            else
            {
                throw boost::system::system_error(socketError);
            }
        }
        catch(boost::system::system_error& bEx)
        {
            ExceptionHandler& exHandler = ExceptionHandlerSingleton::Instance();
            ReactorCriticalException rCex(ReactorError::ReactorErrorCode::RCT_ERR_NETWORK, bEx);
            ReactorError& err = rCex.GetErrorData();
            err.SetClassName("NetworkCommunicator::Read");
            err.SetErrorDetails(__FILE__, __LINE__);
            exHandler.addCriticalException("Network client error occurred during read", rCex);                
        }
        catch(std::runtime_error& rErr)
        {
            ExceptionHandler& exHandler = ExceptionHandlerSingleton::Instance();
            ReactorCriticalException rCex(ReactorError::ReactorErrorCode::RCT_ERR_NETWORK, rErr);
            ReactorError& err = rCex.GetErrorData();
            err.SetClassName("NetworkCommunicator::Read");
            err.SetErrorDetails(__FILE__, __LINE__);
            exHandler.addCriticalException("Network client error occurred during read", rCex);
        }
        catch(std::exception& rEx)
        {
            ExceptionHandler& exHandler = ExceptionHandlerSingleton::Instance();
            ReactorException rCex(ReactorError::ReactorErrorCode::RCT_ERR_NETWORK, rEx);
            ReactorError& err = rCex.GetErrorData();
            err.SetClassName("NetworkCommunicator::Read");
            err.SetErrorDetails(__FILE__, __LINE__);
            exHandler.addException("Network client error occurred during read", rCex);
        }

        return(rt);
    }

    bool NetworkCommunicator::WriteAsync(boost::asio::streambuf& data, NetworkOpCompleteHandler writeHandler)
    {
        if (!initialized_ || !connected_)return(false);
        boost::system::error_code socketError;
        try
        {
            //Prepare the write stream
            std::ostream writeStream(&data);

            //Ensure the socket is not null
            if (tcpSocket_)
            {
                //Write the data into the socket
                size_t sizeWritten = boost::asio::write(*tcpSocket_, data, socketError);
                if (sizeWritten > 0)
                {
                    return(true);
                }
                else
                {
                    throw boost::system::system_error(socketError);
                }
            }
        }
        catch(boost::system::system_error& sysErr)
        {
            ExceptionHandler& exHandler = ExceptionHandlerSingleton::Instance();
            ReactorCriticalException rCex(ReactorError::ReactorErrorCode::RCT_ERR_NETWORK, sysErr);
            ReactorError& err = rCex.GetErrorData();
            err.SetClassName("NetworkCommunicator::WriteAsync");
            err.SetErrorDetails(__FILE__, __LINE__);
            exHandler.addCriticalException("Network client error occurred during async write", rCex);
        }
        catch(std::runtime_error& rErr)
        {
            ExceptionHandler& exHandler = ExceptionHandlerSingleton::Instance();
            ReactorCriticalException rCex(ReactorError::ReactorErrorCode::RCT_ERR_NETWORK, rErr);
            ReactorError& err = rCex.GetErrorData();
            err.SetClassName("NetworkCommunicator::Write");
            err.SetErrorDetails(__FILE__, __LINE__);
            exHandler.addCriticalException("Network client error occurred during async write", rCex);
        }
        catch(std::exception& rEx)
        {
            ExceptionHandler& exHandler = ExceptionHandlerSingleton::Instance();
            ReactorException rCex(ReactorError::ReactorErrorCode::RCT_ERR_NETWORK, rEx);
            ReactorError& err = rCex.GetErrorData();
            err.SetClassName("NetworkCommunicator::Write");
            err.SetErrorDetails(__FILE__, __LINE__);
            exHandler.addException("Network client error occurred during async write", rCex);
        }
        return(false);
    }

    bool NetworkCommunicator::ReadAsync(boost::asio::streambuf& dataOut, const UTF8String& untilStr, NetworkOpCompleteHandler readHandler)
    {
        if (!initialized_ || !connected_)return(false);
        boost::system::error_code socketError;
        size_t readLength = 0, totalRead = 0;
        bool rt = false;
        std::stringstream strBuff;
        try
        {
            if (untilStr.isValid() && !untilStr.isEmpty())
            {
                std::string oStr;
                readLength = boost::asio::read_until(*tcpSocket_, dataOut, untilStr.cnstr(oStr), socketError);
                totalRead = readLength;                
            }
            else
            {                
                do
                {
                    boost::asio::streambuf internalBuffer;
                    readLength = boost::asio::read(*tcpSocket_, internalBuffer, boost::asio::transfer_at_least(1), socketError);
                    totalRead += readLength;
                    strBuff << &internalBuffer;
                }
                while(readLength > 0 && socketError != boost::asio::error::eof);
            }
            if (totalRead > 0)
            {
                rt = true;
                std::ostream dataOutStream(&dataOut);
                dataOutStream << strBuff.str();                
            }
            else
            {
                throw boost::system::system_error(socketError);
            }
        }
        catch(boost::system::system_error& bEx)
        {
            ExceptionHandler& exHandler = ExceptionHandlerSingleton::Instance();
            ReactorCriticalException rCex(ReactorError::ReactorErrorCode::RCT_ERR_NETWORK, bEx);
            ReactorError& err = rCex.GetErrorData();
            err.SetClassName("NetworkCommunicator::ReadAsync");
            err.SetErrorDetails(__FILE__, __LINE__);
            exHandler.addCriticalException("Network client error occurred during async read", rCex);                
        }
        catch(std::runtime_error& rErr)
        {
            ExceptionHandler& exHandler = ExceptionHandlerSingleton::Instance();
            ReactorCriticalException rCex(ReactorError::ReactorErrorCode::RCT_ERR_NETWORK, rErr);
            ReactorError& err = rCex.GetErrorData();
            err.SetClassName("NetworkCommunicator::ReadAsync");
            err.SetErrorDetails(__FILE__, __LINE__);
            exHandler.addCriticalException("Network client error occurred during async read", rCex);
        }
        catch(std::exception& rEx)
        {
            ExceptionHandler& exHandler = ExceptionHandlerSingleton::Instance();
            ReactorException rCex(ReactorError::ReactorErrorCode::RCT_ERR_NETWORK, rEx);
            ReactorError& err = rCex.GetErrorData();
            err.SetClassName("NetworkCommunicator::ReadAsync");
            err.SetErrorDetails(__FILE__, __LINE__);
            exHandler.addException("Network client error occurred during async read", rCex);
        }
        return(rt);
    }

    void NetworkCommunicator::Destroy()
    {
        this->Close();
        if (tcpSocket_ != nullptr)
        {
            delete tcpSocket_;
        }
        tcpSocket_ = nullptr;
        if (tcpQuery_ != nullptr)
        {
            delete tcpQuery_;
        }
        tcpQuery_ = nullptr;
        initialized_ = false;
    }

    NetworkCommunicator::NetworkCommunicator(const NetworkClientType& cType) :
        ioService_(),
        tcpResolver_(ioService_),
        tcpQuery_(nullptr),
        tcpSocket_(nullptr),
        clientType_(cType),
        initialized_(false),
        connected_(false)
    {
    }

    NetworkCommunicator::~NetworkCommunicator()
    {
        Destroy();
    }

    bool NetworkCommunicator::Init(const rct::UTF8String& hostName, const rct::UTF8String& hostPort)
    {
        if (initialized_ && connected_)return(true);
        if (!hostName.isValid() || hostName.isEmpty() || !hostPort.isValid() || hostPort.isEmpty())return(false);
        //UTF8String host = boost::str(boost::format("%s:%s") % hostName.nstr() % hostPort.nstr());
        bool rt = false;
        try
        {
            tcpQuery_ = new boost::asio::ip::tcp::resolver::query(hostName.nstr(), hostPort.nstr());
            if (tcpQuery_)
            {
                boost::asio::ip::tcp::resolver::iterator iter = tcpResolver_.resolve(*tcpQuery_);
                initialized_ = true;
                tcpSocket_ = new boost::asio::ip::tcp::socket(ioService_);
                if (tcpSocket_)
                {
                    boost::asio::connect(*tcpSocket_, iter);
                    connected_ = true;
                }
            }
        }
        catch(boost::system::system_error& sysErr)
        {
            ExceptionHandler& exHandler = ExceptionHandlerSingleton::Instance();
            ReactorCriticalException rCex(ReactorError::ReactorErrorCode::RCT_ERR_NETWORK, sysErr);
            ReactorError& err = rCex.GetErrorData();
            err.SetClassName("NetworkCommunicator::Init");
            err.SetErrorDetails(__FILE__, __LINE__);
            exHandler.addCriticalException("Network client error occurred during connect", rCex);
        }
        catch(std::runtime_error& rErr)
        {
            ExceptionHandler& exHandler = ExceptionHandlerSingleton::Instance();
            ReactorCriticalException rCex(ReactorError::ReactorErrorCode::RCT_ERR_NETWORK, rErr);
            ReactorError& err = rCex.GetErrorData();
            err.SetClassName("NetworkCommunicator::Init");
            err.SetErrorDetails(__FILE__, __LINE__);
            exHandler.addCriticalException("Network client error occurred during connect", rCex);
        }
        catch(std::exception& rEx)
        {
            ExceptionHandler& exHandler = ExceptionHandlerSingleton::Instance();
            ReactorException rCex(ReactorError::ReactorErrorCode::RCT_ERR_NETWORK, rEx);
            ReactorError& err = rCex.GetErrorData();
            err.SetClassName("NetworkCommunicator::Init");
            err.SetErrorDetails(__FILE__, __LINE__);
            exHandler.addException("Network client error occurred during connect", rCex);
        }
        return(initialized_ && connected_);
    }

    /*bool NetworkCommunicator::Init(const boost::asio::ip::address& hostAddr, const rct::UTF8String& hostPort)
    {
        if (hostAddr.is_unspecified() || !hostPort.isValid() || hostPort.isEmpty())return(false);
        tcpQuery_ = new boost::asio::ip::tcp::resolver::query(         
            hostAddr,
            hostPort.nstr());
        boost::asio::ip::tcp::resolver::iterator iter = tcpResolver_.resolve(*tcpQuery_);
        tcpSocket_ = new boost::asio::ip::tcp::socket(ioService_);
        if (tcpSocket_)
        {
            tcpSocket_->connect(*iter);
        }
    }*/
    bool NetworkCommunicator::GetRequest(
        const rct::UTF8String& resourceName, 
        unsigned int& statusCode, 
        rct::UTF8String& responseData,
        const rct::UTF8String& authToken)
    {
        if (!initialized_ || !connected_)return(false);
        if (!resourceName.isValid() || resourceName.isEmpty())return(false);
        Logger& log = LoggerSingleton::Instance();

        //Construct GET request string
        boost::asio::streambuf request;
        if (!constructGetRequest(request, resourceName, "*/*", "close", authToken))
        {
            ExceptionHandler& exHandler = ExceptionHandlerSingleton::Instance();
            ReactorException rCex(ReactorError::ReactorErrorCode::RCT_ERR_NETWORK, std::exception("Network get construct failure"));
            ReactorError& err = rCex.GetErrorData();
            err.SetClassName("NetworkCommunicator::GetRequest");
            err.SetErrorDetails(__FILE__, __LINE__);
            exHandler.addException("Network client error occurred during GET construction", rCex);
            return(false);
        }

        if (log.GetSeverityCeiling() == Logger::SEV_DEBUG)
        {
            UTF8String msg = "Sending GET request for : ";
            log.LogDebug(msg + resourceName);
        }

        //Send the request
        if (!this->Write(request))
        {
            ExceptionHandler& exHandler = ExceptionHandlerSingleton::Instance();
            ReactorException rCex(ReactorError::ReactorErrorCode::RCT_ERR_NETWORK, std::exception("Network write request failure"));
            ReactorError& err = rCex.GetErrorData();
            err.SetClassName("NetworkCommunicator::GetRequest");
            err.SetErrorDetails(__FILE__, __LINE__);
            exHandler.addException("Network client error occurred during GET write", rCex);
            return(false);
        }

        if (log.GetSeverityCeiling() == Logger::SEV_DEBUG)
        {
            UTF8String msg = "Reading GET response for : ";
            log.LogDebug(msg + resourceName);
        }

        //Read the response status
        boost::asio::streambuf response;
        if (!this->Read(response, "\r\n"))
        {
            ExceptionHandler& exHandler = ExceptionHandlerSingleton::Instance();
            ReactorException rCex(ReactorError::ReactorErrorCode::RCT_ERR_NETWORK, std::exception("Network read response failure"));
            ReactorError& err = rCex.GetErrorData();
            err.SetClassName("NetworkCommunicator::GetRequest");
            err.SetErrorDetails(__FILE__, __LINE__);
            exHandler.addException("Network client error occurred during GET read", rCex);
            return(false);
        }

        if (log.GetSeverityCeiling() == Logger::SEV_DEBUG)
        {
            UTF8String msg = "Reading GET response status for : ";
            log.LogDebug(msg + resourceName);
        }

        //Verify the response 
        std::istream responseStream(&response);
        std::string httpVersion;
        responseStream >> httpVersion;
        if (!responseStream || httpVersion.substr(0, 5) != "HTTP/")
        {
            //Invalid response
            ExceptionHandler& exHandler = ExceptionHandlerSingleton::Instance();
            std::string errMsg = "Network read response - invalid HTTP header in response:" + httpVersion;
            ReactorException rCex(ReactorError::ReactorErrorCode::RCT_ERR_NETWORK, std::exception(errMsg.c_str()));
            ReactorError& err = rCex.GetErrorData();
            err.SetClassName("NetworkCommunicator::GetRequest");
            err.SetErrorDetails(__FILE__, __LINE__);
            exHandler.addException("Network client error occurred during GET read", rCex);
            return(false);
        }
        if (log.GetSeverityCeiling() == Logger::SEV_DEBUG)
        {
            UTF8String msg = "GET response http version: ";
            log.LogDebug(msg + httpVersion);
        }
        //Get response code
        responseStream >> statusCode;

        if (log.GetSeverityCeiling() == Logger::SEV_DEBUG)
        {
            UTF8String msg = "GET response HTTP status code: ";
            log.LogDebug(msg + boost::str(boost::format("%d") % statusCode));
        }
        std::string statusMessage;
        std::getline(responseStream, statusMessage);
        if (log.GetSeverityCeiling() == Logger::SEV_DEBUG)
        {
            if (!statusMessage.empty())
            {
                UTF8String msg = "Network status message from GET response: ";
                log.LogDebug(msg + statusMessage);
            }
        }

        if (!this->Read(response, "\r\n\r\n"))
        {
            ExceptionHandler& exHandler = ExceptionHandlerSingleton::Instance();
            ReactorException rCex(ReactorError::ReactorErrorCode::RCT_ERR_NETWORK, std::exception());
            ReactorError& err = rCex.GetErrorData();
            err.SetClassName("NetworkCommunicator::GetRequest");
            err.SetErrorDetails(__FILE__, __LINE__);
            exHandler.addException("Network client error occurred during GET response header read", rCex);
            return(false);
        }

        //Process remainder of response headers
        std::string header;
        std::stringstream headerResponseData;
        while(std::getline(responseStream, header) && header != "\r")
        {
            headerResponseData << header;
        }

        std::string responseDataStr = headerResponseData.str();
        if (log.GetSeverityCeiling() == Logger::SEV_DEBUG)
        {
            UTF8String msg = "Network read full GET header response: ";
            log.LogDebug(msg + responseDataStr);
        }

        //Write remaining response header stream
        if (response.size() > 0)
        {
            headerResponseData << "HeaderRemainder: " << &response;
            responseDataStr = headerResponseData.str();
            if (log.GetSeverityCeiling() == Logger::SEV_DEBUG)
            {
                UTF8String msg = "Network finish GET header response read remainder: ";
                log.LogDebug(msg + responseDataStr);
            }
        }

        std::stringstream getRequestDataStream;
        if (!this->Read(response))
        {
            ExceptionHandler& exHandler = ExceptionHandlerSingleton::Instance();
            ReactorException rCex(ReactorError::ReactorErrorCode::RCT_ERR_NETWORK, std::exception());
            ReactorError& err = rCex.GetErrorData();
            err.SetClassName("NetworkCommunicator::GetRequest");
            err.SetErrorDetails(__FILE__, __LINE__);
            exHandler.addException("Network client error occurred during GET response data read", rCex);
            return(false);
        }

        //Pull data sent to client
        if (response.size() > 0)
        {
            //Convert get data into string stream
            getRequestDataStream << &response;

            //Set response data
            responseData = getRequestDataStream.str();

            return(true);
        }
        return(false);
    }

    bool NetworkCommunicator::GetRequestAsync(
        const rct::UTF8String& resourceName,
        NetworkOpCompleteHandler getHandler)
    {
        return(false);
    }

    bool NetworkCommunicator::PostRequest(            
        const rct::UTF8String& path,
        const rct::UTF8String& dataToPost,
        size_t contentLength,
        const rct::UTF8String& contentType,
        unsigned int& statusCode,
        rct::UTF8String& responseHeader,
        rct::UTF8String& responseDataPackage,
        const rct::UTF8String& authToken)
    {
        if (!initialized_ || !connected_)return(false);
        if (!path.isValid() || path.isEmpty())return(false);
        if (!dataToPost.isValid() || dataToPost.isEmpty())return(false);
        if (!contentType.isValid() || contentType.isEmpty())return(false);
        if (contentLength <= 0)return(false);
        Logger& log = LoggerSingleton::Instance();

        //Construct POST request string
        boost::asio::streambuf request;
        if (!constructPostRequest(request, path, contentLength, contentType, authToken))
        {
            ExceptionHandler& exHandler = ExceptionHandlerSingleton::Instance();
            ReactorException rCex(ReactorError::ReactorErrorCode::RCT_ERR_NETWORK, std::exception("Network post construct failure"));
            ReactorError& err = rCex.GetErrorData();
            err.SetClassName("NetworkCommunicator::PostRequest");
            err.SetErrorDetails(__FILE__, __LINE__);
            exHandler.addException("Network client error occurred during POST construction", rCex);
            return(false);
        }

        if (log.GetSeverityCeiling() == Logger::SEV_DEBUG)
        {
            UTF8String msg = "Sending POST request to : ";
            log.LogDebug(msg + path);
        }

        //Send the post request
        if (!this->Write(request))
        {
            ExceptionHandler& exHandler = ExceptionHandlerSingleton::Instance();
            ReactorException rCex(ReactorError::ReactorErrorCode::RCT_ERR_NETWORK, std::exception("Network write request failure (POST header)"));
            ReactorError& err = rCex.GetErrorData();
            err.SetClassName("NetworkCommunicator::PostRequest");
            err.SetErrorDetails(__FILE__, __LINE__);
            exHandler.addException("Network client error occurred during POST header write", rCex);
            return(false);
        }

        if (log.GetSeverityCeiling() == Logger::SEV_DEBUG)
        {
            UTF8String msg = "Writing POST header data to : ";
            log.LogDebug(msg + path);
        }

        //Write the post data
        boost::asio::streambuf dataBuf;
        std::ostream dataStream(&dataBuf);
        dataStream << dataToPost.nstr() << "\r\n";
        if (!this->Write(dataBuf))
        {
            ExceptionHandler& exHandler = ExceptionHandlerSingleton::Instance();
            ReactorException rCex(ReactorError::ReactorErrorCode::RCT_ERR_NETWORK, std::exception("Network write request failure (POST data)"));
            ReactorError& err = rCex.GetErrorData();
            err.SetClassName("NetworkCommunicator::PostRequest");
            err.SetErrorDetails(__FILE__, __LINE__);
            exHandler.addException("Network client error occurred during POST data write", rCex);
            return(false);
        }
        
        //Read the response status
        boost::asio::streambuf response;
        if (!this->Read(response, "\r\n"))
        {
            ExceptionHandler& exHandler = ExceptionHandlerSingleton::Instance();
            ReactorException rCex(ReactorError::ReactorErrorCode::RCT_ERR_NETWORK, std::exception("Network read response failure (POST response header)"));
            ReactorError& err = rCex.GetErrorData();
            err.SetClassName("NetworkCommunicator::PostRequest");
            err.SetErrorDetails(__FILE__, __LINE__);
            exHandler.addException("Network client error occurred during POST response read", rCex);
            return(false);
        }

        if (log.GetSeverityCeiling() == Logger::SEV_DEBUG)
        {
            UTF8String msg = "Reading POST response status for : ";
            log.LogDebug(msg + path);
        }

        //Verify/read the post response
        std::istream responseStream(&response);
        std::string httpVersion;
        responseStream >> httpVersion;
        if (!responseStream)
        {
            //Invalid response
            ExceptionHandler& exHandler = ExceptionHandlerSingleton::Instance();
            std::string errMsg = "Network read response - invalid HTTP header in response:" + httpVersion;
            ReactorException rCex(ReactorError::ReactorErrorCode::RCT_ERR_NETWORK, std::exception(errMsg.c_str()));
            ReactorError& err = rCex.GetErrorData();
            err.SetClassName("NetworkCommunicator::PostRequest");
            err.SetErrorDetails(__FILE__, __LINE__);
            exHandler.addException("Network client error occurred during POST response read", rCex);
            return(false);
        }
        if (log.GetSeverityCeiling() == Logger::SEV_DEBUG)
        {
            UTF8String msg = "POST response http version: ";
            log.LogDebug(msg + httpVersion);
        }
        //Get response code
        responseStream >> statusCode;

        if (log.GetSeverityCeiling() == Logger::SEV_DEBUG)
        {
            UTF8String msg = "POST response HTTP status code: ";
            log.LogDebug(msg + boost::str(boost::format("%d") % statusCode));
        }
        std::string statusMessage;
        std::getline(responseStream, statusMessage);
        if (log.GetSeverityCeiling() == Logger::SEV_DEBUG)
        {
            if (!statusMessage.empty())
            {
                UTF8String msg = "Network status message from POST response: ";
                log.LogDebug(msg + statusMessage);
            }
        }

        if (!this->Read(response, "\r\n\r\n"))
        {
            ExceptionHandler& exHandler = ExceptionHandlerSingleton::Instance();
            ReactorException rCex(ReactorError::ReactorErrorCode::RCT_ERR_NETWORK, std::exception());
            ReactorError& err = rCex.GetErrorData();
            err.SetClassName("NetworkCommunicator::PostRequest");
            err.SetErrorDetails(__FILE__, __LINE__);
            exHandler.addException("Network client error occurred during POST response header read", rCex);
            return(false);
        }

        //Process remainder of response headers
        std::string header;
        std::stringstream headerResponseData;
        while(std::getline(responseStream, header) && header != "\r")
        {
            headerResponseData << header;
        }

        //std::string responseDataStr = headerResponseData.str();
        //Set response header
        responseHeader = headerResponseData.str();
        if (log.GetSeverityCeiling() == Logger::SEV_DEBUG)
        {
            UTF8String msg = "Network read full POST header response: ";
            log.LogDebug(msg + responseHeader);
        }

        //Write response data stream
        size_t responseRemainder = response.size();
        std::stringstream postData;
        std::string postDataStr;
        if (responseRemainder > 0)
        {
            //Put remaining response into post data
            postData << &response;

            //headerResponseData << "HeaderRemainder: " << &response;
            postDataStr = postData.str();
            if (log.GetSeverityCeiling() == Logger::SEV_DEBUG)
            {
                UTF8String msg = "Network finish POST header response read remainder: ";
                log.LogDebug(msg + responseDataPackage);
            }
            if (postDataStr.length() >= responseRemainder)
            {
                //Finished
                //Push response into outgoing data stream
                responseDataPackage = postDataStr;
                return(true);
            }
        }

        /*std::stringstream getRequestDataStream;
        if (!this->Read(response))
        {
            if (log.GetSeverityCeiling() == Logger::SEV_WARNING)
            {
                log.LogWarning("Network finish POST response read: Nothing remaining to read");
            }
            return(true);
        }

        if (response.size() > 0)
        {
            //Convert get data into string stream
            getRequestDataStream << &response;

            //Set response data
            responseData += UTF8String(getRequestDataStream.str());
        }*/

        return(true);
    }

    bool NetworkCommunicator::PostRequestAsync(
        const rct::UTF8String& resourceName,
        const rct::UTF8String& dataToPost,
        NetworkOpCompleteHandler postHandler)
    {
        return(false);
    }

    void NetworkCommunicator::Close()
    {
        if (initialized_ && connected_ && tcpSocket_ != nullptr)
        {
            try
            {
                if (connected_)
                {
                    tcpSocket_->close();
                    connected_ = false;
                }
            }
            catch(boost::exception& bEx)
            {
                ExceptionHandler& exHandler = ExceptionHandlerSingleton::Instance();
                ReactorCriticalException rCex(ReactorError::ReactorErrorCode::RCT_ERR_NETWORK, bEx);
                ReactorError& err = rCex.GetErrorData();
                err.SetClassName("NetworkCommunicator::Close");
                err.SetErrorDetails(__FILE__, __LINE__);
                exHandler.addCriticalException("Network client error occurred during close", rCex);
            }
            catch(std::runtime_error& rErr)
            {
                ExceptionHandler& exHandler = ExceptionHandlerSingleton::Instance();
                ReactorCriticalException rCex(ReactorError::ReactorErrorCode::RCT_ERR_NETWORK, rErr);
                ReactorError& err = rCex.GetErrorData();
                err.SetClassName("NetworkCommunicator::Close");
                err.SetErrorDetails(__FILE__, __LINE__);
                exHandler.addCriticalException("Network client error occurred during close", rCex);
            }
            catch(std::exception& eX)
            {
                ExceptionHandler& exHandler = ExceptionHandlerSingleton::Instance();
                ReactorException rCex(ReactorError::ReactorErrorCode::RCT_ERR_NETWORK, eX);
                ReactorError& err = rCex.GetErrorData();
                err.SetClassName("NetworkCommunicator::Close");
                err.SetErrorDetails(__FILE__, __LINE__);
                exHandler.addException("Network client error occurred during close", rCex);
            }
        }
    }
}
