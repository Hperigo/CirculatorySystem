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

#include <bsoncxx/json.hpp>
#include <bsoncxx/builder/stream/array.hpp>


namespace csys {
    
    class PlaneManager;
    typedef std::shared_ptr<class Plane> PlaneRef;
    
    class Plane{
    public:
        Plane(){
            
        }
        
        Plane(const std::string& key, const ci::vec2& pos, const std::shared_ptr< ci::JsonTree >& info) : mKey(key), position(pos), lastPosition(pos){
            lastUpdateTime = std::time(nullptr);
            creationTime = std::time(nullptr);
        }
        
        Plane(const std::string& key, const bsoncxx::document::view& doc) : mKey(key){
            
            
            creationTime = doc["creationTime"].get_int64();
            lastUpdateTime = doc["lastUpdateTime"].get_int64();
            
            auto positionsArray = doc["positions"].get_array().value;
            
            for(auto& child : positionsArray){
                
                float x = child.get_document().view()["lat"].get_double();
                float y = child.get_document().view()["long"].get_double();
                

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
        
        std::time_t getCreationTime() const{
            return creationTime;
        };
        
        std::time_t getLastUpdateTime() const{
            return lastUpdateTime;
        };
        
        void setLastUpdateTime(uint64_t time){
            lastUpdateTime = time;
        }
        
        float getNormalizedTime() const{
            return normalizedTime;
        };
        
        void setInitialColor(ci::ColorA& c){
            initialColor = c;
        }
        
        ci::ColorA getInitialColor(){
            return initialColor;
        }
        
        std::vector<ci::vec2> getPositions() const{
            return positions;
        }
        
        void setActive(bool a){
            active = a;
        }
        bool isActive()const { return active; }
        
        bool draw();
        
        
    protected:
        
        float normalizedTime = 0.0f;
        
        bool active = true;
        bool needsRedraw = false;
        
        ci::vec2 position;
        ci::vec2 lastPosition;
        
        ci::ColorA initialColor;
        
        // infos
        std::time_t lastUpdateTime;
        std::time_t creationTime;
        
        std::string mKey;
        std::vector<ci::vec2> positions;
        
        friend class csys::PlaneManager;

    };
    
}// end of namespace csys

#endif /* Plane_hpp */
