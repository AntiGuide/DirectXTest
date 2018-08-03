#include <exception>
#include <Windows.h>

#include "DXEnvironment.h"
#include "DXMaterial.h"
#include "DXMesh.h"

LRESULT CALLBACK DefaultWindowProcedure(
	HWND   windowHandle,
	UINT   message,
	WPARAM wParam,
	LPARAM lParam)
{
	return DefWindowProc(windowHandle, message, wParam, lParam);
}

int WINAPI WinMain(
	HINSTANCE hInstance,
	HINSTANCE hPreviousInstance,
	LPSTR     lpCommandLine,
	int       nCommandShow)
{
	WNDCLASSEX windowClass = {};
	windowClass.cbSize        = sizeof(WNDCLASSEX);
	windowClass.lpszClassName = L"WindowClass";
	windowClass.lpszMenuName  = nullptr;
	windowClass.hInstance     = hInstance;
	windowClass.lpfnWndProc   = DefaultWindowProcedure; 
	windowClass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	windowClass.style         = (CS_VREDRAW | CS_HREDRAW | CS_DROPSHADOW);

	ATOM const successful = RegisterClassEx(&windowClass);
	if (!successful)
	{
		throw std::exception("Failed to register window class.");
	}

	DWORD windowStyle = 0;
	windowStyle = WS_OVERLAPPEDWINDOW;
	windowStyle = WS_VISIBLE;

	HWND const windowHandle = CreateWindowEx
	(
		0,
		L"WindowClass",
		L"My awesome window. FUCK YEAH!~",
		windowStyle,
		0,    0,
		1920, 1080,
		NULL,
		NULL,
		hInstance,
		nullptr
	);

	if (NULL == windowHandle)
	{
		throw std::exception("Failed to create window.");
	}

	std::shared_ptr<DirectXState> directXState = std::make_shared<DirectXState>();
	bool const initialized = directXState->initialize(windowHandle);
	if (!initialized)
	{
		throw std::runtime_error("Failed to initialize DirectXState\n");
	}

	SMesh triangle = {};
	bool const triangleCreated = DXMesh::createTriangle(directXState->device(), triangle);

	DXMaterial triangleMaterial(directXState->device(), triangle, "Default");

	ShowWindow(windowHandle, 1);

	system("PAUSE");

	ShowWindow(windowHandle, 0);

	// directXState->deinitialize();

	DestroyWindow(windowHandle);
}