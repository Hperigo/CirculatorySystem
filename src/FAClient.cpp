//
//  FAClient.cpp
//  CirculatorySys
//
//  Created by Henrique on 05/03/17.
//
//

#include "FAClient.hpp"

#include <iostream>



namespace FlightAware {
    
    
    bool  FAClient::verifyCertificate(bool preverified, asio::ssl::verify_context &ctx){
        
        
        char subject_name[256];
        
        X509* cert = X509_STORE_CTX_get_current_cert(ctx.native_handle());
        X509_NAME_oneline(X509_get_subject_name(cert), subject_name, 256);
        
        
        std::cout << "Verifying " << subject_name << "\n";
        std::cout << "Preverified: " << preverified << "\n";
        return preverified;
        
    }
    
    //-----
    
    void FAClient::connect(){
        

        asio::async_connect(mSocket->lowest_layer(), mResolverIterator,
                            std::bind(&FAClient::handleConnect, this, std::placeholders::_1));
    }
    
    void FAClient::disconnect(){
        
        mSocket->shutdown();

    }
    
    // Callbacks ----
    
    
    void FAClient::handleConnect(const asio::error_code error){
        
        if(!error){
            std::cout << "Connected to FA" << std::endl;
            
            mSocket->async_handshake(asio::ssl::stream_base::client,
                                    std::bind(&FAClient::handleHandshake, this, std::placeholders::_1));
        }else{
            
            std::cout << "Connection failed: " << error.message() << std::endl;
        }
    }
    
    void FAClient::handleHandshake(const asio::error_code error){
        
        
        if(!error){

            asio::async_write(*mSocket, asio::buffer(mInitCommand.data(), mInitCommand.size()),
                              std::bind(&FAClient::handleWrite, this,
                                        std::placeholders::_1,
                                        std::placeholders::_2));
        }else{
            std::cout << "Handshake failed: " << error.message() << std::endl;
        }
        
    }
    
    void FAClient::handleWrite(const asio::error_code error, size_t bytes_transferred){

        
        if(!error){
            dispatchRead();
            
        }else{
            std::cout << "write failed: " << error.message() << std::endl;
        }
    }
    
    void FAClient::handleRead(const asio::error_code error, size_t bytes_transferred){
        
        if(!error){
            
        
                std::istream is(&buff);
                std::getline(is, mReply);
            
            
                if(onReplyCallback)
                    onReplyCallback(mReply);
            
                dispatchRead();
                mHasDataAvailable = true;

        }else{
            std::cout << "read failed: " << error.message() << std::endl;
        }
    
    }
    
    void FAClient::dispatchRead(){
        
        if(mKeepConnection){

            asio::async_read_until(*mSocket, buff, '\n', std::bind(&FAClient::handleRead, this, std::placeholders::_1, std::placeholders::_2));
        }

    }
    
    
}