//
//  Render.hpp
//  CirculatorySys
//
//  Created by Henrique on 11/04/17.
//
//

#ifndef Render_hpp
#define Render_hpp

#include <stdio.h>
#include "cinder/gl/gl.h"
#include "Settings.hpp"

namespace csys {
    
    
    class Render {
        public:

        class RenderSettings : public csys::Settings{
        public:
            RenderSettings(){
                
            }
            
            RenderSettings(Render* r) : mRender(r){
                
                
            }
            virtual ~RenderSettings(){
                
            }
            // global
            ci::ivec2 windowSize = {1580, 640};
            
            
            //Plane Fbo
            bool doClear = false;
            float maxSpeed = 10;
            float mapScale = 1.0f;
            float fadeAlpha = 0.1f;
            ci::ColorA trailColorA {  0.9f, 0.8f, 0.1f, 0.9f };
            ci::ColorA trailColorB {  0.9f, 0.5f, 0.1f, 0.1f };
            ci::ColorA tintColor { 0.9, 0.6, 0.0, 0.9f };
            
            // Terminator Fbo
            float lat;
            float lon;
            float gamma;
            ci::ColorA terminatorColor;
            
            //Main render
            bool drawMap = true;
            bool drawFbo = true;
            ci::ColorA mapColor { 0.9f,0.3f, 0.3f, 0.9f };
            
            
            void drawUi() override;
            
            void save() override;
            void load() override;
            
            Render* mRender;
        };
        
        
        Render(){
            
        }
        ~Render(){
            
        }
        
        void setup(const ci::ivec2& size);
        
        
        // FBO's
        ci::gl::FboRef mPlanesFbo;
        ci::gl::FboRef mTerminatorFbo;
        ci::gl::FboRef mComposeFbo;
        
        static bool terminatorNeedsRedraw;
        
        ci::gl::TextureRef mMapTexture;
        
        ci::gl::GlslProgRef mComposeShader;
        
        
        void renderPlaneFbo();
        void renderTerminatorFbo();
        void renderCompose();
        
        
        void loadAssets();
        
        bool doFirstClear = true;
        RenderSettings mSettings;
        
    };
    
    
}


#endif /* Render_hpp */
