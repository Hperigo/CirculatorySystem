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
            ci::ivec2 windowSize = {1780, 840};
            
            
            //Plane Fbo
            bool doClear = false;
            float maxSpeed = 10;
            float mapScale = 1.0f;
            float fadeAlpha = 0.1f;
            
            float pointSize = 1.0f;
            
            ci::vec2 LRCoord = {0.993f, 0.0f};
            ci::vec2 ULCoord = {0.003f, 1.0f};
            
            
            ci::ColorA trailColorA {  1.0f, 1.0f, 1.0f, 0.9f };
            ci::ColorA trailColorB {  1.0f, 1.0f, 1.0f, 0.1f };
//            ci::ColorA tintColor { 0.9, 0.6, 0.0, 0.9f };
            
            ci::ColorA mapDayTint { 0.9, 0.6, 0.0, 1.0f };
            ci::ColorA mapNightTint { 0.0, 0.0, 1.0, 1.0f };
            
            ci::ColorA planesDayTint { 1.0, 0.6, 0.0, 1.0f };
            ci::ColorA planesNightTint { 0.0, 0.0, 1.0, 1.0f };
            
            
            // Terminator Fbo
            float lat = 0.0f;
            float lon = 0.0f;
            float gamma = 7.0f;
            float blurAmt = 1.0f;
            
            float terminatorOffset = -0.550;
            
            //Main render
            bool drawMap = true;
            bool drawFbo = true;
            ci::ColorA mapColor { 0.9f,0.3f, 0.3f, 0.9f };
            
            void drawUi() override;
            
            void save() override;
            void load() override;
            
            bool doComposite = true;
            
            Render* mRender;
        };
        
        
        Render(){
            
        }
        ~Render(){
            
        }
        
        void setup(const ci::ivec2& size);
        
        
        // FBO's
        ci::gl::FboRef mPlanesFbo;
        ci::gl::FboRef mPlanesEdgeBlend;
        
        // terminator
        ci::gl::FboRef mTerminatorFbo;
        ci::gl::FboRef mHorizontalFbo;
        ci::gl::FboRef mVerticalFbo;
        
        
        ci::gl::FboRef mComposeFbo;
        
        static bool terminatorNeedsRedraw;
        
        ci::gl::TextureRef mMapTexture;
        
        ci::gl::GlslProgRef mDefaultShader;
        ci::gl::GlslProgRef mComposeShader;
        ci::gl::GlslProgRef mBlurShader;

        
        
        void renderPlaneFbo();
        void renderTerminatorFbo();
        void renderBlurFbo();
        
        void renderPlaneEdgeBlend();
        void renderCompose();
        
        
        void loadAssets();
        
        bool doFirstClear = true;
        RenderSettings mSettings;
        
        std::vector<glm::vec2> dotsPosition;
        
    };
    
    
}


#endif /* Render_hpp */
