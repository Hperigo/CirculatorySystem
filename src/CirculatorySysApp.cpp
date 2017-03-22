#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"


#include "CinderImGui.h"

// 3D
#include "cinder/CameraUi.h"
#include "cinder/Log.h"

// Cinder
#include "cinder/Json.h"
#include "cinder/Timer.h"
#include "cinder/params/Params.h"

// mine
#include "GeoUtils.h"

#include "FAClient.hpp"
#include "Plane.hpp"
#include "Database.hpp"


// mongodb

using namespace ci;
using namespace ci::app;
using namespace std;


class CirculatorySysApp : public App {
  public:
	void setup() override;
	void mouseDown( MouseEvent event ) override;
    void mouseMove( MouseEvent event ) override;
	
    void draw() override;
    
    void update() override;
    void updateFromQuery();
    void updateFromDB();

    
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
    std::map<std::string , csys::Plane> mDeletedPlanes;

    
    vec2 mMousePos;
    
    std::time_t globalTime;
    
    csys::Database mDatabase;
    
    std::shared_ptr<csys::Plane> activePlane;
    
    
    bool doQuery = true;
    
    params::InterfaceGlRef mParams;
    
    bool whatPoint = false;
    vec2 pointA;
    vec2 pointB;
};




void CirculatorySysApp::setup()
{
    
    ui::initialize();
    
    mDatabase.setup();
    
    
    auto keyBuffer = loadAsset( "initcommand.cskey" )->getBuffer();
    
    std::string keyString = std::string( static_cast<char *>( keyBuffer->getData() ), keyBuffer->getSize() ) + "\n";
    
    
    globalTime = std::time(nullptr);
    
    
    m2DMap  = gl::Texture::create(loadImage( loadAsset( "map.png")  ) );
    getWindow()->setSize(m2DMap->getSize());
    
    
    console() << "size: "  << getWindow()->getSize() << std::endl;
    
    if( doQuery ){
        
        
        
        auto settings = FlightAware::FAClient::Settings(App::io_service());
        settings.setInitCommand(keyString);
        
        
        
        mFAClientRef = std::make_shared<FlightAware::FAClient>( settings);
        mFAClientRef->connect();

        mFAClientRef->setReplyCallback( [&](const std::string& s){
            commands.push_back(s);
        });
        
        mFAClientRef->setErrorCallback( [&](const std::string& error){
            CI_LOG_E( "ssl error, " << error );
        });
        
    }
    
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
//    mParams = params::InterfaceGl::create("db", { 240, 300} );
}

void CirculatorySysApp::mouseDown( MouseEvent event )
{
 
    if(whatPoint){
        pointA = event.getPos();
    }
    else{
        pointB = event.getPos();
    }
    
    whatPoint = !whatPoint;
    
}

void CirculatorySysApp::mouseMove( MouseEvent event )
{
    mMousePos = event.getPos();
    

    
}

void CirculatorySysApp::updateFromDB(){
    
    
    
    
    
}


void CirculatorySysApp::updateFromQuery(){
    
    for(auto& s : commands){
        
        auto j = JsonTree(s);
        auto FA_type = j["type"].getValue<std::string>();
        
        
        if(FA_type == "position"){
            vec2 p;
            p.x = atof( j["lat"].getValue<std::string>().c_str() ) ;
            p.y = atof( j["lon"].getValue<std::string>().c_str() ) ;
            
            
            if( !j.hasChild("id") ){
                continue;
            }
            
            
            mDatabase.appendData( j );
            
        }
    }
    
    commands.clear();
}

void CirculatorySysApp::update()
{
    
    getWindow()->setTitle( to_string(getAverageFps()) + " | " + to_string( mDatabase.getPlanes().size() ) );
    
    globalTime = std::time(nullptr);


    if(getElapsedFrames() % 10 != 0){
     return;
    }


    
    if(doQuery){
        updateFromQuery();
    }
    

    
    
}

void CirculatorySysApp::draw()
{
    
    if(getElapsedFrames() < 10){
        

        gl::clear(  ColorA(0,0,0,1) );
 
        gl::color( ColorA(0.4, 0.1,0.1, 1.0) );
        gl::draw(m2DMap);
    
        gl::color( ColorA(1.0f, 1.0f, 1.0f, 0.7f) ) ;
        
    }
    
    {
//        ui::ScopedWindow w("Flight Query");
//        
//        std::string buff = activePlane->getKey();
//        ui::InputText("ID", &buff);
//        
//        ui::Text("id" );
        
    }
    
    
//    int neededRedraw = 0;
    for(auto& p : mDatabase.getPlanes() ){
        
        auto& plane = p.second;
        
        plane.draw();
    }

    
    //-23.533773
    //Longitude	-46.625290
    

//    auto pos = csys::geo::latLongToCartesian(m2DMap->getSize(), {0,0} );
//    gl::drawSolidCircle(pos, 5);
//    
//    pos = csys::geo::latLongToCartesian(m2DMap->getSize(), {-23.533773, -46.625290} );
//    gl::drawSolidCircle(pos, 5);
//    
//    
////    Latitude	51.509865
////    Longitude	-0.118092
//    pos = csys::geo::latLongToCartesian(m2DMap->getSize(), {51.509865, -0.118092} );
//    gl::drawSolidCircle(pos, 5);
//    
//    
//    auto latLong = csys::geo::cartesianToLatLong(m2DMap->getSize(), mMousePos);
//    console() << latLong << endl;
//
//    
//    pos = csys::geo::latLongToCartesian(m2DMap->getSize(), latLong );
//    gl::drawSolidCircle(pos, 8);
    
//    auto positions = activePlane->getPositions();
//    
//    for (int i = 1; i < positions.size(); i++) {
//        
//        
//        auto a = positions[i];
//        auto b = positions[i-1];
//        
//        gl::drawLine(a, b);
//        
//    }
//    
    
    
//    auto coord = csys::geo::cartesianToLatLong(m2DMap->getSize(), mMousePos);
//    coord =  csys::geo::latLongToCartesian(m2DMap->getSize(),  coord);
//    
//    
//    gl::color( 0.0f, 0.0f, 1.0f );
//    gl::drawSolidCircle( pointA, 20);
//    
//    gl::color(1.0f, 1.0f, 0.0f  );
//    gl::drawSolidCircle( pointB, 10);
//    
//    console() << csys::geo::distanceLatLong( csys::geo::cartesianToLatLong(m2DMap->getSize(), pointA) , csys::geo::cartesianToLatLong(m2DMap->getSize(), pointB)) <<std::endl;
}

CINDER_APP( CirculatorySysApp, RendererGl( RendererGl::Options().msaa(4) ) )

