//
//  GeoUtils.hpp
//  CirculatorySys
//
//  Created by Henrique on 21/03/17.
//
//

#ifndef GeoUtils_h
#define GeoUtils_h

#include "cinder/Vector.h"

namespace csys { namespace geo {
    
    
   inline ci::vec2 latLongToCartesian(ci::vec2 size, ci::vec2 point){
       
       
       
        float width = size.x;
        float height = size.y;
        
        float lat = point.x;
        float lon = point.y;
        
       
       auto y = (((lat * -1) + 90) * (height/ 180));
       auto x = ((lon + 180) * (width  / 360));
       
        return ci::vec2(x,y);
    }
    
   inline ci::vec2 cartesianToLatLong(ci::vec2 size, ci::vec2 coord){
        
        
        float width = size.x;
        float height = size.y;
        
        ci::vec2 latLong;
       
        latLong.x = (coord.y - height  * 0.5) / height  * -180;
        latLong.y = (coord.x - width * 0.5) / width * 360;

        return latLong;
    }
    
    
   inline double distanceLatLong(ci::vec2 coordA, ci::vec2 coordB){
        
        const double R = 6371e3; // metres
        double latA = ci::toRadians(coordA.x); //lat1.toRadians();
        double latB = ci::toRadians(coordB.x);
        double deltaLat = ci::toRadians(coordB.x-coordA.x);
        double deltaLong = ci::toRadians( coordB.y -coordA.y );
        
        
        double a = sin( deltaLat /2.0) * sin( deltaLat /2.0) +
        cos(latA) * cos(latB) *
        sin(deltaLong/2.0f) * sin(deltaLong/2.0);
       
        double c = 2.0 *  atan2(sqrt(a), sqrt(1.0 - a));
        double d = R * c;
       
        return d * 0.001; // in km
    }
    
    
    inline ci::vec2 getTerminatorCoords(float lat, float lon, float gamma){
        
        ci::vec2 terminator;

        terminator.x = glm::atan(gamma * std::sin( glm::radians(lat - lon ) )) * (180.0f / M_PI);
        terminator.y = lat;
        
        return terminator;
    }
    
} }

#endif /* GeoUtils_hpp */
