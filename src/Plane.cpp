//
//  Plane.cpp
//  CirculatorySys
//
//  Created by Henrique on 11/03/17.
//
//

#include "Plane.hpp"
#include "GeoUtils.h"

namespace csys {
    
    using namespace cinder;

    void Plane::appendPosition(const ci::vec2 &pos){
        
        lastUpdateTime = std::time(nullptr);
        needsRedraw = true;
        position = pos;
        
    }
    
    bool Plane::draw(){
        
        if(!needsRedraw){
            return false;
        }
        
        vec2 pos = getPosition();
        vec2 lastPos = getLastPosition();
        
        if(lastPos != vec2(0)){
            
            auto pointA = csys::geo::latLongToCartesian( { 1200, 600} , pos);
            auto pointB = csys::geo::latLongToCartesian( { 1200, 600} ,lastPos);
            
            if( glm::distance( pointA, pointB) < 500 )
                gl::drawLine(pointA, pointB);
        }
        
        lastPosition = position;
        positions.push_back(position);
        needsRedraw = false;
        
        return true;
    }
    
}// end of namespace