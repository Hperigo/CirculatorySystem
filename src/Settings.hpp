//
//  Settings.hpp
//  CirculatorySys
//
//  Created by Henrique on 12/04/17.
//
//

#ifndef Settings_hpp
#define Settings_hpp

#include <stdio.h>

// Base class for settings


namespace csys{
    
    class Settings{
        
        
    public:
        virtual void drawUi() = 0;
        
        virtual void save() = 0;
        virtual void load() = 0;

    };
    
    typedef std::shared_ptr<Settings> SettingsRef;
    
}


#endif /* Settings_hpp */
