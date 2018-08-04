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
	windowClass.cbSize = sizeof(WNDCLASSEX);
	windowClass.lpszClassName = L"WindowClass";
	windowClass.lpszMenuName = nullptr;
	windowClass.hInstance = hInstance;
	windowClass.lpfnWndProc = DefaultWindowProcedure;
	windowClass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	windowClass.style = (CS_VREDRAW | CS_HREDRAW | CS_DROPSHADOW);

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
		0, 0,
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
	SMesh      triangle = {};
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
	
	float color[4] = { 0.5f, 1.0f, 0.5f, 0.0f };

	// GameLoop
	bool running = true;
	while (running)
	{
		// Handle windows messages
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

		// Render

		// Setup Pipeline
		std::shared_ptr<ID3D11DeviceContext>    renderContext    = directXState->deviceContext();
		std::shared_ptr<IDXGISwapChain>         swapChain        = directXState->swapChain();
		std::shared_ptr<ID3D11RenderTargetView> renderTargetView = directXState->backBufferRTV();

		color[0] = fmod((color[0] + 0.00001f), 1.0f);

		renderContext->ClearRenderTargetView(renderTargetView.get(), color);

		// Input Assembler
		ID3D11Buffer *vertexBuffer = triangle.mVertexBuffer.get();
		UINT          stride       = sizeof(Vertex);
		UINT          offset       = 0;

		ID3D11Buffer      *indexBuffer = triangle.mIndexBuffer.get();
		ID3D11InputLayout *inputLayout = triangleMaterial.mInputLayout.get();

		renderContext->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
		renderContext->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R16_UINT, 0);
		renderContext->IASetInputLayout(inputLayout);
		renderContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		// Rasterizer? Viewport?
		D3D11_VIEWPORT viewPort = {};
		viewPort.TopLeftX = 0;
		viewPort.TopLeftY = 0;
		viewPort.Width    = 1920;
		viewPort.Height   = 1080;
		viewPort.MinDepth = 0.0;
		viewPort.MaxDepth = 1.0;

		renderContext->RSSetViewports(1, &viewPort);

		D3D11_RASTERIZER_DESC rasterizerDescription = {};
		rasterizerDescription.AntialiasedLineEnable = true;
		rasterizerDescription.CullMode              = D3D11_CULL_BACK;
		rasterizerDescription.FrontCounterClockwise = true;
		rasterizerDescription.DepthClipEnable       = true;
		rasterizerDescription.FillMode              = D3D11_FILL_SOLID;
		rasterizerDescription.MultisampleEnable     = false;
		rasterizerDescription.ScissorEnable         = false;
		
		ID3D11RasterizerState *rasterizer = nullptr;
		HRESULT result = directXState->device()->CreateRasterizerState(&rasterizerDescription, &rasterizer);
		if (S_OK != result)
		{
			std::cout << "Failed to create rasterizer state.\n";
		}
		else
			renderContext->RSSetState(rasterizer);

		// Output
		ID3D11RenderTargetView *backBuffer = renderTargetView.get();
		renderContext->OMSetRenderTargets(1, &backBuffer, nullptr);

		// Shader
		ID3D11VertexShader   *vertexShader   = triangleMaterial.mVertexShader.get();
		ID3D11PixelShader    *pixelShader    = triangleMaterial.mPixelShader.get();
		ID3D11GeometryShader *geometryShader = triangleMaterial.mGeometryShader.get();

		renderContext->VSSetShader(vertexShader,   nullptr, 0);
		renderContext->GSSetShader(geometryShader, nullptr, 0);
		renderContext->PSSetShader(pixelShader,    nullptr, 0);

		renderContext->DrawIndexed(3, 0, 0);

		swapChain->Present(0, 0);

		renderContext->ClearState();
		rasterizer->Release();
	}

	ShowWindow(windowHandle, 0);

	directXState->deinitialize();

	DestroyWindow(windowHandle);
}