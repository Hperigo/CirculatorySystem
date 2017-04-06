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
        
        void init();
        
        void calculateInitialTime();
        void calculateEndTime();
        
        void update();
        
        // -----
        // make initial query
        // sort planes
        // calculate initial Time
        // calculate end Time
        // update
        
        
        // MPlanes
        // MDB
        
    private:
        
        std::time_t mInitialTime;
        std::time_t mEndTime;
        
        ci::Timer mGlobalTimer;
        
        
        csys::Database mDatabase;
    };
    
}

#endif /* PlaneManager_hpp */
