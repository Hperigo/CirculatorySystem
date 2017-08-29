//
//  PlaneManager.hpp
//  CirculatorySys
//
//  Created by Henrique on 06/04/17.
//
//

#ifndef PlaneManager_hpp
#define PlaneManager_hpp

#include <stdio.h>
#include "cinder/Timer.h"

#include "Settings.hpp"
#include "Database.hpp"


namespace csys {
    
    class PlaneManager{
        
    public:
        
        
        class PlaneManagerSettings : public csys::Settings{
            
        public:
            int mSpeed = 100;
            // stats
            size_t unbornPlanes = 0;
            size_t deadPlanes = 0;
            size_t activePlanes = 0;
            
            void load() override;
            void save() override;
            void drawUi() override;
            
        };
        
        
        
        
        static PlaneManager& instance()
        {
            static PlaneManager instance;
            return instance;
        }
        
        void initFromDB();
        
        void update();
        void drawUi();
        
        void sortQueryPlanes();
        std::time_t calculateInitialTime();
        std::time_t calculateEndTime();
        
        double getGlobalTime() const {
            return mGlobalTime;
        }

        double getPercentageTime() const {
            return mTimePercentage;
        }
        
        
        void setColorMap(const ci::Surface& surf){
            mColorMap = surf;
        }
        
        std::map<std::string , csys::PlaneRef>& getPlanes() { return mPlanesMap; }
        std::vector<csys::PlaneRef>& getSortedPlanes() { return mSortedPlanes; }
        
    private:
        
        
        std::vector<ci::vec2> filterPlanePositions(const csys::PlaneRef& plane );
        float findIntersection(const ci::vec2& pointA,const ci::vec2& pointB, const float width );
        
        double mTimePercentage = 0.0f;
        
        std::time_t mInitialTime;
        std::time_t mEndTime;
        
        ci::Timer mTimer;
        double mGlobalTime;
        
        csys::Database mDatabase;
        
        std::map<std::string , csys::PlaneRef> mPlanesMap;
        std::vector<csys::PlaneRef> mSortedPlanes;
        
        PlaneManagerSettings mSettings;
        
        //surface
        
        ci::Surface mColorMap;
        
    };
    
}

#endif /* PlaneManager_hpp */
