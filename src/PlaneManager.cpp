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
#include "cinder/Path2D.h"
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
                
                plane->setFilteredPositions( filterPlanePositions( plane ) );
                
                if( plane->getFilteredPositions().size() == 0 ){
                    continue;
                }
                
                mPlanesMap[plane->getKey()] = plane;
            }
            
            for(auto& plane : mPlanesMap){
                
                mSortedPlanes.push_back( plane.second );
            }
            
            auto cmp = [](const csys::PlaneRef& a, const csys::PlaneRef& b ){
                
                return a->getCreationTime() < b->getCreationTime();
                
            };
        
            std::sort(mSortedPlanes.begin(), mSortedPlanes.end(), cmp);
        

            
            
            mInitialTime = calculateInitialTime();
            mEndTime     = calculateEndTime();
            
            ci::app::console() << mInitialTime << std::endl;
            ci::app::console() << mEndTime << std::endl;
            
        });
        
        //mSettings.load();
        mTimer.start();
    }
    
    
    float PlaneManager::findIntersection(const ci::vec2& pointA,const ci::vec2& pointB, const float width ){
        
        
        ci::vec2 target = pointB - pointA;
        float r = glm::atan(target.y, target.x);
        
        float h =  width * glm::tan(r);
        
        return h;
    }

    
    
    std::vector<ci::vec2> PlaneManager::filterPlanePositions(const PlaneRef& plane ){
        
        ci::vec2 mapSize{ 2160, 1080 };

        
        
        std::vector<ci::Path2d> paths;
        
        auto positions = plane->getPositions();
        
        paths.push_back(ci::Path2d());
        paths.back().moveTo( csys::geo::latLongToCartesian(mapSize, positions[0])  );
        
        
        bool found = false;
        bool leftToRight = false;
        
        
        ci::vec2 edgePointA;
        ci::vec2 edgePointB;
        
        for( int i = 0; i < positions.size(); ++i){
            
            
            ci::vec2 p = positions[i];
            ci::vec2 screen_pos = csys::geo::latLongToCartesian(mapSize, p);
            

            if( (i + 1) < (positions.size()) ){
                
                ci::vec2 pNext = positions[i + 1];
                ci::vec2 screenNext = csys::geo::latLongToCartesian(mapSize, pNext);
                
                
                if( glm::distance(screen_pos, screenNext) > mapSize.x - 500 ){
                    
                    edgePointA = screen_pos;
                    edgePointB = screenNext;
                    found = true;
                    
                    float intersection = screen_pos.y + (screenNext.y - screen_pos.y ) / 2.0f;
                    
                    if( screen_pos.x > screenNext.x ){ // means right to left
                        
                        ci::vec2 a = screen_pos;
                        ci::vec2 b =  ci::vec2(mapSize.x + screenNext.x, screenNext.y);
                        
                        intersection =  findIntersection(a, b, mapSize.x - a.x);
                        
                        paths.back().lineTo( screen_pos );
                        paths.back().lineTo( ci::vec2( mapSize.x,  screen_pos.y + intersection ) );
                        
                        paths.push_back(ci::Path2d());
                        
                        
                        
                        paths.back().moveTo( ci::vec2(0,   screen_pos.y + intersection ) );
                        paths.back().lineTo( screenNext );
                        continue;
                        
                    }else{
                        
                        leftToRight = true;
                        
                        float w = mapSize.x - screenNext.x;
                        
                        ci::vec2 a(-w, screenNext.y);
                        ci::vec2 b = screen_pos;
                        
                        intersection =  findIntersection(a, b, -w);
                        
                        paths.back().lineTo( screen_pos );
                        paths.back().lineTo( ci::vec2(0,  screenNext.y - intersection ) );
                        
                        paths.push_back(ci::Path2d());
                        paths.back().moveTo( ci::vec2( mapSize.x, screenNext.y - intersection ) );
                        paths.back().lineTo( screenNext );
                        continue;
                        
                    }
                    
                }
                
            }
            
            paths.back().lineTo( screen_pos );
            
        }
        
        std::vector<ci::vec2> pppoints;
        
        
        
        for(auto shape : paths){
            
            float totalLength = 0;
            
            totalLength = shape.calcLength();
            
            
            for( int i = 0; i < totalLength; i+= 2){
                
                float v = shape.calcTimeForDistance(i + 0.001);
                ci::vec2 p = shape.getPosition(v);
                pppoints.push_back( p );
                
            }
        }
        
        return pppoints;
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
               
                
//                auto pos =  geo::latLongToCartesian(mColorMap.getSize(), plane->getPositions()[0] );
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
