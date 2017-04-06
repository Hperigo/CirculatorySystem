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
#include "PlaneManager.hpp"


using namespace ci;
using namespace ci::app;
using namespace std;


template<typename T>
T vectorLerp( const std::vector<T>& values, float t)
{
    
    
    int lowerIndex = std::floor(t);
    int upperIndex = std::ceil(t);

    float f;
    float scaledTime = modf(t, &f);

    T value = ci::lerp<T>(values[lowerIndex], values[upperIndex], scaledTime);
    
    return value;
}


class CirculatorySysApp : public App {
  public:

    CirculatorySysApp();
    void prepareSetup(app::App::Settings* settings );
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
    
    void setPlanes();
    
//    std::set<csys::Plane> mPlanes;
    std::map<std::string , csys::PlaneRef>* mPlanes = nullptr;
    std::vector<csys::PlaneRef> mSortedPlanes;

    
    vec2 mMousePos;
    
    
    std::time_t initialTime;
    std::time_t endTime;
    
    ci::Timer mGlobalTimer;
    
    csys::PlaneManager mPlaneManager;
    
    
    std::shared_ptr<csys::Plane> activePlane;
    
    
    bool doQuery = false;
    
    params::InterfaceGlRef mParams;
    
    bool whatPoint = false;
    vec2 pointA;
    vec2 pointB;
};


CirculatorySysApp::CirculatorySysApp(){
    

    
}


void CirculatorySysApp::prepareSetup(app::AppBase::Settings* settings ){
    
 
    
}


void CirculatorySysApp::setup()
{
    
    ui::initialize();
    
//    mDatabase.setup();
//
//    mDatabase.queryEveryPlane();

    
    mPlaneManager.initFromDB();
    
    
    m2DMap  = gl::Texture::create(loadImage( loadAsset( "map_lat_long.jpg")  ) );
    getWindow()->setSize(m2DMap->getSize());
    
    
    console() << "size: "  << getWindow()->getSize() << std::endl;
    

    std::vector<vec2> vecs{ vec2(0), vec2(1), vec2(2), vec2(4) };

    
    
    
    if( doQuery ){
        
        auto keyBuffer = loadAsset( "initcommand.cskey" )->getBuffer();
        std::string keyString = std::string( static_cast<char *>( keyBuffer->getData() ), keyBuffer->getSize() ) + "\n";
        
        
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
    
    //activePlane = mDatabase.getPlane("SIA32-1489972800-schedule-0001");
    
    
    setFrameRate(30);
    
    
    CI_LOG_V( "initialTime: " << initialTime << " end time " << endTime );

    

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
    
    
//    mGlobalTimer.start();
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
            
            
//            mDatabase.appendData( j );
            
        }
    }
    
    commands.clear();
}

void CirculatorySysApp::update()
{
    
//    getWindow()->setTitle( to_string(getAverageFps()) + " | " + to_string( mDatabase.getPlanes().size() ) + " \\ " +  to_string(getElapsedFrames()) );
    
    
    if(doQuery){
        updateFromQuery();
    }
    
//    if( mDatabase.isQueryAvailable() &&  mPlanes == nullptr){
//        console() << "DONEE!" << std::endl;
//        setPlanes();
//    }
    
    
    mPlaneManager.update();
}

void CirculatorySysApp::draw()
{
    
    if(getElapsedFrames() < 10){
    
        gl::clear(  ColorA(0,0,0, 0.000f) );
        gl::color(1.0f,0.1f, 0.1f, 1.0f);
        gl::draw(m2DMap);
        
    }
    
    gl::enableAlphaBlending();
    gl::color(1.0f,0.1f, 0.1f, 0.05f);
    gl::draw(m2DMap);
    
    
    
    
    // Draw planes ----
    long timeCursor = mPlaneManager.getGlobalTime() * 500;
    if(timeCursor > (endTime - initialTime ) ){
        mGlobalTimer.start();
    }
    

    
    gl::enableAdditiveBlending();
    gl::begin(GL_LINES);
    
    int unbornPlanes = 0;
    
    auto sortedPlanes = mPlaneManager.getSortedPlanes();
    for(auto& plane : sortedPlanes){
        
        
        if(!plane->isActive()){
            continue;
        }
        
        unbornPlanes++;
        
        auto positions = plane->getPositions();
        float normalizedIndex = ci::constrain<float>(   plane->getNormalizedTime() *  (positions.size() - 1 ), 0, positions.size() - 1 );
        
        auto latA = vectorLerp(positions, normalizedIndex);
        auto pointA = csys::geo::latLongToCartesian(  m2DMap->getSize() , latA);
        
        gl::color( ColorA(0.6, 0.6, 0.8, 0.05) );
        gl::vertex(pointA);
        if(normalizedIndex > 1){
            
            auto latB = vectorLerp(positions, normalizedIndex - 1);
            
            auto geoDistance = csys::geo::distanceLatLong(latA, latB);
            
            auto pointB = csys::geo::latLongToCartesian( m2DMap->getSize() , latB);
            
            if( geoDistance < 500 && glm::distance(pointA, pointB) < 300){
                
                gl::color( ColorA(1.0, 1.0, 1.0, 0.01) );
                gl::vertex(pointB);
                
            }else{
                
                gl::color( ColorA(1.0, 1.0, 1.0, 1.0) );
                gl::vertex(pointA);
            }
            
        }else{
            
            gl::color( ColorA(1.0, 1.0, 1.0, 1.0) );
            gl::vertex(pointA);
        }
        
    }
    gl::end();
    
    
    
    
    // DRAW UI ----------
    {
        ui::ScopedWindow w("General");
        ui::LabelText(to_string(getAverageFps()).c_str(), "FPS: ");
        if(ui::Button("make query")){
            
        }
    }
    
    {
        ui::ScopedWindow w("Plane Manager");
        mPlaneManager.drawUi();
    }

    
}

CINDER_APP( CirculatorySysApp, RendererGl( RendererGl::Options().msaa(8) ) );

