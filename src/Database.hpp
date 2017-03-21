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
        
        void insertPlane(const csys::Plane& plane);
        void getPlane();
       
        void getPlanes();
        
        
    private:
        
        document planeToBson(const csys::Plane& plane);
        
        mongocxx::instance mMongoInstance{};
        mongocxx::database mDB;
        mongocxx::collection mCollection;
        mongocxx::client mMongoClient;
        
    };
    
}



#endif /* Database_hpp */
