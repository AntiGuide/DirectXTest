#include <exception>
#include <Windows.h>

#include "DXEnvironment.h"
#include "DXMesh.h"
#include "DXMaterial.h"

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
	windowClass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	windowClass.style         = (CS_VREDRAW | CS_HREDRAW | CS_DROPSHADOW);

	ATOM const successful = RegisterClassEx(&windowClass);
	if (!successful)
	{
		throw std::exception("Failed to register window class.");
	}

	DWORD windowStyle = 0;
	windowStyle |= WS_OVERLAPPEDWINDOW;
	windowStyle |= WS_VISIBLE;

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
		throw std::runtime_error("Failed to initialized DirectXState\n");
	}

	// Load triangle
	SMesh      triangle        = {};
	bool const triangleCreated = DXMesh::createTriangle(directXState->device(), triangle);
	if (!triangleCreated)
	{
		throw std::runtime_error("Failed to create triangle.\n");
	}

	SMaterial triangleMaterial = {};
	bool const materialCreated = DXMaterial::create(directXState->device(), triangle, "Default", triangleMaterial);
	if (!materialCreated)
	{
		throw std::runtime_error("Failed to create material.\n");
	}

	ShowWindow(windowHandle, 1);

	MSG msg = {};

	//GameLoop
	bool running = true;

	while (running)
	{
		//Handle windows messages
		while (PeekMessage(&msg, windowHandle, 0, 0, PM_REMOVE))
		{
			if (WM_QUIT == msg.message)
			{
				running = false;
				break;
			}

			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		// Renderer

		// Setup Pipeline
		std::shared_ptr<ID3D11DeviceContext>    renderContext    = directXState->deviceContext();
		std::shared_ptr<IDXGISwapChain>         swapChain        = directXState->swapChain();
		std::shared_ptr<ID3D11RenderTargetView> renderTargetView = directXState->backBufferRTV();

		float color[4] = { 0.5f, 1.0f, 0.5f, 0.0f };
		renderContext->ClearRenderTargetView(renderTargetView.get(), color);

		//Input Assembler
		ID3D11Buffer *vertexBuffer = triangle.mVertexBuffer.get();
		UINT          stride       = sizeof(Vertex);
		UINT          offset       = 0;

		ID3D11Buffer      *indexBuffer = triangle.mIndexBuffer.get();
		ID3D11InputLayout *inputLayout = triangleMaterial.mInputLayout.get();

		renderContext->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
		renderContext->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R16_UINT, 0);
		renderContext->IASetInputLayout(inputLayout);
		renderContext->IASetPrimitiveTopology(? );

		swapChain->Present(0, 0);

		renderContext->ClearState();
	}

	ShowWindow(windowHandle, 0);

	directXState->deinitialize();

	DestroyWindow(windowHandle);
}