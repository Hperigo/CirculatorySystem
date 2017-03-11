#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"

// 3D
#include "cinder/CameraUi.h"
#include "cinder/Log.h"



using namespace ci;
using namespace ci::app;
using namespace std;

class CirculatorySysApp : public App {
  public:
	void setup() override;
	void mouseDown( MouseEvent event ) override;
	void update() override;
	void draw() override;
    
    
    CameraPersp			_CameraDebug;
    CameraUi			_CamUi;
    
    gl::BatchRef        _wireGrid;

};

void CirculatorySysApp::setup()
{
    
    
    
    
    // ---- Debug ---
    
    _CameraDebug.lookAt( vec3( 2.0f, 3.0f, 1.0f ), vec3( 0 ) );
    _CameraDebug.setPerspective( 40.0f, getWindowAspectRatio(), 0.01f, 100.0f );
    _CamUi = CameraUi( &_CameraDebug, getWindow() );
    
    auto colorShader = gl::getStockShader( gl::ShaderDef().color() );
    _wireGrid = gl::Batch::create( geom::WirePlane().size( vec2( 10 ) ).subdivisions( ivec2( 10 ) ), colorShader );
    
    getWindow()->getSignalResize().connect([&]{
        
    });
    
}

void CirculatorySysApp::mouseDown( MouseEvent event )
{
    
    
}

void CirculatorySysApp::update()
{
    
    
}

void CirculatorySysApp::draw()
{
	gl::clear( Color( 0, 0, 0 ) );
    
        
    gl::setMatrices( _CameraDebug );
    gl::drawCoordinateFrame();
    
    gl::ScopedColor color( Color::gray( 0.2f ) );
    _wireGrid->draw();
}

CINDER_APP( CirculatorySysApp, RendererGl )

