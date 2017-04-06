//
//  Database.cpp
//  CirculatorySys
//
//  Created by Henrique on 14/03/17.
//
//
//

#include "cinder/Timer.h"
#include "cinder/Thread.h"

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
            
            auto oldPos = found->second->getPosition();
            
            // 500km, distance threashold
            if( csys::geo::distanceLatLong(newPos, oldPos) < 100 ){
                return false;
            }
            
            
            mPlanes[id]->appendPosition( newPos );
            insertPlaneToDB(mPlanes[id]);
            
        }
        else{
            
            
//            auto plane = std::make_shared<csys::Plane>(id, newPos, std::make_shared<ci::JsonTree>( jsonData ) );
//            
//            mPlanes[id] = plane;
//            insertPlaneToDB(plane);
            
            
        }
        
        return true;
    }
    
    
    
    void Database::insertPlaneToDB(const csys::PlaneRef &plane){
        
        try{
            
            mongocxx::stdx::optional<bsoncxx::document::value> maybe_result = mCollection.find_one(bsoncxx::v_noabi::builder::stream::document{} << "id" <<  plane->getKey() << bsoncxx::v_noabi::builder::stream::finalize );
            
            if(maybe_result) { // plane already on DB
                
                std::int64_t  now = std::time(nullptr);
                document updateDoc{};
                updateDoc << "$set" << open_document << "lastUpdateTime" << now << close_document;
                updateDoc << "$push" << open_document << "positions" << open_document << "lat" << plane->getPosition().x << "long" << plane->getPosition().y << close_document << close_document;
                

                mCollection.update_one( document{} << "id" << plane->getKey() << finalize, updateDoc.view()); // search quer
                
                
            }else{ // create a  plane, should be an error?
                
               auto doc = planeToBson(plane);
                mCollection.insert_one(doc.view());
            }
            
            
        }catch(std::exception& e){
            std::cout << "error inserting plane" << e.what() << std::endl;
        }
        
        
    
    }
    
    
    document Database::planeToBson(const csys::PlaneRef &plane){
        
        
        // Todo:
        // make positions array insert full postion vector
        
        document doc {};
//        doc << bsoncxx::builder::concatenate( bsoncxx::from_json( plane->getInfo()->serialize() ) );

        std::int64_t now = std::time(nullptr);
        doc << "creationTime" << now;
        doc << "lastUpdateTime" << now;
        
        bsoncxx::builder::stream::array positions{};
        positions << open_document << "lat" << plane->getPosition().x << "long" << plane->getPosition().y << close_document;

        doc << "positions" << positions;
        
        return doc;
    }
    
    csys::PlaneRef Database::getPlane(const std::string& string_key){
        //
       ci::Timer t(true);
        std::lock_guard<std::mutex> lock(mQueryMutex);

        mongocxx::stdx::optional<bsoncxx::document::value> maybe_result = mCollection.find_one(bsoncxx::v_noabi::builder::stream::document{} << "id" <<  string_key << bsoncxx::v_noabi::builder::stream::finalize );
       
       
        if(maybe_result){
            
            auto plane = std::make_shared<csys::Plane>(string_key, maybe_result->view() );
            ci::app::console() << "query took: " << t.getSeconds() * 1000.0f << "ms" << std::endl;
    
            return plane;
        }else{
            return nullptr;
        }
    }

    void Database::queryEveryPlane(){

        
//        ci::Timer t(true);
//
//        mongocxx::options::find opt;
////        opt.limit(50000);
//        opt.sort( document{} << "creationTime" << 1 << finalize );
//
//        mongocxx::cursor cursor = mCollection.find(document{} << finalize, opt);
//        mPlanes.clear();
//        
//        for(auto doc : cursor) {
//            auto plane =  std::make_shared<csys::Plane>( doc["id"].get_utf8().value.to_string(), doc );
//            mPlanes[plane->getKey()] = plane;
//        }
//        

        
        
        addQuery( document{} << finalize , [&]( DocContainer& cursor ){
            
            
            mPlanes.clear();
            
            for(auto doc : cursor) {
                mPlanes[doc->getKey()] = doc;
            }
            
            isMainQueryDone = true;
            ci::app::console() << "query ended " << std::endl;
        });
        
    }
    
    
    void Database::queryEveryPlaneAsync(){
        
//        mQueryFuture = std::async( std::launch::async, [&]{
//            
//            ci::app::console() << "initializing thread";
//            
//            this->queryEveryPlane();
//            
//            ci::app::console() << " >>> done";
//            
//        });
        
        
        
    }
    
    bool Database::isQueryAvailable(){
        
        return isMainQueryDone;
    }
    
    void Database::update(){
        
        
        for(auto& r : mQueries){
            
            auto status = r->future.wait_for(std::chrono::microseconds(1) );
            
            if(status == std::future_status::ready){
                
                DocContainer d = r->future.get();
                
                r->fnFucntion( d );
                r->isDirty = true;
            }
            
        }
        
        
        for(auto it = mQueries.begin(); it != mQueries.end();){
            
            if( (*it)->isDirty ){
                mQueries.erase(it);
            }else{
                ++it;
            }
        }

    
    }
    
    void Database::addQuery( const bsoncxx::document::value& doc, std::function<void( DocContainer& cur )> fn)   {
    
        QueryResultRef result = std::make_shared< QueryResult> ();
        
        
        result->fnFucntion = fn;

        result->future = std::async(std::launch::async, [&, doc, result]{
            ci::ThreadSetup t;

            mongocxx::uri uri("mongodb://127.0.0.1:27017");
            auto client = mongocxx::client(uri);
            
            auto db  = client["csys"];
            auto colllection = db["Planes"];
            
            mongocxx::cursor cursor = colllection.find(doc.view());
            
            DocContainer docs;
            
            for(auto&& d : cursor){
                
              docs.push_back( std::make_shared<csys::Plane>( d["id"].get_utf8().value.to_string(), d ));
            }
            
            return docs;

        });
        
        
       mQueries.push_back(result);
    }
    
}