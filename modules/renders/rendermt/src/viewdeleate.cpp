#include "viewdelegate.h"

#include "rendermtsystem.h"
#if defined(SHARED_DEFINE)
#include "editor/viewport/viewport.h"
#endif

extern "C" NS::String* NSTemporaryDirectory( void );

ViewDelegate::ViewDelegate(RenderMtSystem *system, Viewport *viewport) :
        m_render(system),
        m_viewport(viewport) {

}

void ViewDelegate::drawInMTKView(MTK::View *view) {
    NS::AutoreleasePool *pool = NS::AutoreleasePool::alloc()->init();
#if defined(SHARED_DEFINE)
    NS::String* _pTraceSaveFilePath;
    if(frame == 100) {
        MTL::CaptureManager* pCaptureManager = MTL::CaptureManager::sharedCaptureManager();
        bool success = pCaptureManager->supportsDestination( MTL::CaptureDestinationGPUTraceDocument );
        if ( !success )
        {
            __builtin_printf( "Capture support is not enabled\n");
            assert( false );
        }

        char filename[NAME_MAX];
        std::time_t now;
        std::time( &now );
        std::strftime( filename, NAME_MAX, "capture-%H-%M-%S_%m-%d-%y.gputrace", std::localtime( &now ) );

        _pTraceSaveFilePath = NSTemporaryDirectory()->stringByAppendingString( NS::String::string( filename, NS::UTF8StringEncoding ) );
        NS::URL *pURL = NS::URL::alloc()->initFileURLWithPath( _pTraceSaveFilePath );

        MTL::CaptureDescriptor* pCaptureDescriptor = MTL::CaptureDescriptor::alloc()->init();

        pCaptureDescriptor->setDestination( MTL::CaptureDestinationGPUTraceDocument );
        pCaptureDescriptor->setOutputURL( pURL );
        pCaptureDescriptor->setCaptureObject( WrapperMt::device() );

        NS::Error *pError = nullptr;

        success = pCaptureManager->startCapture( pCaptureDescriptor, &pError );
        if ( !success )
        {
            assert( false );
        }

        pURL->release();
        pCaptureDescriptor->release();

        m_captureInprogress = true;
    }
    frame++;
#endif
    MTL::CommandBuffer *cmd = WrapperMt::queue()->commandBuffer();

    m_render->setCurrentView(view, cmd);

    // Render cycle here
#if defined(SHARED_DEFINE)
    m_viewport->onDraw();
#endif
    CA::MetalDrawable *drawable = view->currentDrawable();
    if(drawable) {
        cmd->presentDrawable(drawable);
        cmd->commit();
    }
#if defined(SHARED_DEFINE)
    if(m_captureInprogress) {
        MTL::CaptureManager* pCaptureManager = MTL::CaptureManager::sharedCaptureManager();
        pCaptureManager->stopCapture();

        NS::String* pOpenCmd = NS::MakeConstantString( "open " )->stringByAppendingString( _pTraceSaveFilePath );
        system( pOpenCmd->utf8String() );

        m_captureInprogress = false;
    }
#endif

    pool->release();
}
