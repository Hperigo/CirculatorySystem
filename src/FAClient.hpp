//
//  FAClient.hpp
//  CirculatorySys
//
//  Created by Henrique on 05/03/17.
//
//

#ifndef FAClient_hpp
#define FAClient_hpp

#include <stdio.h>

#include "asio/asio.hpp"
#include "asio/ssl.hpp"

namespace FlightAware{
    class FAClient {
        
    public:
        
        class Settings{
        public:
            Settings(asio::io_service& io_service) : mIoServiceRef( io_service ){
                
                mServer = "firehose.flightaware.com";
                mPort   = "1501";
                
            }
            
            Settings& setServer(const std::string& server){ mServer = server; return *this; }
            
            Settings& setPort(const std::string& port){ mPort = port; return *this; }
            
            Settings& setInitCommand(const std::string& initCommand){ mInitCommand = initCommand; return *this; }
            
            std::string getServer() const { return mServer; }
            std::string getPort() const { return mPort; }
            std::string getInitCommand() const { return mInitCommand; }
            
            
            asio::io_service& getIoService()  const {return mIoServiceRef; }
            
        private:
            std::string mServer="";
            std::string mPort="";
            std::string mInitCommand="";
            
            // Todo: make this a shared ptr?
            asio::io_service& mIoServiceRef;
            
        };
        

        
        FAClient(const Settings& settings) : mSettings(settings){
            
            
 
        }
        
        bool hasDataAvailable(){
            // TODO: make this return a size of vector containing the data
            auto has = mHasDataAvailable;
            mHasDataAvailable = false;
            return has;
        }
        
        
        
    public:
        void connect();
        void disconnect();
        
        
        //TODO: make getters
        std::function<void(const std::string&)> onReplyCallback;
        std::function<void(const std::string&)> onErrorCallback;
        
        
    private:
        
        // Callbacks ------
        
        bool verifyCertificate(bool preverified, asio::ssl::verify_context& ctx);
        
        void handleConnect(const  asio::error_code error );
        
        void handleHandshake(const  asio::error_code error);
        
        void handleWrite(const  asio::error_code error, size_t bytes_transferred);
        
        void handleRead(const  asio::error_code error, size_t bytes_transferred);
        
        void dispatchRead();
        
        
        std::shared_ptr< asio::ssl::stream<asio::ip::tcp::socket>> mSocket;
    
        asio::ip::tcp::resolver::iterator mResolverIterator;
        std::shared_ptr<asio::ip::tcp::resolver::query> mQuery;
        
        
        Settings mSettings;
        std::string mInitCommand;
        
        
        std::string mReply;
        std::vector<std::string> mAvailableData;
        asio::streambuf buff;

        bool mKeepConnection = true;
        bool mHasDataAvailable = false;
        
        
    };
}



#endif /* FAClient_hpp */
