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
        void setup();
        
        
        bool appendData(const  ci::JsonTree& jsonData);
        std::shared_ptr<csys::Plane> getPlane(const std::string& string_key);
        
        
        std::map<std::string, csys::Plane>& getPlanes() { return mPlanes; };
        
    private:
        
        
        void insertPlaneToDB(const csys::Plane& plane);
        std::shared_ptr<csys::Plane>  getPlaneFromDB(const csys::Plane& plane);
        
        document planeToBson(const csys::Plane& plane);
        
        mongocxx::instance mMongoInstance{};
        mongocxx::database mDB;
        mongocxx::collection mCollection;
        mongocxx::client mMongoClient;
        
        
        // Software cache ----
        std::map<std::string, csys::Plane> mPlanes;
        
    };
    
}



#endif /* Database_hpp */
