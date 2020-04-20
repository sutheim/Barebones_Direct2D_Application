#pragma once
#include <windows.h>
#include <d2d1.h>

// Not sure why I can't get VS to deal with this, but getting linking errors without it
#pragma comment(lib, "d2d1")


// Nice little template to deal with releasing drawing resources
template<class T> void SafeRelease(T **ppT)
{
	if (*ppT)
	{
		(*ppT)->Release();
		(*ppT) = nullptr;
	}
}


// some windows wizardry to get access to the window instance in
// the application instance when initializing the window?
#ifndef HINST_THISCOMPONENT
EXTERN_C IMAGE_DOS_HEADER __ImageBase;
#define HINST_THISCOMPONENT ((HINSTANCE)&__ImageBase)
#endif


class Application
{
public:
	Application();
	~Application();

	// Registers the window class and calls methods for instantiating drawing resources
	HRESULT initialize();

	// Process and dispatch messages
	void run_message_loop();

private:

	// initialize device-independent resources, like the Direct2D factory
	HRESULT create_device_independent_resources();

	// initialize device-dependent resources, like render targets and brushes
	HRESULT create_device_resources();

	// release device-dependent resource when they are no longer needed
	void discard_device_resources();

	// draw content when told to do so by window_procedure
	HRESULT on_render();

	// resize the render target
	void on_resize(UINT width, UINT height);

	// Handles messages from Windows. It is static because instance
	// methods are not supported. This is circumvented by creating
	// a pointer to the class instance in the window handle
	static LRESULT CALLBACK window_procedure(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam);

	
	HWND window_handle;
	
	// Rendering resources
	ID2D1Factory* direct2d_factory_ptr;
	ID2D1HwndRenderTarget* render_target_ptr;
	ID2D1SolidColorBrush* dark_gray_brush_ptr;
	ID2D1SolidColorBrush* blue_brush_ptr;

	UINT8* image_data;
	ID2D1Bitmap* image_bitmap;

};