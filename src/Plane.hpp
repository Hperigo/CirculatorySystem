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
#include "cinder/Json.h"

namespace csys {
    
    
    class Plane{
    public:
        Plane(){
            
        }
        
        Plane(const std::string& key, const ci::vec2& pos, const std::shared_ptr< ci::JsonTree >& info) : mKey(key), position(pos), lastPosition(pos), mJsonInfo(info){
            lastUpdateTime = std::time(nullptr);
        }
        
        Plane(const std::string& key, const std::shared_ptr< ci::JsonTree >& info) : mKey(key), mJsonInfo(info){
            
            
            auto positionArray = (*mJsonInfo)["positions"];
            
            for(auto& child : positionArray.getChildren()){
            
//                std::cout << "x: " << child["lat"].getValue<float>() << "\n";
//                std::cout << "y: " << child["long"].getValue<float>()<< "\n";
                
                
                float x = child["lat"].getValue<float>();
                float y = child["long"].getValue<float>();
                
                positions.push_back({x, y});
                
            }
            
            
        }
        
        
        void appendPosition(const ci::vec2& pos);
        
        ci::vec2 getPosition() const{
            return position;
        }
        
        ci::vec2 getLastPosition() const {
            
            return lastPosition;
        }
        
        std::string getKey() const {
            return mKey;
        }
        
        std::time_t getLastUpdateTime() const{
            return lastUpdateTime;
        };
        
        std::vector<ci::vec2> getPositions() const{
            return positions;
        }
        
        
        std::shared_ptr< ci::JsonTree > getInfo()const {
            return mJsonInfo;
        }
        
        bool draw();
        
        
    protected:
        bool needsRedraw = false;
        
        ci::vec2 position;
        ci::vec2 lastPosition;
        
        // infos
        std::time_t lastUpdateTime;
        std::string mKey;
        std::shared_ptr< ci::JsonTree > mJsonInfo;
        
        std::vector<ci::vec2> positions;

    };
    
}// end of namespace csys

#endif /* Plane_hpp */
