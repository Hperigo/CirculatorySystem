//
//  Database.cpp
//  CirculatorySys
//
//  Created by Henrique on 14/03/17.
//
//

#include "Database.hpp"

#include "GeoUtils.h"

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
    
    
    
    bool Database::appendData(const ci::JsonTree& jsonData)
    {
        
        auto FA_type = jsonData["type"].getValue<std::string>();
        if(FA_type != "position"){
            return false;
        }
        
        
        auto id = jsonData["id"].getValue<std::string>();
        
        ci::vec2 newPos;
        newPos.x = atof( jsonData["lat"].getValue<std::string>().c_str() ) ;
        newPos.y = atof( jsonData["lon"].getValue<std::string>().c_str() ) ;
        

        
        auto found = mPlanes.find( id );

        if(found != mPlanes.end() ){
            
            auto oldPos = found->second.getPosition();
            
            // 500km, distance threashold
            if( csys::geo::distanceLatLong(newPos, oldPos) < 100 ){
                return false;
            }
            
            
            mPlanes[id].appendPosition( newPos );
            insertPlaneToDB(mPlanes[id]);
            
        }
        else{
            
            
            auto plane = csys::Plane(id, newPos, std::make_shared<ci::JsonTree>( jsonData ) );
            mPlanes[id] = plane;
            insertPlaneToDB(plane);
            
            
        }
        
        return true;
    }
    
    
    
    void Database::insertPlaneToDB(const csys::Plane &plane){
        
        try{
            
            mongocxx::stdx::optional<bsoncxx::document::value> maybe_result = mCollection.find_one(bsoncxx::v_noabi::builder::stream::document{} << "id" <<  plane.getKey() << bsoncxx::v_noabi::builder::stream::finalize );
            
            if(maybe_result) { // plane already on DB
                
                std::int64_t  now = std::time(nullptr);
                document updateDoc{};
                updateDoc << "$set" << open_document << "lastUpdateTime" << now << close_document;
                updateDoc << "$push" << open_document << "positions" << open_document << "lat" << plane.getPosition().x << "long" << plane.getPosition().y << close_document << close_document;
                

                mCollection.update_one( document{} << "id" << plane.getKey() << finalize, updateDoc.view()); // search quer
                
                
            }else{ // create a  plane, should be an error?
                
               auto doc = planeToBson(plane);
                mCollection.insert_one(doc.view());
            }
            
            
        }catch(std::exception& e){
            std::cout << "error inserting plane" << e.what() << std::endl;
        }
        
        
    
    }
    
    
    document Database::planeToBson(const csys::Plane &plane){
        
        
        // Todo:
        // make positions array insert full postion vector
        
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
    
   std::shared_ptr<csys::Plane> Database::getPlane(const std::string& string_key){
        
        mongocxx::stdx::optional<bsoncxx::document::value> maybe_result = mCollection.find_one(bsoncxx::v_noabi::builder::stream::document{} << "id" <<  string_key << bsoncxx::v_noabi::builder::stream::finalize );
        
        if(maybe_result){
            
            std::cout << "Found plane!\n";
            
            auto json = std::make_shared< ci::JsonTree >( bsoncxx::to_json( maybe_result->view() ) );
            
            auto plane = std::make_shared<csys::Plane>(string_key, json );
            
            return plane;
            
        }else{
            return nullptr;
        }
        
    }

}