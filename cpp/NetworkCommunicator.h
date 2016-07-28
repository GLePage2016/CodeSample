#ifndef NETWORK_CLIENT_H_
#define NETWORK_CLIENT_H_

#include "UTF8String.h"
#include "Reactor.h"
#include <boost/asio.hpp>

namespace rct 
{
    static UTF8String HttpVersion = "HTTP/1.0";
    static UTF8String GetOp = "GET";
    static UTF8String PostOp = "POST";


    typedef boost::function<bool (UTF8String&, unsigned int&)> NetworkOpCompleteHandler;

    class REACTOR_API NetworkCommunicator
    {
    public:
        typedef enum NetworkClientType
        {
            NETWORK_CLIENT_TCP,
            NETWORK_CLIENT_UDP            
        };
    public:
        static rct::UTF8String URLEncodeString(const rct::UTF8String& toEncode);
        static rct::UTF8String URLDecodeString(const rct::UTF8String& toDecode);
    private:
        boost::asio::io_service ioService_;
        boost::asio::ip::tcp::resolver tcpResolver_;
        boost::asio::ip::tcp::resolver::query* tcpQuery_;
        boost::asio::ip::tcp::socket* tcpSocket_;
        NetworkClientType clientType_;
        bool initialized_;
        bool connected_;

    private:
        bool constructGetRequest(
            boost::asio::streambuf& data, 
            const rct::UTF8String& resourceName,
            const rct::UTF8String& acceptStr,
            const rct::UTF8String& connection,
            const rct::UTF8String& authToken = rct::EmptyString);
        bool constructPostRequest(
            boost::asio::streambuf& data,
            const rct::UTF8String& path,
            size_t contentLength,
            const rct::UTF8String& contentType,
            const rct::UTF8String& authToken = rct::EmptyString);
        bool Write(boost::asio::streambuf& data);
        bool Read(boost::asio::streambuf& dataOut, const UTF8String& untilStr = rct::EmptyString);

        bool WriteAsync(boost::asio::streambuf& data, NetworkOpCompleteHandler writeHandler);
        bool ReadAsync(boost::asio::streambuf& dataOut, const UTF8String& untilStr, NetworkOpCompleteHandler readHandler);

        void Destroy();
        
    public:
        NetworkCommunicator(const NetworkClientType& cType);
        virtual ~NetworkCommunicator();

        bool Init(const rct::UTF8String& hostName, const rct::UTF8String& hostPort);
        //bool Init(const boost::asio::ip::address& hostAddr, const rct::UTF8String& hostPort);

        bool GetRequest(
            const rct::UTF8String& resourceName, 
            unsigned int& statusCode, 
            rct::UTF8String& responseData,
            const rct::UTF8String& authToken = rct::EmptyString);

        bool GetRequestAsync(
            const rct::UTF8String& resourceName,
            NetworkOpCompleteHandler getHandler);

        bool PostRequest(
            const rct::UTF8String& path,
            const rct::UTF8String& dataToPost,
            size_t contentLength,
            const rct::UTF8String& contentType,
            unsigned int& statusCode,
            rct::UTF8String& responseHeader,
            rct::UTF8String& responseDataPackage,
            const rct::UTF8String& authToken = rct::EmptyString);

        bool PostRequestAsync(
            const rct::UTF8String& resourceName,
            const rct::UTF8String& dataToPost,
            NetworkOpCompleteHandler postHandler);

        

        void Close();
    };
}

#endif //NETWORK_CLIENT_H_