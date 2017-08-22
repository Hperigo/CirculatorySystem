//
//  PlaneManager.cpp
//  CirculatorySys
//
//  Created by Henrique on 06/04/17.
//
//

#include <string>

#include "PlaneManager.hpp"
#include "GeoUtils.h"

#include "cinder/Log.h"
#include "CinderImGui.h"

#include "CinderCereal.h"
#include <cereal/archives/json.hpp>
#include <cereal/types/vector.hpp>

namespace csys {
    
    using bsoncxx::builder::stream::close_array;
    using bsoncxx::builder::stream::close_document;
    using bsoncxx::builder::stream::document;
    using bsoncxx::builder::stream::finalize;
    using bsoncxx::builder::stream::open_array;
    using bsoncxx::builder::stream::open_document;

    
    
    void PlaneManager::PlaneManagerSettings::load(){
        
        
        try{
            
            auto path = cinder::app::getResourcePath("out_plane.json").string();
            std::ifstream inFile(path);
            cereal::JSONInputArchive archive(inFile);
            
            archive(mSpeed);
            
            
        }catch(std::exception &e){
            
            CI_LOG_EXCEPTION("Error loading: ", e );
            
        }
    }
    
    void PlaneManager::PlaneManagerSettings::save(){
        
        try{
            
            auto path = cinder::app::getResourcePath("out_plane.json").string();;
            
            std::ofstream outFile(path);
            cereal::JSONOutputArchive archive(outFile);
            
            archive(mSpeed);
            
        }catch(std::exception &e){
            
            CI_LOG_EXCEPTION("Error saving: ", e);
        }

        
    }
    
    void PlaneManager::PlaneManagerSettings::drawUi(){
        
        
        ui::LabelText(std::to_string(unbornPlanes).c_str(), "unborn: ");
        ui::LabelText(std::to_string(deadPlanes).c_str(), "dead: ");
        ui::LabelText(std::to_string(activePlanes).c_str(), "active: ");
        
        
        ui::Spacing();
        //                ui::LabelText(std::to_string(mGlobalTime).c_str(), "global time: ");
        
        ui::SliderInt("speed", &mSpeed, 1, 5000);
        
        if(ui::Button("Save")){
            save();
        }
        if(ui::Button("Load")){
            load();
        }
    }
    
    
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
            
            ci::app::console() << mInitialTime << std::endl;
            ci::app::console() << mEndTime << std::endl;
            
        });
        
        mSettings.load();
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
        
    
        mGlobalTime = mTimer.getSeconds() * mSettings.mSpeed;
        if(mGlobalTime > (mEndTime - mInitialTime) ){
            mTimer.start();
        }
        
        mTimePercentage = mGlobalTime / (mEndTime - mInitialTime);

        
        mSettings.unbornPlanes = 0;
        mSettings.deadPlanes = 0;
        mSettings.activePlanes = 0;
        
        for(auto& plane : mSortedPlanes){
            
            time_t planeIntialTime = plane->getCreationTime()  - mInitialTime;
            time_t planeLastupdateTime = plane->getLastUpdateTime();
            time_t planeEndTime = planeLastupdateTime  -  mInitialTime;
            
            
            // check if plane exists in our time frame
            if( mGlobalTime < planeIntialTime){
                plane->setActive(false);
                mSettings.unbornPlanes++;
                continue;
            }else if(mGlobalTime > planeEndTime){
                plane->setActive(false);
                mSettings.deadPlanes++;
                continue;
            }
        
            float deltaTime = planeEndTime - planeIntialTime;
            if(deltaTime == 0){
                plane->setActive(false);
                continue;
            }
            
            // Plane will born
            if(plane->isActive() == false){
               
                
                auto pos =  geo::latLongToCartesian(mColorMap.getSize(), plane->getPositions()[0] );
                
                ci::ColorA col =  mColorMap.getPixel( pos );
//                col.a = 0.5f;
                
                plane->setInitialColor(col);
            }
            
            plane->setActive(true);
            plane->normalizedTime = (mGlobalTime - planeIntialTime) / deltaTime;
            mSettings.activePlanes++;
        }// eo-for
        
    }
    
    
    void PlaneManager::drawUi(){

        mSettings.drawUi();
    }
    
    
}// endof namespace
