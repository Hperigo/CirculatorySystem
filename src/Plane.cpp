//
//  Plane.cpp
//  CirculatorySys
//
//  Created by Henrique on 11/03/17.
//
//

#include "Plane.hpp"


namespace csys {
    
    using namespace cinder;

    void Plane::appendPosition(const ci::vec2 &pos){
        
        
        float distance = glm::distance(lastPosition, pos);
        
        
        if(distance > 5){
            needsRedraw = true;
        }else{
            needsRedraw = false;
        }
        
        lastUpdateTime = std::time(nullptr);
        position = pos;
    }
    
    bool Plane::draw(){
        
        if(!needsRedraw){
            return false;
        }
        
        vec2 pos = getPosition();
        vec2 lastPos = getLastPosition();
        
        if(lastPos != vec2(0)){
            gl::drawLine(pos, lastPos);
        }
        
        lastPosition = position;
        positions.push_back(position);
        needsRedraw = false;
        
        return true;
    }
    
}// end of namespace