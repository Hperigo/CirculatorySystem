//
//  PlaneManager.cpp
//  CirculatorySys
//
//  Created by Henrique on 06/04/17.
//
//

#include <string>

#include "PlaneManager.hpp"

#include "cinder/Log.h"
#include "CinderImGui.h"

namespace csys {
    
    using bsoncxx::builder::stream::close_array;
    using bsoncxx::builder::stream::close_document;
    using bsoncxx::builder::stream::document;
    using bsoncxx::builder::stream::finalize;
    using bsoncxx::builder::stream::open_array;
    using bsoncxx::builder::stream::open_document;

    
    // DB mode
    void PlaneManager::initFromDB(){
        
        CI_LOG_I("Init plane manager");
        
        mDatabase.setup();

        mDatabase.addQuery( document{} << finalize , [&] ( Database::DocContainer& planes ){
            
            CI_LOG_I("Init callback");
            
            //
            mPlanesMap.clear();
            mSortedPlanes.clear();
            
            for(auto plane : planes) {
                mPlanesMap[plane->getKey()] = plane;
            }
            
            for(auto& plane : mPlanesMap){
                
                mSortedPlanes.push_back( plane.second );
            }
            
            auto cmp = [](const csys::PlaneRef& a, const csys::PlaneRef& b ){
                
                return a->getCreationTime() < b->getCreationTime();
                
            };
            
            CI_LOG_I("Sorted planes");
            
            std::sort(mSortedPlanes.begin(), mSortedPlanes.end(), cmp);
            
            mInitialTime = calculateInitialTime();
            mEndTime     = calculateEndTime();
            
        });
        
        
        mTimer.start();
    }
    
    
    std::time_t PlaneManager::calculateInitialTime(){
        
        if(mSortedPlanes.size() > 0){
            return mSortedPlanes[0]->getCreationTime();
        }
        return 0;
    }
    
    std::time_t PlaneManager::calculateEndTime(){
        
        if(mSortedPlanes.size() > 0){
            return  mSortedPlanes[mSortedPlanes.size() - 1]->getCreationTime();
        }
        return 0;
    }
    
    
    void PlaneManager::update(){
        
    
        mGlobalTime = mTimer.getSeconds() * mSpeed;
        if(mGlobalTime > (mEndTime - mInitialTime) ){
            mTimer.start();
        }
        unbornPlanes = 0;
        deadPlanes = 0;
        
        for(auto& plane : mSortedPlanes){
            
            time_t planeIntialTime = plane->getCreationTime()  - mInitialTime;
            time_t planeLastupdateTime = plane->getLastUpdateTime();
            time_t planeEndTime = planeLastupdateTime  -  mInitialTime;
            
            
            if( mGlobalTime < planeIntialTime){
                plane->setActive(false);
                unbornPlanes++;
                continue;
            }else if(mGlobalTime > planeEndTime){
                plane->setActive(false);
                deadPlanes++;
                continue;
            }
            
            
            float deltaTime = planeEndTime - planeIntialTime;
            if(deltaTime == 0){
                plane->setActive(false);
                continue;
            }
            
            plane->setActive(true);
            plane->normalizedTime = (mGlobalTime - planeIntialTime) / deltaTime;
            
        }// eo-for
        
    }
    
    
    void PlaneManager::drawUi(){
        
        ui::LabelText(std::to_string(unbornPlanes).c_str(), "unborn: ");
        ui::LabelText(std::to_string(deadPlanes).c_str(), "dead: ");
        
        ui::SliderInt("speed", &mSpeed, 1, 10000);
        
    }
    
    
}// endof namespace