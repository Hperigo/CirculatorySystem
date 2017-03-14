#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"

// 3D
#include "cinder/CameraUi.h"
#include "cinder/Log.h"

// Cinder
#include "cinder/Json.h"
#include "cinder/Timer.h"


// mine
#include "FAClient.hpp"
#include "Plane.hpp"
using namespace ci;
using namespace ci::app;
using namespace std;


class CirculatorySysApp : public App {
  public:
	void setup() override;
	void mouseDown( MouseEvent event ) override;
    void mouseMove( MouseEvent event ) override;
	void update() override;
	void draw() override;
    
    
    vec2 latLongToXY( vec2 point );
    
    CameraPersp			_CameraDebug;
    CameraUi			_CamUi;
    
    gl::BatchRef        _wireGrid;
    
    
    gl::TextureRef m2DMap;
    vec2 mLastPoint;

    std::shared_ptr<FlightAware::FAClient> mFAClientRef;
    
    int once = 0;
    unsigned long int numberOfCalls = 0;
    
    
    std::vector<vec2> points;
    std::vector<string> commands;
    
//    std::set<csys::Plane> mPlanes;
    std::map<std::string , csys::Plane> mPlanes;

    
    vec2 mMousePos;
    
    std::time_t globalTime;
};


vec2 CirculatorySysApp::latLongToXY(vec2 point){
    
    float width = m2DMap->getWidth();
    float height = m2DMap->getHeight();
    
    float lon = point.y;
    float lat = point.x;
    
    
    auto x = ((lon + 180) * (width  / 360));
    auto y = (((lat * -1) + 90) * (height/ 180));
    
    return vec2(x,y);
}


void CirculatorySysApp::setup()
{
    
    
    
    auto keyBuffer = loadAsset( "initcommand.cskey" )->getBuffer();
    
    std::string keyString = std::string( static_cast<char *>( keyBuffer->getData() ), keyBuffer->getSize() ) + "\n";
    
    
    globalTime = std::time(nullptr);
    
    
    m2DMap  = gl::Texture::create(loadImage( loadAsset( "map.png")  ) );
    getWindow()->setSize(m2DMap->getSize());
    
    
    auto settings = FlightAware::FAClient::Settings(App::io_service());
    settings.setInitCommand(keyString);
    
    
    
    mFAClientRef = std::make_shared<FlightAware::FAClient>( settings);
    mFAClientRef->connect();

    mFAClientRef->onReplyCallback = [&](const std::string& s){
        commands.push_back(s);
    };
    
    mFAClientRef->onErrorCallback = [&](const std::string& error){
        CI_LOG_E( "ssl error, " << error );
    };
    
    // ---- Debug ---
    
//    _CameraDebug.lookAt( vec3( 2.0f, 3.0f, 1.0f ), vec3( 0 ) );
//    _CameraDebug.setPerspective( 40.0f, getWindowAspectRatio(), 0.01f, 100.0f );
//    _CamUi = CameraUi( &_CameraDebug, getWindow() );
//    
//    auto colorShader = gl::getStockShader( gl::ShaderDef().color() );
//    _wireGrid = gl::Batch::create( geom::WirePlane().size( vec2( 10 ) ).subdivisions( ivec2( 10 ) ), colorShader );
//    
//    getWindow()->getSignalResize().connect([&]{
//        
//    });


}

void CirculatorySysApp::mouseDown( MouseEvent event )
{
 
    std::time_t now = std::time(nullptr);
    
    console() << now - globalTime << std::endl;
    globalTime = now;
}

void CirculatorySysApp::mouseMove( MouseEvent event )
{
    mMousePos = event.getPos();
}

void CirculatorySysApp::update()
{
    
    getWindow()->setTitle( to_string(getAverageFps()) + " | " + to_string( mPlanes.size() ) );

    for(auto& s : commands){
        
        auto j = JsonTree(s);
        auto FA_type = j["type"].getValue<std::string>();
        
        
        if(FA_type == "position"){
            vec2 p;
            p.x = atof( j["lat"].getValue<std::string>().c_str() ) ;
            p.y = atof( j["lon"].getValue<std::string>().c_str() ) ;
            
            
            if( !j.hasChild("reg") ){
                continue;
            }
            
            
            auto screenPos = latLongToXY( p );
            auto key = j["reg"].getValue<std::string>();
            
            auto found = mPlanes.find( key );
            
            
            if( found !=  mPlanes.end() ){
                found->second.setPosition(screenPos);
            }else{
                mPlanes[key] = csys::Plane(key, screenPos);
            }
            

        
        }
    }
    
    commands.clear();
    
    
    
    for(auto p = mPlanes.begin(); p != mPlanes.end();){
        auto plane = p->second;
        
        int timeDiff = plane.getLastUpdateTime() - globalTime;
        if(timeDiff > 1500){ //
            console() << "Erasing plane " << plane.getKey()  <<  std::endl;;
            p = mPlanes.erase(p);
        }else{
          
        }
          ++p;
    }
    
}

void CirculatorySysApp::draw()
{

    if( once  < 10){
        
        gl::clearColor( ColorA(0,0,0,1));
        gl::clear();
 
        gl::color( ColorA(0.4, 0.1,0.1, 1.0) );
        gl::draw(m2DMap);
        
        once++;
    
    }
    
    if( getElapsedFrames() % 10800 == 0){
        gl::color( ColorA(0.4, 0.1,0.1, 0.01) );
        gl::draw(m2DMap);
    }

    
    gl::color( ColorA(1.0f, 1.0f, 1.0f, 0.1f) ) ;
    
    int neededRedraw = 0;
    for(auto& p :mPlanes){
        
        auto& plane = p.second;
        
        if(plane.draw()){
            neededRedraw++;
        }
    }
    
   // console() << neededRedraw << " planes needed redraw and " << mPlanes.size() - neededRedraw << " did not" << std::endl;
}

CINDER_APP( CirculatorySysApp, RendererGl( RendererGl::Options().msaa(4) ) )

