#include "DXMaterial.h"

#include <string>
#include <vector>
#include <fstream>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

static bool loadShader(
	std::string          const &aFilename, 
	std::vector<uint8_t>       &aOutByteCode)
{
	std::ifstream in;
	in.open(aFilename, std::ios::in | std::ios::binary | std::ios::ate);

	if (in.bad() || in.fail())
	{
		return false;
	}

	std::streampos size = in.tellg();
	in.seekg(0, std::ios::beg);

	aOutByteCode.resize(size);
	in.read(reinterpret_cast<char *>(&aOutByteCode.front()), size);

	return true;
}

bool DXMaterial::create(
	std::shared_ptr<ID3D11Device>        aDevice, 
	SMesh                         const &aMesh,
	std::string                   const &aMaterialName, 
	SMaterial                           &aOutMaterial)
{
	std::vector<uint8_t> vertexShaderByteCode    = {};
	std::vector<uint8_t> pixelShaderByteCode     = {};
	std::vector<uint8_t> geometryShaderByteCode  = {};

	bool const vertexShaderLoaded = loadShader(std::string(aMaterialName) + "_vs.cso", vertexShaderByteCode);
	if(false == vertexShaderLoaded)
	{
		std::cout << "Failed to load shaders for " << aMaterialName << ".\n";
		return false; 
	}

	bool const pixelShaderLoaded = loadShader(std::string(aMaterialName) + "_ps.cso", pixelShaderByteCode);
	if (false == pixelShaderLoaded)
	{
		std::cout << "Failed to load shaders for " << aMaterialName << ".\n";
		return false;
	}

	bool const geometryShaderLoaded = loadShader(std::string(aMaterialName) + "_gs.cso", geometryShaderByteCode);
	if (false == geometryShaderLoaded)
	{
		std::cout << "Failed to load shaders for " << aMaterialName << ".\n";
		return false;
	}

	ID3D11InputLayout *layoutUnmanaged = nullptr;

	HRESULT result = aDevice->CreateInputLayout(aMesh.mInputElements, 3, vertexShaderByteCode.data(), vertexShaderByteCode.size(), &layoutUnmanaged);
	if (S_OK != result)
	{
		std::cout << "Failed to create input layout for triangle.\n";
		return false;
	}

	ID3D11VertexShader   *vertexShaderUnmanaged   = nullptr;
	ID3D11PixelShader    *pixelShaderUnmanaged    = nullptr;
	ID3D11GeometryShader *geometryShaderUnmanaged = nullptr;

	result = aDevice->CreateVertexShader(vertexShaderByteCode.data(), vertexShaderByteCode.size(), nullptr, &vertexShaderUnmanaged);
	if (S_OK != result)
	{
		std::cout << "Failed to create vertex shader for triangle.\n";
		layoutUnmanaged->Release();
		return false;
	}

	result = aDevice->CreatePixelShader(pixelShaderByteCode.data(), pixelShaderByteCode.size(), nullptr, &pixelShaderUnmanaged);
	if (S_OK != result)
	{
		std::cout << "Failed to create pixel shader for triangle.\n";
		layoutUnmanaged->Release();
		vertexShaderUnmanaged->Release();
		return false;
	}

	result = aDevice->CreateGeometryShader(geometryShaderByteCode.data(), geometryShaderByteCode.size(), nullptr, &geometryShaderUnmanaged);
	if (S_OK != result)
	{
		std::cout << "Failed to create geometry shader for triangle.\n";
		layoutUnmanaged->Release();
		vertexShaderUnmanaged->Release();
		pixelShaderUnmanaged->Release();
		return false;
	}

	ID3D11Texture2D          *textureUnmanaged     = nullptr;
	ID3D11ShaderResourceView *textureViewUnmanaged = nullptr;
	ID3D11SamplerState       *samplerUnmanaged     = nullptr;

	int32_t width       = 0;
	int32_t height      = 0;
	int32_t numChannels = 0;

	// size = width * height * numChannels * 8.
	stbi_uc *imageData = stbi_load("textures/test.png", &width, &height, &numChannels, 4);
	if (nullptr != imageData)
	{
		D3D11_TEXTURE2D_DESC textureDescription{};
		textureDescription.Width              = width;
		textureDescription.Height             = height;
		textureDescription.Format             = DXGI_FORMAT_R8G8B8A8_UNORM;
		textureDescription.ArraySize          = 1;
		textureDescription.MipLevels          = 1;
		textureDescription.BindFlags          = D3D11_BIND_SHADER_RESOURCE;
		textureDescription.CPUAccessFlags     = 0;
		textureDescription.MiscFlags          = 0;
		textureDescription.SampleDesc.Count   = 1;
		textureDescription.SampleDesc.Quality = 0;
		textureDescription.Usage              = D3D11_USAGE_DEFAULT;

		D3D11_SUBRESOURCE_DATA textureData = {};
		textureData.pSysMem     = imageData;
		textureData.SysMemPitch = width * 4 * sizeof(stbi_uc);

		result = aDevice->CreateTexture2D(&textureDescription, &textureData, &textureUnmanaged);

		stbi_image_free(imageData);

		if (S_OK != result)
		{
			std::cout << "Failed to load default texture.\n";
			layoutUnmanaged->Release();
			vertexShaderUnmanaged->Release();
			pixelShaderUnmanaged->Release();
			geometryShaderUnmanaged->Release();
			return false;
		}		

		D3D11_SHADER_RESOURCE_VIEW_DESC textureViewDescription = {};
		textureViewDescription.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		textureViewDescription.Texture2D.MipLevels       = 1;
		textureViewDescription.Texture2D.MostDetailedMip = 0;
		textureViewDescription.Format                     = DXGI_FORMAT_R8G8B8A8_UNORM;

		result = aDevice->CreateShaderResourceView(textureUnmanaged, &textureViewDescription, &textureViewUnmanaged);
		if (S_OK != result)
		{
			std::cout << "Failed to create texture view.\n";
			layoutUnmanaged->Release();
			vertexShaderUnmanaged->Release();
			pixelShaderUnmanaged->Release();
			geometryShaderUnmanaged->Release();
			textureUnmanaged->Release();
			return false;
		}
		
		D3D11_SAMPLER_DESC samplerDescription = {};
		samplerDescription.AddressU       = D3D11_TEXTURE_ADDRESS_CLAMP;
		samplerDescription.AddressV       = D3D11_TEXTURE_ADDRESS_CLAMP;
		samplerDescription.AddressW       = D3D11_TEXTURE_ADDRESS_CLAMP;
		samplerDescription.Filter         = D3D11_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;
		samplerDescription.ComparisonFunc = D3D11_COMPARISON_ALWAYS;

		result = aDevice->CreateSamplerState(&samplerDescription, &samplerUnmanaged);
		if (S_OK != result)
		{
			std::cout << "Failed to create sampler state.\n";
			layoutUnmanaged->Release();
			vertexShaderUnmanaged->Release();
			pixelShaderUnmanaged->Release();
			geometryShaderUnmanaged->Release();
			textureUnmanaged->Release();
			textureViewUnmanaged->Release();
			return false;
		}
	}

	SMaterial material = {};
	material.mInputLayout    = makeDirectXResourcePointer(layoutUnmanaged);
	material.mVertexShader   = makeDirectXResourcePointer(vertexShaderUnmanaged);
	material.mPixelShader    = makeDirectXResourcePointer(pixelShaderUnmanaged);
	material.mGeometryShader = makeDirectXResourcePointer(geometryShaderUnmanaged);
	material.mTexture        = makeDirectXResourcePointer(textureUnmanaged);
	material.mTextureView    = makeDirectXResourcePointer(textureViewUnmanaged);
	material.mSampler        = makeDirectXResourcePointer(samplerUnmanaged);

	aOutMaterial = material;

	return true;
}