#include "Application.h"



Application::Application()
	: window_handle(nullptr),
	direct2d_factory_ptr(nullptr),
	render_target_ptr(nullptr),
	dark_gray_brush_ptr(nullptr),
	blue_brush_ptr(nullptr),
	image_bitmap(nullptr)
{
}

/*
	Release rendering resources when application is destroyed
*/
Application::~Application()
{
	SafeRelease(&direct2d_factory_ptr);
	SafeRelease(&render_target_ptr);
	SafeRelease(&dark_gray_brush_ptr);
	SafeRelease(&blue_brush_ptr);
	SafeRelease(&image_bitmap);
}

/*
	Initializes application and window
*/
HRESULT Application::initialize()
{
	HRESULT hr;

	// initialize device-independent resources, such as the factory
	hr = create_device_independent_resources();

	if (SUCCEEDED(hr))
	{
		// register the window class
		WNDCLASSEX wcex = { sizeof(WNDCLASSEX) };
		wcex.style = CS_HREDRAW | CS_VREDRAW;
		wcex.lpfnWndProc = Application::window_procedure;
		wcex.cbClsExtra = 0;
		wcex.cbWndExtra = sizeof(LONG_PTR);
		wcex.hInstance = HINST_THISCOMPONENT;
		wcex.hbrBackground = NULL;
		wcex.lpszMenuName = NULL;
		wcex.hCursor = LoadCursor(NULL, IDI_APPLICATION);
		wcex.lpszClassName = "D2D1App";

		RegisterClassEx(&wcex);


		UINT width, height;
		width = 640;
		height = 480;

		// Create the window
		window_handle = CreateWindow(
			"D2D1App",
			"Direct2D Application",
			WS_OVERLAPPEDWINDOW,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			width,
			height,
			nullptr,
			nullptr,
			HINST_THISCOMPONENT,
			// passing a pointer to this application instance, so it's
			// available in the static method
			this
		);

		hr = window_handle ? S_OK : E_FAIL;
		if (SUCCEEDED(hr))
		{
			ShowWindow(window_handle, SW_SHOWNORMAL);
			UpdateWindow(window_handle);
		}
	}

	return hr;
}

/*
	The message loop. This is what is actually keeping the window responsive
	It receives messages from Windows, potentially translates them, and dispatch
	them back, and they eventually end up in Application::windows_procedure

	This will loop until the end of the application lifespan
*/
void Application::run_message_loop()
{
	MSG msg;

	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}

/*
	For creating the Direct2D factory to be used for creating render targets later
	Also assembles the color data that will be used to create a bitmap later
*/
HRESULT Application::create_device_independent_resources()
{
	HRESULT hr = S_OK;

	// Creates the factory, enabling most of the D2D1 action
	hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &direct2d_factory_ptr);


	// Example of assembling data to be used later to create a D2D1 bitmap from scratch
	// Essentially renders out as UV coordinates
	// Note this doesn't utilize the D2D1::ColorF
	UINT height = 400;
	UINT width = 400;
	image_data = new UINT8[height * width * 4];
	for (UINT y = 0; y < height; y++)
	{
		for (UINT x = 0; x < width; x++)
		{
			UINT8* pixel_data = image_data + ((y * width) + x) * 4;
			pixel_data[0] = static_cast<byte>(0);
			pixel_data[1] = static_cast<byte>(((float)(height - y)/ height ) * 255);
			pixel_data[2] = static_cast<byte>(((float)(width - x) / width ) * 255);
			pixel_data[3] = static_cast<byte>(255);
		}
	}

	return hr;
}


/*
	Sets up rendering resources. If they're already set up it will just return OK
*/
HRESULT Application::create_device_resources()
{
	HRESULT hr = S_OK;

	if (!render_target_ptr)
	{
		RECT rc;
		GetClientRect(window_handle, &rc);

		D2D1_SIZE_U size = D2D1::SizeU(
			rc.right - rc.left,
			rc.bottom - rc.top
		);

		hr = direct2d_factory_ptr->CreateHwndRenderTarget(
			D2D1::RenderTargetProperties(),
			D2D1::HwndRenderTargetProperties(window_handle, size),
			&render_target_ptr
		);

		if (SUCCEEDED(hr))
		{
			// creates the gray brush
			hr = render_target_ptr->CreateSolidColorBrush(
				D2D1::ColorF(0.1f,0.1f,0.1f,1.0f),
				&dark_gray_brush_ptr
				);
		}

		if (SUCCEEDED(hr))
		{
			// creates the gray brush
			hr = render_target_ptr->CreateSolidColorBrush(
				D2D1::ColorF(D2D1::ColorF::DeepSkyBlue),
				&blue_brush_ptr
			);
		}

		if (SUCCEEDED(hr))
		{
			// creates the image bitmap
			hr = render_target_ptr->CreateBitmap(
				D2D1::SizeU(400, 400),
				image_data,
				400 * 4,
				D2D1::BitmapProperties( D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_IGNORE)),
				&image_bitmap
			);
		}

	}


	return hr;
}

/*
Discard rendering objects safely
*/
void Application::discard_device_resources()
{
	SafeRelease(&render_target_ptr);
	SafeRelease(&dark_gray_brush_ptr);
	SafeRelease(&blue_brush_ptr);
	SafeRelease(&image_bitmap);
}

/*
Main rendering function, called by Application::window_procedure
*/
HRESULT Application::on_render()
{
	HRESULT hr = S_OK;

	hr = create_device_resources();

	if (SUCCEEDED(hr))
	{
		render_target_ptr->BeginDraw();
		render_target_ptr->SetTransform(D2D1::Matrix3x2F::Identity());
		render_target_ptr->Clear(D2D1::ColorF(0.15f,0.15f,0.15f,1.0f));

		D2D1_SIZE_F rt_size = render_target_ptr->GetSize();

		// draw a grid
		int width = static_cast<int>(rt_size.width);
		int height = static_cast<int>(rt_size.height);

		for (int x = 0; x < width; x += 25)
		{
			render_target_ptr->DrawLine(
				D2D1::Point2F(static_cast<FLOAT>(x), 0.0f),
				D2D1::Point2F(static_cast<FLOAT>(x), rt_size.height),
				dark_gray_brush_ptr,
				1.5f
			);
		}

		for (int y = 0; y < height; y += 25)
		{
			render_target_ptr->DrawLine(
				D2D1::Point2F(0.0f, static_cast<FLOAT>(y)),
				D2D1::Point2F(rt_size.width, static_cast<FLOAT>(y)),
				dark_gray_brush_ptr,
				1.5f
				);
		}


		// Draw the bitmap
		render_target_ptr->DrawBitmap(
			image_bitmap,
			D2D1::RectF(
				100,
				100,
				image_bitmap->GetSize().width + 100,
				image_bitmap->GetSize().height + 100
		));

		// set up the box
		D2D1_RECT_F rectangle = D2D1::RectF(
			rt_size.width / 2 - 100.0f,
			rt_size.height / 2 - 100.0f,
			rt_size.width / 2 + 100.0f,
			rt_size.height / 2 + 100.0f
		);

		// draw the box
		render_target_ptr->FillRectangle(&rectangle, blue_brush_ptr);

		hr = render_target_ptr->EndDraw();
	}

	if (hr == D2DERR_RECREATE_TARGET)
	{
		hr = S_OK;
		discard_device_resources();
	}

	return hr;
}


/*
Resizes the render target when window is resized from Application:window_procedure
*/
void Application::on_resize(UINT width, UINT height)
{
	if (render_target_ptr)
	{
		// note from MSDN: this method can fail, but it's okay to
		// ignore the error here, because the error will be
		// returned again the next time EndDraw is called

		// resizes the render target when the window is resized
		render_target_ptr->Resize(D2D1::SizeU(width, height));
	}
}


/*
The windows procedure actually handling messages
needs to be static because the windows API doesn't
support pointing to an instance method.
*/
LRESULT CALLBACK Application::window_procedure(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam)
{
	LRESULT result = 0;

	// runs when the window is created, and sets up the communication to the app instance
	if (message == WM_CREATE)
	{
		LPCREATESTRUCT pcs = (LPCREATESTRUCT)lparam;
		Application* app_ptr = (Application*)pcs->lpCreateParams;

		::SetWindowLongPtrW(
			hwnd,
			GWLP_USERDATA,
			reinterpret_cast<LONG_PTR>(app_ptr)
		);

		result = 1;
	}
	else
	{
		// Get a pointer to the app instance through the extra data stored in the window class
		Application* app_ptr = reinterpret_cast<Application*>(static_cast<LONG_PTR>(
			::GetWindowLongPtrW(
				hwnd,
				GWLP_USERDATA
				)));

		bool was_handled = false;


		// handle messages depending on type
		if (app_ptr)
		{
			switch (message)
			{
			case WM_SIZE:
			{
				UINT width = LOWORD(lparam);
				UINT height = HIWORD(lparam);
				app_ptr->on_resize(width, height);
			}
			result = 0;
			was_handled = true;
			break;

			case WM_DISPLAYCHANGE:
			{
				InvalidateRect(hwnd, NULL, FALSE);
			}
			result = 0;
			was_handled = true;
			break;

			case WM_PAINT:
			{
				app_ptr->on_render();
				ValidateRect(hwnd, NULL);
			}
			result = 0;
			was_handled = true;
			break;

			case WM_DESTROY:
			{
				PostQuitMessage(0);
			}
			result = 1;
			was_handled = true;
			break;
			}
		}

		if (!was_handled)
		{
			result = DefWindowProc(hwnd, message, wparam, lparam);
		}
	}

	return result;
}


/*
The actual entry point of the program, mainly initializes the app and then starts the message pump
Note how we don't need to deal with the variables sent to us by the win32 API, because we will
fetch them later
*/
int WINAPI WinMain(
	HINSTANCE,
	HINSTANCE,
	LPSTR,
	int
)
{
	// Use HeapSetInformation to specify that the process should
	// terminate if the heap manager detects an error in any heap
	// used by the process.
	// The return value is ignored, because we want to continue 
	// running in the unlikely event that HeapSetInformation fails.
	HeapSetInformation(NULL, HeapEnableTerminationOnCorruption, NULL, 0);

	if (SUCCEEDED(CoInitialize(NULL)))
	{
		{
			Application app;

			if (SUCCEEDED(app.initialize()))
			{
				app.run_message_loop();
			}
		}
		CoUninitialize();
	}

	return 0;
}