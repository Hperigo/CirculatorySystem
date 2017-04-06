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

#include "Database.hpp"


namespace csys {
    
    class PlaneManager{
        
    public:
        
        void initFromDB();
        
        void update();
        void drawUi();
        
        void sortQueryPlanes();
        std::time_t calculateInitialTime();
        std::time_t calculateEndTime();
        
        double getGlobalTime() const {
            return mGlobalTime;
        }
        
        std::map<std::string , csys::PlaneRef>& getPlanes() { return mPlanesMap; }
        std::vector<csys::PlaneRef>& getSortedPlanes() { return mSortedPlanes; }
        
    private:
        
        std::time_t mInitialTime;
        std::time_t mEndTime;
        
        ci::Timer mTimer;
        double mGlobalTime;
        int mSpeed = 1500;
        
        csys::Database mDatabase;
        
        std::map<std::string , csys::PlaneRef> mPlanesMap;
        std::vector<csys::PlaneRef> mSortedPlanes;
        
        
        // stats
        size_t unbornPlanes = 0;
        size_t deadPlanes = 0;
        
        
    };
    
}

#endif /* PlaneManager_hpp */
