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
        ui::Checkbox("Do composite" , &doComposite);
        ui::DragFloat("max speed", &maxSpeed, 0.00001f, 0.0f, 1.0f, "%.5f");
//        //    ui::SliderFloat("scale", &mapScale, 0.0f, 10.0f);
//        ui::ColorEdit4("trail A", &trailColorA[0]);
//        ui::ColorEdit4("trail B", &trailColorB[0]);
        
        
        ui::DragFloat("Alpha clear", &fadeAlpha, 0.0001f, 0.0f, 1.0f, "%.5f");
        
        
        

        ui::DragFloat("Point size", &pointSize, 0.01f);

        ui::Dummy(ImVec2(0, 10));

        ui::DragFloat("Map Day Color", &mapDayTint[0], 0.01f);
        ui::DragFloat("Map Night Color", &mapNightTint[0], 0.01f);
        
        
        ui::Dummy(ImVec2(0, 10));
        
        
        //terminator
//        if(ui::DragFloat("lat", &lat)){
//            Render::terminatorNeedsRedraw = true;
//        }
        if(ui::DragFloat("lon", &lon)){
            Render::terminatorNeedsRedraw = true;
        }
        if(ui::DragFloat("gamma", &gamma)){
            Render::terminatorNeedsRedraw = true;
        }
        
        if(ui::DragFloat("Blur Amt", &blurAmt)){
            Render::terminatorNeedsRedraw = true;
        }
        
        ui::DragFloat("Terminator offset", &terminatorOffset, 0.01f);

        ui::Dummy(ImVec2(0, 10));

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
//           auto path = getAssetPath("").string() + "/out.json";
            
            auto path = getResourcePath("out.json").string();
            
            console() << path << std::endl;
            
            std::ofstream outFile(path);
            cereal::JSONOutputArchive archive(outFile);
            
            CI_LOG_I("Saving...");
            
            archive(doClear, maxSpeed, fadeAlpha,
                        trailColorA, trailColorB, mapDayTint, mapNightTint, planesDayTint, planesNightTint,
                        drawMap, mapColor,
                        lat, lon, gamma, blurAmt, pointSize, terminatorOffset);
            
        }catch(std::exception &e){
            
            CI_LOG_EXCEPTION("Error saving: ", e);
        }
        
    }
    
    void Render::RenderSettings::load(){
        
        try{
           
            auto path = getResourcePath("out.json").string();
            
            console() << path << std::endl;
            
            std::ifstream inFile(path);
            cereal::JSONInputArchive archive(inFile);
            
            CI_LOG_I("loading...");
            archive(doClear, maxSpeed, fadeAlpha,
                        trailColorA, trailColorB, mapDayTint, mapNightTint, planesDayTint, planesNightTint,
                        drawMap, mapColor,
                        lat, lon, gamma, blurAmt, pointSize, terminatorOffset);
            
            
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
        
//        GL_RGBA16F
        auto texFmt = gl::Texture2d::Format().internalFormat( GL_RGBA32F );
        texFmt.setWrap(GL_REPEAT, GL_REPEAT);
        
        auto fmt = gl::Fbo::Format();
        fmt.setColorTextureFormat( texFmt );
        fmt.setSamples(4);
        
        // FBO'S
//        mPlanesFbo = gl::Fbo::create( 2160,1080 , fmt );
        mPlanesFbo = gl::Fbo::create( size.x,size.y , fmt );
        

        texFmt = gl::Texture2d::Format().internalFormat( GL_RGBA );
        texFmt.setWrap(GL_REPEAT, GL_REPEAT);
        
        fmt = gl::Fbo::Format();
        fmt.setColorTextureFormat( texFmt );
        fmt.setSamples(0);
        
        
        mPlanesEdgeBlend = gl::Fbo::create(size.x,size.y, fmt );
        mComposeFbo = gl::Fbo::create( size.x,size.y , fmt );
        
        
        
        auto t_size = vec2(size) / 2.0f;
        
        mTerminatorFbo = gl::Fbo::create( t_size.x, t_size.y , fmt );
        mHorizontalFbo = gl::Fbo::create(t_size.x, t_size.y , fmt );
        mVerticalFbo = gl::Fbo::create( t_size.x, t_size.y , fmt );
        
    }
    
    void Render::loadAssets(){
        
        try{
            mMapTexture = gl::Texture::create(loadImage( loadResource( "map.png")));
            mMapTexture->setWrap(GL_REPEAT, GL_REPEAT);

            
            mComposeShader = gl::GlslProg::create( gl::GlslProg::Format()
                                                  .vertex( loadResource("composeVertex.glsl"))
                                                  .fragment(loadResource("composeFrag.glsl")) );
            
            
            mBlurShader = gl::GlslProg::create( gl::GlslProg::Format()
                                               .vertex( loadResource("blurVertex.glsl"))
                                               .fragment(loadResource("blurFrag.glsl")) );
            
            
            mDefaultShader = gl::GlslProg::create( gl::GlslProg::Format()
                                                  .vertex( loadResource("defaultVertex.glsl"))
                                                  .fragment(loadResource("defaultFrag.glsl")) );
            
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
        
        
        gl::pointSize(mSettings.pointSize);
        gl::begin(GL_POINTS);
        
        
        auto sortedPlanes =  PlaneManager::instance().getSortedPlanes();
        
//        dotsPosition.clear();
        
        
//        int index = 0;
        for(auto& plane : sortedPlanes){
            
            
            if(!plane->isActive()){
                continue;
            }
        
            
            auto filteredPositions = plane->getFilteredPositions();
            
            if( filteredPositions.size() == 0 ){
                continue;
            }
            
            /*
                ci::app::console() << "Normalized time: " << plane->getNormalizedTime() << "\n";
                ci::app::console() << "filtered pos size: " << filteredPositions.size() << "\n";
                ci::app::console() << "index: " << index << "\n";
                ci::app::console() << "plane: " << plane->getKey() << "\n";
                index++;
            */


            
            float normalizedIndex = ci::constrain<float>(   plane->getNormalizedTime() *  (filteredPositions.size() - 1 ), 0, filteredPositions.size() - 1 );
            

            ColorA finalColor = plane->getInitialColor(); //..lerp(mSettings.trailColorA, mSettings.trailColorB, colorFactor);
            
            ci::vec2 pointA = vectorLerp(filteredPositions, normalizedIndex);
            gl::color( finalColor );
            gl::vertex(pointA);
            
            
            if(normalizedIndex){
                
                
//                auto latA = vectorLerp(positions, normalizedIndex);
//                auto pointA = csys::geo::latLongToCartesian(  size , latA);
//                
//                auto latB = vectorLerp(positions, normalizedIndex - 1);
//                auto pointB = csys::geo::latLongToCartesian( size , latB);
//                
//                auto geoDistance = csys::geo::distanceLatLong(latA, latB);
//                auto pixelDistance = glm::distance(pointA, pointB);
//                
////                float colorFactor =  constrain(pixelDistance / mSettings.maxSpeed, 0.0f, 1.0f);
//                //ColorA(1.0f, 0.3f, 0.3f, 1.0f);/
//                ColorA finalColor = plane->getInitialColor(); //..lerp(mSettings.trailColorA, mSettings.trailColorB, colorFactor);
//                
////                finalColor.a = 0.6f;
//                
//                
////                gl::color( finalColor );
////                gl::vertex(pointA);
//                
//                
//                if( geoDistance < 1000){
//                    
//                    gl::color( finalColor );
//                    gl::vertex(pointA);
//                    
//                }
                
                
                

                
            }
            
        }
        
        gl::end();
        
//        gl::color( Color::white());
//        
//        if (  sortedPlanes.size() > 0 ){
//            auto p = sortedPlanes[32930];
//            
//            for( auto& fp : p->getFilteredPositions() ){
//                
//                gl::drawSolidCircle(fp, 5);
//                
//            }
//        }

   
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
        
        gl::color( Color::white() );
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
        
//        gl::drawSolidCircle(p2, 10);
//        gl::drawSolidCircle(p1, 10);
        
        gl::enableAlphaBlending();
        
        terminatorNeedsRedraw = false;
        
    }
    
    void Render::renderBlurFbo(){
        
        {
            auto size = mTerminatorFbo->getSize();

            gl::ScopedFramebuffer f(mHorizontalFbo);
            gl::ScopedViewport v( size  );
            gl::setMatricesWindow( size );
            
            gl::clear(ColorA(0.0f,0.0f,0.0f,1.0f));
            
            
            gl::ScopedGlslProg prog( mBlurShader );
            auto tex = mTerminatorFbo->getColorTexture();
            
            mBlurShader->uniform("sample_offset", vec2( mSettings.blurAmt / size.x, 0.0f ) );
            
            tex->bind();
            
            gl::drawSolidRect( Rectf(0,0, size.x, size.y) );
            
            tex->unbind();
        }
        
        
        
        {
            auto size = mTerminatorFbo->getSize();
            
            gl::ScopedFramebuffer f(mVerticalFbo);
            gl::ScopedViewport v( size  );
            gl::setMatricesWindow( size );
            
            gl::clear(ColorA(0.0f,0.0f,0.0f,1.0f));
            
            
            gl::ScopedGlslProg prog( mBlurShader );
            auto tex = mHorizontalFbo->getColorTexture();
            
            mBlurShader->uniform("sample_offset", vec2( 0.0f , mSettings.blurAmt / size.y ) );
            
            tex->bind();
            
            gl::drawSolidRect( Rectf(0,0, size.x, size.y) );
            
            tex->unbind();
        }
        
    }
    
    void Render::renderPlaneEdgeBlend(){
        
        
        auto size = mPlanesEdgeBlend->getSize();

        
        gl::ScopedFramebuffer f(mPlanesEdgeBlend);
        gl::ScopedViewport v( size  );
        gl::setMatricesWindow( size );
        
        gl::enableAlphaBlending();
        
        gl::GlslProgRef prog = gl::getStockShader( gl::ShaderDef().texture().color() );
        
        gl::ScopedGlslProg shader(prog);
        
        gl::clear( ColorA(0,0,0,0) );
        
        auto planes = mPlanesFbo->getColorTexture();

        
        planes->bind(0);
        gl::drawSolidRect(Rectf(0,0,size.x, size.y), mSettings.ULCoord, mSettings.LRCoord  );
    
        planes->unbind();
        
        gl::color(1.0f, 0.0f, 0.0f, 1.0f);
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
        
        
        if(mSettings.doComposite){
            gl::ScopedGlslProg prog( mComposeShader );

            
            mMapTexture->bind(0);

            auto planesTexture = mPlanesEdgeBlend->getColorTexture();
            planesTexture->bind(1);
            
            auto terminatorTexture = mVerticalFbo->getColorTexture();
            terminatorTexture->bind(2);
            
            mComposeShader->uniform("uTexPlanes", 1);
            mComposeShader->uniform("uTexTerminator", 2);

//            mComposeShader->uniform("uTime", float(PlaneManager::instance().getPercentageTime()));
            
            mComposeShader->uniform("uTime", float(mSettings.maxSpeed * app::getElapsedSeconds()) );
            
            mComposeShader->uniform("offset", mSettings.terminatorOffset);
            
            // --- Colors
            // Map
            mComposeShader->uniform("uMapDayColor", mSettings.mapDayTint);
            mComposeShader->uniform("uMapNightColor", mSettings.mapNightTint);
            
            mComposeShader->uniform("uPlanesDayColor", mSettings.planesDayTint);
            mComposeShader->uniform("uPlanesNightColor", mSettings.planesNightTint);

            
            gl::drawSolidRect(Rectf(0,0,size.x, size.y));
        
            
            mMapTexture->unbind();
            planesTexture->unbind();
            terminatorTexture->unbind();
            
        }else{
            gl::ScopedGlslProg prog( mDefaultShader );

            mDefaultShader->uniform("uTexPlanes", 1);
//            mDefaultShader->uniform("uTime", float(ci::app::getElapsedSeconds()) * mSettings.speedScale );
            mDefaultShader->uniform("uMapColor", mSettings.mapNightTint);

            auto planesTexture = mPlanesFbo->getColorTexture();
            mMapTexture->bind(0);
            planesTexture->bind(1);
            
            gl::drawSolidRect(Rectf(0,0,size.x, size.y) );
            
            mMapTexture->unbind();
            planesTexture->unbind();

        }
    }
    
}
