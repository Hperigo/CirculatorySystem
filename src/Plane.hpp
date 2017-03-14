//
//  Plane.hpp
//  CirculatorySys
//
//  Created by Henrique on 11/03/17.
//
//

#ifndef Plane_hpp
#define Plane_hpp

#include <stdio.h>
#include "cinder/Vector.h"


namespace csys {
    
    
    class Plane{
    public:
        Plane(){
            
        }
        
        Plane(const std::string& key, const ci::vec2& pos) : mKey(key), position(pos), lastPosition(pos){
            lastUpdateTime = std::time(nullptr);
        }
        
        void setPosition(const ci::vec2& pos);
        
        ci::vec2 getPosition() const{
            return position;
        }
        
        ci::vec2 getLastPosition() const {
            
            return lastPosition;
        }
        
        std::string getKey() const {
            return mKey;
        }
        
    
        
        bool draw();
        
        std::time_t getLastUpdateTime() const{
            return lastUpdateTime;
        };

    protected:
        bool needsRedraw = false;
        
        ci::vec2 position;
        ci::vec2 lastPosition;
        std::string mKey;
        
        std::time_t lastUpdateTime;
    };
    
}// end of namespace csys

#endif /* Plane_hpp */
