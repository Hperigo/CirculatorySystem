//
//  Render.cpp
//  CirculatorySys
//
//  Created by Henrique on 11/04/17.
//
//


#include <cmath>
#include "Render.hpp"

#include "cinder/Log.h"
#include "CinderImGui.h"

#include "CinderCereal.h"
#include <cereal/archives/json.hpp>
#include <cereal/types/vector.hpp>

#include "GeoUtils.h"
#include "PlaneManager.hpp"


namespace csys {
    
    using namespace ci;
    using namespace ci::app;
    /// Settings --------
    
    
    template<typename T>
    T vectorLerp( const std::vector<T>& values, float t)
    {
        
        
        int lowerIndex = std::floor(t);
        int upperIndex = std::ceil(t);
        
        float f;
        float scaledTime = std::modf(t, &f);
        
        T value = ci::lerp<T>(values[lowerIndex], values[upperIndex], scaledTime);
        
        return value;
    }

    
    void Render::RenderSettings::drawUi(){
        
        ui::Spacing();
        ui::Checkbox("clear fbo" , &doClear);
        ui::DragFloat("max speed", &maxSpeed);
        //    ui::SliderFloat("scale", &mapScale, 0.0f, 10.0f);
        ui::SliderFloat("Alpha clear", &fadeAlpha, 0.0f, 1.0f);
        ui::ColorEdit4("trail A", &trailColorA[0]);
        ui::ColorEdit4("trail B", &trailColorB[0]);
        ui::ColorEdit4("trail tint", &tintColor[0]);

        ui::Dummy(ImVec2(0, 10));
        
        //terminator
        if(ui::DragFloat("lat", &lat)){
            Render::terminatorNeedsRedraw = true;
        }
        if(ui::DragFloat("lon", &lon)){
            Render::terminatorNeedsRedraw = true;
        }
        if(ui::DragFloat("gamma", &gamma)){
            Render::terminatorNeedsRedraw = true;
        }
        
        if(ui::ColorEdit4("Color", &terminatorColor[0])){
            Render::terminatorNeedsRedraw = true;
        }

        
        
        ui::Dummy(ImVec2(0, 10));
        
        ui::Checkbox("draw Map" , &drawMap);
        ui::Checkbox("draw Fbo" , &drawFbo);
        ui::ColorEdit4("map tint", &mapColor[0]);
        
        
        
        auto store = ui::GetStateStorage();
        auto kvp = store->Data;
        
        ui::Dummy(ImVec2(0, 10));

        if(ui::Button("Reload Assets")){
            mRender->loadAssets();
        }
        
        ui::Dummy(ImVec2(0, 15));

        if(ui::Button("Save")){
            save();
        }
        
        if(ui::Button("Load")){
            load();
        }
        
    }
    
    void Render::RenderSettings::save(){
        
        
        try{
           auto path = getAssetPath("").string() + "/out.json";
            
            std::ofstream outFile(path);
            cereal::JSONOutputArchive mOutArchive(outFile);
            
            CI_LOG_I("Saving...");
            
            mOutArchive(doClear, maxSpeed, fadeAlpha, trailColorA, trailColorB, tintColor, drawMap, mapColor, lat, lon, gamma, terminatorColor);
            
        }catch(std::exception &e){
            
            CI_LOG_EXCEPTION("Error saving: ", e);
        }
        
    }
    
    void Render::RenderSettings::load(){
        
        try{
           
            auto path = getAssetPath("").string() + "/out.json";
            std::ifstream inFile(path);
            cereal::JSONInputArchive mInArchive(inFile);
            
            CI_LOG_I("loading...");
            mInArchive(doClear, maxSpeed, fadeAlpha, trailColorA, trailColorB, tintColor, drawMap, mapColor, lat, lon, gamma, terminatorColor);
            
            
        }catch(std::exception &e){
            
            CI_LOG_EXCEPTION("Error loading: ", e );
            
        }
        
    }
    
    // End settings -----
    
    bool Render::terminatorNeedsRedraw = true;
    
    void Render::setup(const cinder::ivec2 &size){
        
        
        mSettings = RenderSettings(this);
        mSettings.load();
        
        loadAssets();
        
        auto texFmt = gl::Texture2d::Format().internalFormat( GL_RGBA16F );
        texFmt.setWrap(GL_REPEAT, GL_REPEAT);
        
        auto fmt = gl::Fbo::Format();
        fmt.setColorTextureFormat( texFmt );
        fmt.setSamples(0);
        
        // FBO'S
        mPlanesFbo = gl::Fbo::create( 2560, 1280 , fmt );
        mTerminatorFbo = gl::Fbo::create( 2560, 1280 , fmt );
        mComposeFbo = gl::Fbo::create( 2560, 1280 , fmt );
        
        
    }
    
    void Render::loadAssets(){
        
        try{
            mMapTexture = gl::Texture::create(loadImage( loadAsset( "map_lat_long.jpg")));
            mMapTexture->setWrap(GL_REPEAT, GL_REPEAT);

            
            mComposeShader = gl::GlslProg::create( gl::GlslProg::Format()
                                                  .vertex( loadAsset("composeVertex.glsl"))
                                                  .fragment(loadAsset("composeFrag.glsl")) );
        }catch(std::exception &e){
            
            CI_LOG_EXCEPTION("could not load render assets..", e);
            
        }
        
    }
    
 
    void Render::renderPlaneFbo(){
        
        auto size = mPlanesFbo->getSize();
        
        gl::ScopedFramebuffer f(mPlanesFbo);
        gl::ScopedViewport v( size  );
        gl::setMatricesWindow( size );
        
        gl::enableAlphaBlending();
        
        
        if(doFirstClear || mSettings.doClear){
            gl::clear(ColorA(1.0f,1.0f,1.0f,0.0f));
            doFirstClear = false;
        }else{
        
            gl::color( 1.0f, 1.0f, 1.0f, mSettings.fadeAlpha );
            gl::draw(mPlanesFbo->getColorTexture());
        }
        
        
        
        gl::ScopedModelMatrix m;
        gl::scale(mSettings.mapScale,mSettings.mapScale,1.0f);
        
        gl::lineWidth(3.0f);
        gl::begin(GL_LINES);
        
        
        auto sortedPlanes =  PlaneManager::instance().getSortedPlanes();
        for(auto& plane : sortedPlanes){
            
            
            if(!plane->isActive()){
                continue;
            }
            
            auto positions = plane->getPositions();
            float normalizedIndex = ci::constrain<float>(   plane->getNormalizedTime() *  (positions.size() - 1 ), 0, positions.size() - 1 );
            
            
            
            if(normalizedIndex > 1){
                
                
                auto latA = vectorLerp(positions, normalizedIndex);
                auto pointA = csys::geo::latLongToCartesian(  size , latA);
                
                auto latB = vectorLerp(positions, normalizedIndex - 1);
                auto pointB = csys::geo::latLongToCartesian( size , latB);
                
                auto geoDistance = csys::geo::distanceLatLong(latA, latB);
                auto pixelDistance = glm::distance(pointA, pointB);
                
                float colorFactor =  constrain(pixelDistance / mSettings.maxSpeed, 0.0f, 1.0f);
                
                ColorA finalColor = lerp(mSettings.trailColorA, mSettings.trailColorB, colorFactor);
                
                
                
                
                if( geoDistance < 500 &&  pixelDistance < 300){
                    
                    gl::color( finalColor );
                    gl::vertex(pointA);
                    gl::color( finalColor );
                    gl::vertex(pointB);
                    
                    gl::color( finalColor );
                    gl::vertex(pointA-vec2(1));
                    gl::color( finalColor );
                    gl::vertex(pointB-vec2(1));
                    
                    gl::color( finalColor );
                    gl::vertex(pointA-vec2(2));
                    gl::color( finalColor );
                    gl::vertex(pointB-vec2(2));
                    
                }
                
            }
            
        }
        gl::end();
   
    }
    
    void Render::renderTerminatorFbo(){
        auto size = mTerminatorFbo->getSize();
        
//        if(!terminatorNeedsRedraw){
//            return;
//        }
        

        gl::ScopedFramebuffer f(mTerminatorFbo);
        gl::ScopedViewport v( size  );
        gl::setMatricesWindow( size );

        gl::clear(ColorA(0.0f,0.0f,0.0f,1.0f));

        gl::enableAlphaBlending();
        
        gl::color(mSettings.terminatorColor);
        Path2d terminatorPath;

        terminatorPath.moveTo(vec2(0, -10));

        auto v1 = csys::geo::getTerminatorCoords(-180.0f, mSettings.lon, mSettings.gamma);
        auto p1 = csys::geo::latLongToCartesian(size, v1);
        terminatorPath.lineTo(p1);

        for(int x = -180; x <= 180; x+=4){

            auto vv = csys::geo::getTerminatorCoords(x+mSettings.lat, mSettings.lon, mSettings.gamma);
            auto pp = csys::geo::latLongToCartesian(size, vv);
            terminatorPath.lineTo(pp);
        }

        auto v2 = csys::geo::getTerminatorCoords(180.0f, mSettings.lon, mSettings.gamma);
        auto p2 = csys::geo::latLongToCartesian(size, v2);
        terminatorPath.lineTo(p2);

        terminatorPath.lineTo(vec2(size.x, -10));
        
        terminatorPath.close();
        
        
        gl::drawSolid(terminatorPath);
        
        gl::drawSolidCircle(p2, 10);
        gl::drawSolidCircle(p1, 10);
        
        gl::enableAlphaBlending();
        
        terminatorNeedsRedraw = false;
        
    }
    
    void Render::renderCompose(){
        
        auto size = mComposeFbo->getSize();
        
        gl::ScopedFramebuffer f(mComposeFbo);
        gl::ScopedViewport v( size  );
        gl::setMatricesWindow( size );
        
        
        gl::clear(Color::black());
        
        gl::color( Color::black() );
        gl::drawSolidRect( Rectf(0.0f,0.0f, size.x, size.y));
        
        gl::color( Color::white() );
        
        gl::ScopedGlslProg prog( mComposeShader );
        
        
        mMapTexture->bind(0);
        
        auto planesTexture = mPlanesFbo->getColorTexture();
        planesTexture->bind(1);
        
        auto terminatorTexture = mTerminatorFbo->getColorTexture();
        terminatorTexture->bind(2);
        
        mComposeShader->uniform("uTexPlanes", 1);
        mComposeShader->uniform("uTexTerminator", 2);
        mComposeShader->uniform("uTime", float(ci::app::getElapsedSeconds()) * 0.01f );
        
        gl::drawSolidRect(Rectf(0,0,size.x, size.y) );
        
        
        mMapTexture->unbind();
        planesTexture->unbind();
        terminatorTexture->unbind();
        
    }
    
}