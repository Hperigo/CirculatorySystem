//
//  Database.hpp
//  CirculatorySys
//
//  Created by Henrique on 14/03/17.
//
//

#ifndef Database_hpp
#define Database_hpp

#include <stdio.h>
#include <thread>
#include <functional>
#include "Plane.hpp"

// MongoDB
#include <bsoncxx/json.hpp>
#include <bsoncxx/builder/stream/array.hpp>

#include <mongocxx/instance.hpp>
#include <mongocxx/client.hpp>
#include <mongocxx/stdx.hpp>
#include <mongocxx/uri.hpp>

namespace csys {
    
    using bsoncxx::builder::stream::close_array;
    using bsoncxx::builder::stream::close_document;
    using bsoncxx::builder::stream::document;
    using bsoncxx::builder::stream::finalize;
    using bsoncxx::builder::stream::open_array;
    using bsoncxx::builder::stream::open_document;
    
    class Database{
        public:
        typedef std::vector< PlaneRef > DocContainer;

        struct QueryResult{
            
            std::function<void( DocContainer& cur )> fnFucntion;
            std::future<  DocContainer > future;
            std::shared_ptr< DocContainer > cursor;
            bool isDirty = false;
            
        };
        
        typedef std::shared_ptr<QueryResult> QueryResultRef;

        
    
        void setup();
        
        
        bool appendData(const  ci::JsonTree& jsonData);
        void queryEveryPlane();
        
        void queryEveryPlaneAsync();
        bool isQueryAvailable();

        csys::PlaneRef getPlane(const std::string& string_key);
        std::map<std::string, csys::PlaneRef>& getPlanes() { return mPlanes; };
        
        void update();
        void addQuery(const bsoncxx::document::value& doc, std::function<void( DocContainer& cur )> fn);
//        void addQuery(const document& doc, const document::options& opt, const std::function<void( const document& doc)>& fn);

        
    private:
        
        
        void insertPlaneToDB(const csys::PlaneRef& plane);
        
        csys::PlaneRef  getPlaneFromDB(const csys::Plane& plane);
        
        document planeToBson(const csys::PlaneRef& plane);
        
        mongocxx::instance mMongoInstance{};
        mongocxx::database mDB;
        mongocxx::collection mCollection;
        mongocxx::client mMongoClient;
        
        
        // Software cache ----
        std::map<std::string, csys::PlaneRef> mPlanes;
        
        
        // threading ----
    
        bool isMainQueryDone = false;
        std::mutex mQueryMutex;
        std::vector<QueryResultRef> mQueries;
        
    };
    
}



#endif /* Database_hpp */
