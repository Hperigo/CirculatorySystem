//
//  Database.cpp
//  CirculatorySys
//
//  Created by Henrique on 14/03/17.
//
//

#include "Database.hpp"


namespace csys {
    
    
    void Database::setup(){
        
        mongocxx::uri uri("mongodb://127.0.0.1:27017");
        mMongoClient = mongocxx::client(uri);
        
        try {
        
        mDB = mMongoClient["csys"];
        mCollection = mDB["Planes"];
            
            
        } catch (std::exception& e) {
            std::cout << "error: " << e.what() << std::endl;
        }
        
        
    }
    
    
    void Database::insertPlane(const csys::Plane &plane){
        
        try{
            
            mongocxx::stdx::optional<bsoncxx::document::value> maybe_result = mCollection.find_one(bsoncxx::v_noabi::builder::stream::document{} << "id" <<  plane.getKey() << bsoncxx::v_noabi::builder::stream::finalize );
            
            if(maybe_result) { // plane already on DB
                
                std::int64_t  now = std::time(nullptr);
                
                bsoncxx::document::element element = maybe_result->view()["lastUpdateTime"];
                
                std::int64_t diff = now - element.get_int64();
                if( diff < 400 ){
                    return;
                }
                else{
                    
                }

                document updateDoc{};
                updateDoc << "$set" << open_document << "lastUpdateTime" << now << close_document;
                updateDoc << "$push" << open_document << "positions" << open_document << "lat" << plane.getPosition().x << "long" << plane.getPosition().y << close_document << close_document;
                

                mCollection.update_one( document{} << "id" << plane.getKey() << finalize, updateDoc.view()); // search quer
                
                
            }else{ // create a  plane
                
               auto doc = planeToBson(plane);
                mCollection.insert_one(doc.view());
            }
            
            
        }catch(std::exception& e){
            std::cout << "error inserting plane" << e.what() << std::endl;
        }
        
        
    
    }
    
    
    document Database::planeToBson(const csys::Plane &plane){
        
        
        
        document doc {};
        doc << bsoncxx::builder::concatenate( bsoncxx::from_json( plane.getInfo()->serialize() ) );
        
        
        std::int64_t now = std::time(nullptr);
        doc << "creationTime" << now;
        doc << "lastUpdateTime" << now;
        
        bsoncxx::builder::stream::array positions{};
        positions << open_document << "lat" << plane.getPosition().x << "long" << plane.getPosition().y << close_document;

        doc << "positions" << positions;
        
        return doc;
    }
    
    void Database::getPlane(){
        
        
    }

}