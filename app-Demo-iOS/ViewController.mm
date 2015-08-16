//
//  ViewController.m
//  comgr
//
//  Created by Marcus Hudritsch on 30.11.11.
//  Copyright (c) 2011 __MyCompanyName__. All rights reserved.
//

#import "ViewController.h"

// The only C-interface to include for the SceneLibrary
#include <SLInterface.h>
#include <SLImage.h>
#include <mach/mach_time.h>

//-----------------------------------------------------------------------------
// C-Prototypes
float GetSeconds();
SLbool onPaintRTGL();
//-----------------------------------------------------------------------------
// Global pointer to the GLView instance that can be accessed by onPaintRTGL
GLKView* myView = 0;
//-----------------------------------------------------------------------------
// Global SLSceneView handle
int svIndex = 0;
//-----------------------------------------------------------------------------
// Global screen scale (2.0 for retina, 1.0 else)
float screenScale = 1.0f;

//-----------------------------------------------------------------------------
// C-Function used as C-function callback for raytracing update
SLbool onPaintRTGL()
{
   [myView display];
   return true;
}
//-----------------------------------------------------------------------------
/*!
 Returns the absolute time in seconds since the system started. It is based
 on a CPU clock counter.
 */
float GetSeconds()
{
    static mach_timebase_info_data_t info;
    mach_timebase_info(&info);
    uint64_t now = mach_absolute_time();
    now *= info.numer;
    now /= info.denom;
    double sec = (double)now / 1000000000.0;
    return (float)sec;
}
//-----------------------------------------------------------------------------
@interface ViewController ()
{
    SLfloat  m_lastFrameTimeSec;  //!< Timestamp for passing highres time
    SLfloat  m_lastTouchTimeSec;  //!< Frame time of the last touch event
    SLfloat  m_lastTouchDownSec;  //!< Time of last touch down
    SLint    m_touchDowns;        //!< No. of finger touchdowns
    
    // Video stuff
    AVCaptureSession*           m_avSession;            //!< Audio video session
    NSString*                   m_avSessionPreset;      //!< Session name
    //CVOpenGLESTextureRef        m_texture;
    //CVOpenGLESTextureCacheRef   m_videoTextureCache;
    bool                        m_lastVideoImageIsConsumed;
}
@property (strong, nonatomic) EAGLContext *context;
@end

@implementation ViewController

@synthesize context = _context;

- (void)dealloc
{
   [_context release];
   [super dealloc];
}
//-----------------------------------------------------------------------------
- (void)viewDidLoad
{
    [super viewDidLoad];
   
    self.context = [[[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES2] autorelease];
   
    if (!self.context) NSLog(@"Failed to create ES context");
   
    myView = (GLKView *)self.view;
    myView.context = self.context;
    myView.drawableDepthFormat = GLKViewDrawableDepthFormat24;
   
    if([UIDevice currentDevice].multitaskingSupported)
       myView.drawableMultisample = GLKViewDrawableMultisample4X;
   
    self.preferredFramesPerSecond = 30;
    self.view.multipleTouchEnabled = true;
    m_touchDowns = 0;
   
    //[self setupGL];
    [EAGLContext setCurrentContext:self.context];
   
    // Get the main bundle path and pass it the SLTexture and SLShaderProg
    // This will be the default storage location for textures and shaders
    NSString* bundlePath =[[NSBundle mainBundle] resourcePath];
    string pathUTF8 = [bundlePath UTF8String];
    pathUTF8 += "/";
    SLVstring cmdLineArgs;
   
    // determine device pixel ratio and dots per inch
    screenScale = [UIScreen mainScreen].scale;
    float dpi;
    if (UI_USER_INTERFACE_IDIOM() == UIUserInterfaceIdiomPad)
        dpi = 132 * screenScale;
    else if (UI_USER_INTERFACE_IDIOM() == UIUserInterfaceIdiomPhone)
        dpi = 163 * screenScale;
    else
        dpi = 160 * screenScale;
   
    slCreateScene(pathUTF8,
                  pathUTF8,
                  pathUTF8);
   
    svIndex = slCreateSceneView(self.view.bounds.size.width * screenScale,
                                self.view.bounds.size.height * screenScale,
                                dpi,
                                cmdSceneSmallTest,
                                cmdLineArgs,
                                (void*)&onPaintRTGL,
                                0,
                                0,
                                0);
    
    [self setupAVCapture];
}
//-----------------------------------------------------------------------------
- (void)viewDidUnload
{
    [super viewDidUnload];
   
    slTerminate();
   
    if ([EAGLContext currentContext] == self.context)
    {   [EAGLContext setCurrentContext:nil];
    }
    self.context = nil;
}
//-----------------------------------------------------------------------------
- (void)didReceiveMemoryWarning
{
    [super didReceiveMemoryWarning];
    // Release any cached data, images, etc. that aren't in use.
}
//-----------------------------------------------------------------------------
- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation
{
    // Return YES for supported orientations
    if ([[UIDevice currentDevice] userInterfaceIdiom] == UIUserInterfaceIdiomPhone) {
        return (interfaceOrientation != UIInterfaceOrientationPortraitUpsideDown);
    } else {
        return YES;
    }
}
//-----------------------------------------------------------------------------
- (void)update
{
   slResize(svIndex, self.view.bounds.size.width  * screenScale,
                     self.view.bounds.size.height * screenScale);
}
//-----------------------------------------------------------------------------
- (void)glkView:(GLKView *)view drawInRect:(CGRect)rect
{
    slUpdateAndPaint(svIndex);
    m_lastVideoImageIsConsumed = true;
}
//-----------------------------------------------------------------------------
// touchesBegan receives the finger thouch down events
- (void)touchesBegan:(NSSet *)touches withEvent:(UIEvent *) event
{
   NSArray* myTouches = [touches allObjects];
   UITouch* touch1 = [myTouches objectAtIndex:0];
   CGPoint pos1 = [touch1 locationInView:touch1.view];
   pos1.x *= screenScale;
   pos1.y *= screenScale;
   float touchDownNowSec = GetSeconds();
   
   // end touch actions on sequential finger touch downs
   if (m_touchDowns > 0)
   {  if (m_touchDowns == 1)
      slMouseUp(svIndex, ButtonLeft, pos1.x, pos1.y, KeyNone);
      if (m_touchDowns == 2)
      slTouch2Up(svIndex, 0, 0, 0, 0);
      // Reset touch counter if last touch event is older than a second.
      // This resolves the problem off loosing track in touch counting e.g.
      // when somebody touches with the flat hand.
      if (m_lastTouchTimeSec < (m_lastFrameTimeSec - 2.0f))
      {  m_touchDowns = 0;
      }
   }
   m_touchDowns += [touches count];
   //printf("Begin tD: %d, touches count: %d\n", m_touchDowns, [touches count]);
   
   if (m_touchDowns == 1 && [touches count] == 1)
   {  if (touchDownNowSec - m_lastTouchDownSec < 0.3f)
      slDoubleClick(svIndex, ButtonLeft, pos1.x, pos1.y, KeyNone);
      else
      slMouseDown(svIndex, ButtonLeft, pos1.x, pos1.y, KeyNone);
   } else
   if (m_touchDowns == 2)
   {  if ([touches count] == 2)
      {  UITouch* touch2 = [myTouches objectAtIndex:1];
         CGPoint pos2 = [touch2 locationInView:touch2.view];
         pos2.x *= screenScale;
         pos2.y *= screenScale;
         slTouch2Down(svIndex, pos1.x, pos1.y, pos2.x, pos2.y);
      } else
      if ([touches count] == 1) // delayed 2nd finger touch
      {  slTouch2Down(svIndex, 0, 0, 0, 0);
      }
   }
   
   m_lastTouchTimeSec = m_lastTouchDownSec = touchDownNowSec;
}
//-----------------------------------------------------------------------------
// touchesMoved receives the finger move events
- (void)touchesMoved:(NSSet *)touches withEvent:(UIEvent *)event
{
    NSArray* myTouches = [touches allObjects];
    UITouch* touch1 = [myTouches objectAtIndex:0];
    CGPoint pos1 = [touch1 locationInView:touch1.view];
    pos1.x *= screenScale;
    pos1.y *= screenScale;
   
    if (m_touchDowns == 1 && [touches count] == 1)
    {   slMouseMove(svIndex, pos1.x, pos1.y);
    } else
    if (m_touchDowns == 2 && [touches count] == 2)
    {   UITouch* touch2 = [myTouches objectAtIndex:1];
        CGPoint pos2 = [touch2 locationInView:touch2.view];
        pos2.x *= screenScale;
        pos2.y *= screenScale;
        slTouch2Move(svIndex, pos1.x, pos1.y, pos2.x, pos2.y);
    }
   
    m_lastTouchTimeSec = m_lastFrameTimeSec;
}
//-----------------------------------------------------------------------------
// touchesEnded receives the finger thouch release events
- (void)touchesEnded:(NSSet *)touches withEvent:(UIEvent *)event
{
    NSArray* myTouches = [touches allObjects];
    UITouch* touch1 = [myTouches objectAtIndex:0];
    CGPoint pos1 = [touch1 locationInView:touch1.view];
    pos1.x *= screenScale;
    pos1.y *= screenScale;
   
    if (m_touchDowns == 1 || [touches count] == 1)
    {   slMouseUp(svIndex, ButtonLeft, pos1.x, pos1.y, KeyNone);
    } else
    if (m_touchDowns == 2 && [touches count] >= 2)
    {   UITouch* touch2 = [myTouches objectAtIndex:1];
        CGPoint pos2 = [touch2 locationInView:touch2.view];
        pos2.x *= screenScale;
        pos2.y *= screenScale;
        slTouch2Up(svIndex, pos1.x, pos1.y, pos2.x, pos2.y);
    }
    m_touchDowns -= (int)[touches count];
    if (m_touchDowns < 0) m_touchDowns = 0;
   
    //printf("End   tD: %d, touches count: %d\n", m_touchDowns, [touches count]);
   
    m_lastTouchTimeSec = m_lastFrameTimeSec;
}
//-----------------------------------------------------------------------------
// touchesCancle receives the cancle event on an iPhone call
- (void)touchesCancle:(NSSet *)touches withEvent:(UIEvent *)event
{
    NSArray* myTouches = [touches allObjects];
    UITouch* touch1 = [myTouches objectAtIndex:0];
    CGPoint pos1 = [touch1 locationInView:touch1.view];
   
    if (m_touchDowns == 1 || [touches count] == 1)
    {   slMouseUp(svIndex, ButtonLeft, pos1.x, pos1.y, KeyNone);
    } else
    if (m_touchDowns == 2 && [touches count] >= 2)
    {   UITouch* touch2 = [myTouches objectAtIndex:1];
        CGPoint pos2 = [touch2 locationInView:touch2.view];
        slTouch2Up(svIndex, pos1.x, pos1.y, pos2.x, pos2.y);
    }
    m_touchDowns -= (int)[touches count];
    if (m_touchDowns < 0) m_touchDowns = 0;
   
    //printf("End   tD: %d, touches count: %d\n", m_touchDowns, [touches count]);
   
    m_lastTouchTimeSec = m_lastFrameTimeSec;
}
//-----------------------------------------------------------------------------
// Event handler for a new live camera image
- (void)captureOutput:(AVCaptureOutput *)captureOutput
didOutputSampleBuffer:(CMSampleBufferRef)sampleBuffer
       fromConnection:(AVCaptureConnection *)connection
{
    if (slNeedsVideoImage() && m_lastVideoImageIsConsumed)
    {
        CVReturn err;
        CVImageBufferRef pixelBuffer = CMSampleBufferGetImageBuffer(sampleBuffer);
        CVPixelBufferLockBaseAddress(pixelBuffer,0);
        int width  = (int) CVPixelBufferGetWidth(pixelBuffer);
        int height = (int) CVPixelBufferGetHeight(pixelBuffer);
        unsigned char* data = (unsigned char*)CVPixelBufferGetBaseAddress(pixelBuffer);
        
        if(!data)
        {   NSLog(@"No pixel buffer data");
            return;
        }
        
        slCopyVideoImage(width, height, SL_BGRA, data, true);
        
        m_lastVideoImageIsConsumed = false;
        
        /*
        if (!m_videoTextureCache)
        {   NSLog(@"No video texture cache");
            return;
        }
        // CVOpenGLESTextureCacheCreateTextureFromImage will create GLES texture
        // optimally from CVImageBufferRef.
        
        [self cleanUpTexture];
        
        glActiveTexture(GL_TEXTURE0);
        err = CVOpenGLESTextureCacheCreateTextureFromImage(kCFAllocatorDefault,
                                                           m_videoTextureCache,
                                                           pixelBuffer,
                                                           NULL,
                                                           GL_TEXTURE_2D,
                                                           GL_RED_EXT,
                                                           (GLsizei) width,
                                                           (GLsizei) height,
                                                           GL_RED_EXT,
                                                           GL_UNSIGNED_BYTE,
                                                           0,
                                                           &m_texture);
        if (err)
        {
            NSLog(@"Error at CVOpenGLESTextureCacheCreateTextureFromImage %d", err);
        }
        
        glBindTexture(CVOpenGLESTextureGetTarget(m_texture), CVOpenGLESTextureGetName(m_texture));
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        */
    }
}
//-----------------------------------------------------------------------------
- (void)setupAVCapture
{
    //-- Create CVOpenGLESTextureCacheRef for optimal CVImageBufferRef to GLES texture conversion.
    /*
    #if COREVIDEO_USE_EAGLCONTEXT_CLASS_IN_API
    CVReturn err = CVOpenGLESTextureCacheCreate(kCFAllocatorDefault, NULL, _context, NULL, &m_videoTextureCache);
    #else
    CVReturn err = CVOpenGLESTextureCacheCreate(kCFAllocatorDefault, NULL, (__bridge void *)_context, NULL, &m_videoTextureCache);
    #endif
    if (err)
    {   NSLog(@"Error at CVOpenGLESTextureCacheCreate %d", err);
        return;
    }
    */
    
    if (UI_USER_INTERFACE_IDIOM() == UIUserInterfaceIdiomPad)
    {   // Choosing bigger preset for bigger screen.
        m_avSessionPreset = AVCaptureSessionPreset1280x720;
    } else
    {   m_avSessionPreset = AVCaptureSessionPreset640x480;
    }
    
    //-- Setup Capture Session.
    m_avSession = [[AVCaptureSession alloc] init];
    [m_avSession beginConfiguration];
    
    //-- Set preset session size.
    [m_avSession setSessionPreset:m_avSessionPreset];
    
    //-- Creata a video device and input from that Device.  Add the input to the capture session.
    AVCaptureDevice * videoDevice = [AVCaptureDevice defaultDeviceWithMediaType:AVMediaTypeVideo];
    if(videoDevice == nil)
        assert(0);
    
    //-- Add the device to the session.
    NSError *error;
    AVCaptureDeviceInput *input = [AVCaptureDeviceInput deviceInputWithDevice:videoDevice error:&error];
    if(error)
        assert(0);
    
    [m_avSession addInput:input];
    
    //-- Create the output for the capture session.
    AVCaptureVideoDataOutput * dataOutput = [[AVCaptureVideoDataOutput alloc] init];
    [dataOutput setAlwaysDiscardsLateVideoFrames:YES]; // Probably want to set this to NO when recording
    
    //-- Set to BGRA.
    [dataOutput setVideoSettings:[NSDictionary dictionaryWithObject:[NSNumber numberWithInt:kCVPixelFormatType_32BGRA]
                                                             forKey:(id)kCVPixelBufferPixelFormatTypeKey]]; // Necessary for manual preview
    
    // Set dispatch to be on the main thread so OpenGL can do things with the data
    [dataOutput setSampleBufferDelegate:self queue:dispatch_get_main_queue()];
    
    [m_avSession addOutput:dataOutput];
    [m_avSession commitConfiguration];
    
    [m_avSession startRunning];
    m_lastVideoImageIsConsumed = true;
}

- (void)cleanUpTexture
{
    /*
    if (m_texture)
    {
        CFRelease(m_texture);
        m_texture = NULL;
    }
    
    // Periodic texture cache flush every frame
    CVOpenGLESTextureCacheFlush(m_videoTextureCache, 0);
    */
}
//-----------------------------------------------------------------------------
@end
