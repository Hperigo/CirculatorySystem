#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"

// blocks
#include "CinderImGui.h"
#include "Syphon.h"

#include "CinderCereal.h"
#include <cereal/archives/json.hpp>
#include <cereal/types/vector.hpp>

// 3D
#include "cinder/CameraUi.h"
#include "cinder/Log.h"

// Cinder
#include "cinder/Json.h"
#include "cinder/Timer.h"
#include "cinder/params/Params.h"


// project
#include "GeoUtils.h"

#include "FAClient.hpp"
#include "Plane.hpp"
#include "PlaneManager.hpp"
#include "Render.hpp"



using namespace ci;
using namespace ci::app;
using namespace std;


class CirculatorySysApp : public App {
  public:

    CirculatorySysApp();
    ~CirculatorySysApp();
    
    void prepareSetup(app::App::Settings* settings );
	void setup() override;
    
    
    void keyUp( KeyEvent event ) override;
    
    
	void mouseDown( MouseEvent event ) override;
    void mouseMove( MouseEvent event ) override;
	
    void renderFbo();
    void draw() override;
//
    void update() override;
    void updateFromQuery();
    void updateFromDB();
    
    
    void manageFullScreen(bool toogle);

    
    CameraPersp			_CameraDebug;
    CameraUi			_CamUi;
    
    gl::BatchRef        _wireGrid;
    
    gl::BatchRef        mSphere;
    
    std::shared_ptr<FlightAware::FAClient> mFAClientRef;
    csys::PlaneManager* mPlaneManager;

    // wat
    int once = 0;
    unsigned long int numberOfCalls = 0;

    reza::syphon::Server mSyphonServer;

    vec2 mMousePos;
        
    

    bool doQuery = false;
    
    std::vector<vec2> points;
    std::vector<string> commands;
    
    
    csys::Render::RenderSettings* mSettings;
    csys::Render mRender;
    

    // Settings -----
    class MainSettings : public csys::Settings{
        
    public:
        MainSettings(){
            

            
        };
        virtual ~MainSettings(){
            
        }
        
        
        void drawUi() override{
            
            ui::Checkbox("Draw sphere", &drawSphere);
            
            ui::Checkbox("Start Fullscreen", &startFullScreen);
            if(ui::Button("Set Fullscreen")){
                app = (CirculatorySysApp*) app::App::get();
//                app->setFullScreen( !app->isFullScreen() );
                app->manageFullScreen(!app->isFullScreen() );
                
            }
            
            if(ui::Button("Save")){
                save();
            }
            
            if(ui::Button("Load")){
                load();
            }
            
            ui::Spacing();
            ui::Spacing();
            
            if(ui::Button("QUIT")){
                app = (CirculatorySysApp*) app::App::get();
                
                app->quit();
            }
            
            

            
        }
        
        void save() override{
            app = (CirculatorySysApp*) app::App::get();
            
            try{
                auto path = getResourcePath("out_main.json").string();
                
                std::ofstream outFile(path);
                cereal::JSONOutputArchive archive(outFile);
                
                CI_LOG_I("Saving..." << startFullScreen);
                
                archive( drawSphere, startFullScreen );
                
            }catch(std::exception &e){
                
                CI_LOG_EXCEPTION("Error saving: ", e);
            }
            
        }
        
        void load() override{
            
            try{
                
                auto path = getResourcePath("out_main.json").string();
                std::ifstream inFile(path);
                cereal::JSONInputArchive archive(inFile);
                
                
                archive(drawSphere, startFullScreen);
                
                CI_LOG_I("loading..." << startFullScreen);
                
            }catch(std::exception &e){
                
                CI_LOG_EXCEPTION("Error loading: ", e );
                
            }
        }
        
        bool drawSphere = false;
        bool startFullScreen = false;
        
        CirculatorySysApp* app;
        
        
        
    };
    
    
    MainSettings mMainSettings;
    friend class MainSettings;
    
};


CirculatorySysApp::~CirculatorySysApp(){
    
    console() << "deconstructor"<< std::endl;
    
}


CirculatorySysApp::CirculatorySysApp(){
    
    
}


void CirculatorySysApp::prepareSetup(app::AppBase::Settings* settings ){
    
 
    
}


void CirculatorySysApp::setup()
{

    
    
    ui::initialize();
    
    mSettings = &mRender.mSettings;
    mMainSettings.load();
    
    mPlaneManager = &csys::PlaneManager::instance();
    mPlaneManager->initFromDB();
    
    mPlaneManager->setColorMap(  loadImage(loadResource("color_map.png"))  );
    
    
    getWindow()->setSize(mSettings->windowSize);
    
    
    mRender.setup( {1280, 640} );
    
    mSyphonServer.setName("Csys");
    
    CI_LOG_I( "window size: "  << getWindow()->getSize() );

    if( doQuery ){
        
        auto keyBuffer = loadResource( "initcommand.cskey" )->getBuffer();
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

//    setFrameRate(30);
    
    gl::enableVerticalSync(false);
    
    
    // ---- Debug ---
    
    _CameraDebug.lookAt( vec3( 2.0f, 3.0f, 1.0f ), vec3( 0 ) );
    _CameraDebug.setPerspective( 40.0f, getWindowAspectRatio(), 0.01f, 100.0f );
    _CamUi = CameraUi( &_CameraDebug, getWindow() );
    
    auto colorShader = gl::getStockShader( gl::ShaderDef().color() );
    _wireGrid = gl::Batch::create( geom::WirePlane().size( vec2( 10 ) ).subdivisions( ivec2( 10 ) ), colorShader );
    
    
    
    mSphere = gl::Batch::create(geom::Sphere().subdivisions(32).radius(3), gl::getStockShader( gl::ShaderDef().color().texture() ));
    
    
    
    if(mMainSettings.startFullScreen == true){
        
        setFullScreen(true);
    }
}


void CirculatorySysApp::manageFullScreen(bool toogle){
    
    setFullScreen(toogle);
    
    if(toogle == true){
        hideCursor();
    }
    else{
        showCursor();
    }
    
    
}

void CirculatorySysApp::mouseDown( MouseEvent event )
{
 
    
}

void CirculatorySysApp::mouseMove( MouseEvent event )
{
    mMousePos = event.getPos();    
}


void CirculatorySysApp::keyUp(cinder::app::KeyEvent event){
    
    if(event.getChar() == 'f' ){
        manageFullScreen( !isFullScreen() );
    }
    
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
    

    if(doQuery){
        updateFromQuery();
    }
    
//    if( mDatabase.isQueryAvailable() &&  mPlanes == nullptr){
//        console() << "DONEE!" << std::endl;
//        setPlanes();
//    }
    
    
    mPlaneManager->update();
    
    
    //
    mRender.renderPlaneFbo();
    mRender.renderTerminatorFbo();
    mRender.renderBlurFbo();
    mRender.renderPlaneEdgeBlend();
    mRender.renderCompose();
     
    
}


void CirculatorySysApp::renderFbo(){
    
}

void CirculatorySysApp::draw()
{
    
    
    // set matrices
    gl::ScopedViewport v( vec2(0), getWindowSize() );
    
    // clear
    gl::clear( Color(0.3, 0.3, 0.3) );
    gl::enableAlphaBlending();
    
    auto tex = mRender.mComposeFbo->getColorTexture();


    if( mMainSettings.drawSphere ){
        
        gl::setMatrices(_CameraDebug);
        gl::enableDepth(true);

        _wireGrid->draw();
        
        gl::ScopedTextureBind t(tex);
        mSphere->draw();
        
        gl::enableDepth(false);
    }else{
        
        
        vec2 syphonSize = { 2160,1080 };
        
        {
         
            gl::ScopedViewport v( vec2(0), syphonSize );
            gl::setMatricesWindow(syphonSize.x, syphonSize.y);
            
            //        mSyphonServer.bind( {1280, 640} );
            
            mSyphonServer.bind( syphonSize );
            gl::draw( tex, Rectf(0,0, syphonSize.x, syphonSize.y) );
            mSyphonServer.unbind();
        }
    
        {
            gl::ScopedViewport v( vec2(0), getWindowSize() );

            gl::setMatricesWindow(getWindowWidth(), getWindowHeight());
            gl::draw( tex, Rectf(0,0, syphonSize.x, syphonSize.y) );
        }

        
    }
    

    {
        vec2 size = { 1000, 500 };
        auto shader = gl::getStockShader( gl::ShaderDef().color().texture() );
        gl::ScopedGlslProg prog(shader);
    }

    
    if(!isFullScreen()){
    
        // DRAW UI ----------
        {
            ui::ScopedWindow w("Geral");
            ui::LabelText(to_string(getAverageFps()).c_str(), "FPS: ");
            mMainSettings.drawUi();
        }
        
        {
            ui::ScopedWindow w("Plane Manager");
            mPlaneManager->drawUi();
        }
        
        {
            ui::ScopedWindow w("Render settings");
            mSettings->drawUi();
        }

    }
    
}

CINDER_APP( CirculatorySysApp, RendererGl( RendererGl::Options().msaa(4) ) );

