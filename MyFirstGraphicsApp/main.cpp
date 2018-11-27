#include <exception>
#include <Windows.h>

#include "DXEnvironment.h"
#include "DXMesh.h"
#include "DXMaterial.h"
#include "DXTexture.h"

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

	// Load Unit-Quad
	SMesh      unitQuad = {};
	bool const unitQuadCreated = DXMesh::createUnitQuad(directXState->device(), unitQuad);
	if (!unitQuadCreated)
	{
		throw std::runtime_error("Failed to create unit quad.\n");
	}

	SMaterialDesc blitMaterialDesc = {};
	blitMaterialDesc.vertex.useStage = true;
	blitMaterialDesc.vertex.shaderFileFn = "Perlin_vs.cso";
	blitMaterialDesc.fragment.useStage = true;
	blitMaterialDesc.fragment.shaderFileFn = "Perlin_ps.cso";

	SMaterialTextureDesc perlinInput = {};
	perlinInput.textureFilename = "textures/perlin_input.jpg";
	blitMaterialDesc.fragment.textures.push_back(perlinInput);

	SMaterial blitMaterial = {};
	bool const materialCreated = DXMaterial::create(directXState->device(), unitQuad, "PerlinNoise", blitMaterialDesc, blitMaterial);
	if (!materialCreated)
	{
		throw std::runtime_error("Failed to create material.\n");
	}

	// Load triangle
	SMesh      triangle = {};
	bool const triangleCreated = DXMesh::createUnitQuad(directXState->device(), triangle, 2);
	if (!triangleCreated)
	{
		throw std::runtime_error("Failed to create triangle.\n");
	}

	SMaterialDesc triangleMaterialDesc = {};
	triangleMaterialDesc.vertex.useStage       = true;
	triangleMaterialDesc.vertex.shaderFileFn   = "Default_vs.cso";
	triangleMaterialDesc.geometry.useStage     = true;
	triangleMaterialDesc.geometry.shaderFileFn = "Default_gs.cso";
	triangleMaterialDesc.fragment.useStage     = true;
	triangleMaterialDesc.fragment.shaderFileFn = "Default_ps.cso";

	SMaterialTextureDesc textureDesc = {};
	textureDesc.textureFilename = "textures/test.png";
	triangleMaterialDesc.fragment.textures.push_back(textureDesc);

	SMaterial triangleMaterial = {};
	bool const triangleMaterialCreated = DXMaterial::create(directXState->device(), triangle, "Default", triangleMaterialDesc, triangleMaterial);
	if (!triangleMaterialCreated)
	{
		throw std::runtime_error("Failed to create material.\n");
	}

	DirectX::XMMATRIX unitQuadTranslation          = DirectX::XMMatrixTranslation(0.0f, 0.0f, 0.2f);
	DirectX::XMMATRIX unitQuadScale                = DirectX::XMMatrixScaling(1024.0, 1024.0, 1.0f);
	DirectX::XMMATRIX worldMatrix                  = DirectX::XMMatrixMultiply(unitQuadScale, unitQuadTranslation);
	DirectX::XMMATRIX viewMatrix                   = DirectX::XMMatrixLookToLH(
														{ 0.0, 0.0, 0.0, 0.0 }, // Position
													    { 0.0, 0.0, 1.0, 0.0 }, // Forward
														{ 0.0, 1.0, 0.0, 0.0 });// Up
	DirectX::XMMATRIX orthographicProjectionMatrix = DirectX::XMMatrixOrthographicLH(1024, 1024, 0.1f, 1.0f);

	struct Matrices
	{
		DirectX::XMMATRIX world;
		DirectX::XMMATRIX view;
		DirectX::XMMATRIX projection;
	} unitQuadMatrices;

	D3D11_BUFFER_DESC constantBufferDesc = {};
	constantBufferDesc.ByteWidth           = sizeof(Matrices);
	constantBufferDesc.BindFlags           = D3D11_BIND_CONSTANT_BUFFER;
	constantBufferDesc.Usage               = D3D11_USAGE_DYNAMIC;
	constantBufferDesc.MiscFlags           = 0;
	constantBufferDesc.CPUAccessFlags      = D3D11_CPU_ACCESS_WRITE;
	constantBufferDesc.StructureByteStride = 0;

	ID3D11Buffer *constantBufferUnmanaged = nullptr;
	HRESULT result = directXState->device()->CreateBuffer(&constantBufferDesc, nullptr, &constantBufferUnmanaged);
	if (S_OK != result)
	{
		std::cout << "Failed to create constant buffer.\n";
	}

	std::shared_ptr<ID3D11DeviceContext> deviceContext = directXState->deviceContext();
	D3D11_MAPPED_SUBRESOURCE mappedConstantBufferData = {};

	result = deviceContext->Map(constantBufferUnmanaged,
								0,
								D3D11_MAP_WRITE_DISCARD,
								0,
								&mappedConstantBufferData);

	if (S_OK == result)
	{
		Matrices *matrices = static_cast<Matrices*>(mappedConstantBufferData.pData);
		matrices->world      = worldMatrix;
		matrices->view       = viewMatrix;
		matrices->projection = orthographicProjectionMatrix;

		deviceContext->Unmap(constantBufferUnmanaged, 0);
	}



	D3D11_RASTERIZER_DESC rasterizerDescription = {};
	rasterizerDescription.AntialiasedLineEnable = true;
	rasterizerDescription.CullMode = D3D11_CULL_BACK;
	rasterizerDescription.FrontCounterClockwise = true;
	rasterizerDescription.DepthClipEnable = true;
	rasterizerDescription.FillMode = D3D11_FILL_SOLID;
	rasterizerDescription.MultisampleEnable = false;
	rasterizerDescription.ScissorEnable = false;

	ID3D11RasterizerState *rasterizer = nullptr;
	result = directXState->device()->CreateRasterizerState(&rasterizerDescription, &rasterizer);
	if (S_OK != result)
	{
		std::cout << "Failed to create rasterizer state.\n";
	}

	// Render To Texture

	std::shared_ptr<ID3D11Texture2D> renderTargetTexture = DXTexture::createTexture(directXState->device(),
		                                                                            1024, 
		                                                                            1024, 
		                                                                            1,
		                                                                            DXGI_FORMAT_R8_UNORM, 
		                                                                            nullptr);
	std::shared_ptr<ID3D11RenderTargetView> renderTargetView = 
		DXTexture::createTextureRenderTargetView(directXState->device(),
			                                     renderTargetTexture, 
			                                     DXGI_FORMAT_R8_UNORM);

	std::shared_ptr<ID3D11ShaderResourceView> renderTargetResourceView =
		DXTexture::createTextureResourceView(directXState->device(),
			                                 renderTargetTexture,
			                                 DXGI_FORMAT_R8_UNORM);

	// deviceContext->ClearRenderTargetView(renderTargetView.get(), color);

	// Input Assembler
	ID3D11Buffer *vertexBuffer = unitQuad.mVertexBuffer.get();
	UINT          stride = sizeof(Vertex);
	UINT          offset = 0;

	ID3D11Buffer      *indexBuffer = unitQuad.mIndexBuffer.get();
	ID3D11InputLayout *inputLayout = blitMaterial.mInputLayout.get();

	deviceContext->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
	deviceContext->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R16_UINT, 0);
	deviceContext->IASetInputLayout(inputLayout);
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// Rasterizer? Viewport?
	D3D11_VIEWPORT viewPort = {};
	viewPort.TopLeftX = 0;
	viewPort.TopLeftY = 0;
	viewPort.Width    = 1024;
	viewPort.Height   = 1024;
	viewPort.MinDepth = 0.0;
	viewPort.MaxDepth = 1.0;

	deviceContext->RSSetViewports(1, &viewPort);
	deviceContext->RSSetState(rasterizer);

	// Output
	ID3D11RenderTargetView *renderToTextureTarget = renderTargetView.get();
	deviceContext->OMSetRenderTargets(1, &renderToTextureTarget, nullptr);

	// Shader
	ID3D11VertexShader       *vertexShader   = blitMaterial.mVertexShader.get();
	ID3D11PixelShader        *pixelShader    = blitMaterial.mPixelShader.get();

	deviceContext->VSSetShader(vertexShader, nullptr, 0);
	deviceContext->VSSetConstantBuffers(0, 1, &constantBufferUnmanaged);

	// renderContext->GSSetShader(geometryShader, nullptr, 0);
	deviceContext->PSSetShader(pixelShader, nullptr, 0);
	for (uint32_t k = 0; k < blitMaterial.fragmentShaderTextures.size(); ++k)
	{
		ID3D11ShaderResourceView *const texture = blitMaterial.fragmentShaderTextures[k].mTextureResourceView.get();
		ID3D11SamplerState       *const sampler = blitMaterial.fragmentShaderTextures[k].mSampler.get();
		deviceContext->PSSetShaderResources(k, 1, &texture);
		deviceContext->PSSetSamplers(k, 1, &sampler);
	}

	deviceContext->DrawIndexed(6, 0, 0);

	// Render To Texture

	triangleMaterial.fragmentShaderTextures[0].mTexture             = renderTargetTexture;
	triangleMaterial.fragmentShaderTextures[0].mTextureResourceView = renderTargetResourceView;


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

		// color[0] = fmod((color[0] + 0.00001f), 1.0f);

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
				
		renderContext->RSSetState(rasterizer);

		// Output
		ID3D11RenderTargetView *backBuffer = renderTargetView.get();
		renderContext->OMSetRenderTargets(1, &backBuffer, nullptr);

		// Shader
		ID3D11VertexShader       *vertexShader    = triangleMaterial.mVertexShader.get();
		ID3D11PixelShader        *pixelShader     = triangleMaterial.mPixelShader.get();
		ID3D11GeometryShader     *geometryShader  = triangleMaterial.mGeometryShader.get();

		renderContext->VSSetShader(vertexShader, nullptr, 0);
		for (uint32_t k = 0; k < triangleMaterial.vertexShaderTextures.size(); ++k)
		{
			ID3D11ShaderResourceView *const texture = triangleMaterial.vertexShaderTextures[k].mTextureResourceView.get();
			ID3D11SamplerState       *const sampler = triangleMaterial.vertexShaderTextures[k].mSampler.get();
			renderContext->VSSetShaderResources(k, 1, &texture);
			renderContext->VSSetSamplers(k, 1, &sampler);
		}

		// renderContext->GSSetShader(geometryShader, nullptr, 0);
		renderContext->PSSetShader(pixelShader,    nullptr, 0);
		for (uint32_t k = 0; k < triangleMaterial.fragmentShaderTextures.size(); ++k)
		{
			ID3D11ShaderResourceView *const texture = triangleMaterial.fragmentShaderTextures[k].mTextureResourceView.get();
			ID3D11SamplerState       *const sampler = triangleMaterial.fragmentShaderTextures[k].mSampler.get();
			renderContext->PSSetShaderResources(k, 1, &texture);
			renderContext->PSSetSamplers(k, 1, &sampler);
		}

		renderContext->DrawIndexed(6, 0, 0);

		swapChain->Present(0, 0);

		renderContext->ClearState();
	}

	rasterizer->Release();

	ShowWindow(windowHandle, 0);

	directXState->deinitialize();

	DestroyWindow(windowHandle);
}