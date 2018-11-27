#include "DXMaterial.h"

#include <string>
#include <vector>
#include <fstream>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include "DXTexture.h"

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
	SMaterialDesc                 const &aInMaterialDesc,
	SMaterial                           &aOutMaterial)
{
	auto const createTextureFn = [&](std::vector<SMaterialTextureDesc> const &aTextures) -> std::vector<SMaterialTexture>
	{
		std::vector<SMaterialTexture> textures = {};

		if (not aTextures.empty())
		{
			for (auto const &d : aTextures)
			{
				std::shared_ptr<ID3D11Texture2D> textureManaged = nullptr;

				ID3D11Texture2D          *textureUnmanaged     = nullptr;
				ID3D11ShaderResourceView *textureViewUnmanaged = nullptr;
				ID3D11SamplerState       *samplerUnmanaged     = nullptr;

				int32_t width = 0;
				int32_t height = 0;
				int32_t numChannels = 0;

				// size = width * height * numChannels * 8
				stbi_uc *imageData = stbi_load(d.textureFilename.c_str(), &width, &height, &numChannels, 4);
				if (nullptr != imageData)
				{
					D3D11_SUBRESOURCE_DATA textureData = {};
					textureData.pSysMem     = imageData;
					textureData.SysMemPitch = width * 4 * sizeof(stbi_uc);

					textureManaged = DXTexture::createTexture(aDevice, width, height, 1, DXGI_FORMAT_R8G8B8A8_UNORM, &textureData);

					stbi_image_free(imageData);

					if (nullptr == textureManaged)
					{
						std::cout << "Failed to load default texture.\n";
						break;
					}

					std::shared_ptr<ID3D11ShaderResourceView> resourceView = DXTexture::createTextureResourceView(aDevice, textureManaged, DXGI_FORMAT_R8G8B8A8_UNORM);

					D3D11_SAMPLER_DESC samplerDescription = {};
					samplerDescription.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
					samplerDescription.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
					samplerDescription.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
					samplerDescription.Filter   = D3D11_FILTER_MIN_MAG_MIP_POINT;
					samplerDescription.ComparisonFunc = D3D11_COMPARISON_ALWAYS;

					HRESULT result = aDevice->CreateSamplerState(&samplerDescription, &samplerUnmanaged);
					if (S_OK != result)
					{
						std::cout << "Failed to create sampler state.\n";

						textureUnmanaged->Release();
						textureViewUnmanaged->Release();

						break;
					}

					SMaterialTexture texture     = {};
					texture.mTexture             = textureManaged;
					texture.mTextureResourceView = resourceView;
					texture.mSampler             = makeDirectXResourcePointer(samplerUnmanaged);

					textures.push_back(texture);
				}
			}
		}

		return textures;
	};

	HRESULT result = S_OK;

	SMaterial material = {};

	if (aInMaterialDesc.vertex.useStage)
	{
		std::vector<uint8_t> vertexShaderByteCode = {};

		bool const vertexShaderLoaded = loadShader(aInMaterialDesc.vertex.shaderFileFn, vertexShaderByteCode);
		if (false == vertexShaderLoaded)
		{
			std::cout << "Failed to load shaders for " << aMaterialName << ".\n";
			return false;
		}

		ID3D11InputLayout  *layoutUnmanaged       = nullptr;
		ID3D11VertexShader *vertexShaderUnmanaged = nullptr;

		HRESULT result = aDevice->CreateInputLayout(aMesh.mInputElements, 3, vertexShaderByteCode.data(), vertexShaderByteCode.size(), &layoutUnmanaged);
		if (S_OK != result)
		{
			std::cout << "Failed to create input layout for triangle.\n";
			return false;
		}

		result = aDevice->CreateVertexShader(vertexShaderByteCode.data(), vertexShaderByteCode.size(), nullptr, &vertexShaderUnmanaged);
		if (S_OK != result)
		{
			std::cout << "Failed to create vertex shader for triangle.\n";
			layoutUnmanaged->Release();
			return false;
		}

		material.mInputLayout         = makeDirectXResourcePointer(layoutUnmanaged);
		material.mVertexShader        = makeDirectXResourcePointer(vertexShaderUnmanaged);
		material.vertexShaderTextures = createTextureFn(aInMaterialDesc.vertex.textures);		
	}

	if (aInMaterialDesc.geometry.useStage)
	{
		std::vector<uint8_t> geometryShaderByteCode = {};

		bool const shaderLoaded = loadShader(aInMaterialDesc.geometry.shaderFileFn, geometryShaderByteCode);
		if (false == shaderLoaded)
		{
			std::cout << "Failed to load shaders for " << aMaterialName << ".\n";
			return false;
		}

		ID3D11GeometryShader *shaderUnmanaged = nullptr;

		result = aDevice->CreateGeometryShader(geometryShaderByteCode.data(), geometryShaderByteCode.size(), nullptr, &shaderUnmanaged);
		if (S_OK != result)
		{
			std::cout << "Failed to create geometry shader for triangle.\n";
			return false;
		}

		material.mGeometryShader        = makeDirectXResourcePointer(shaderUnmanaged);
		material.geometryShaderTextures = createTextureFn(aInMaterialDesc.geometry.textures);
	}

	if (aInMaterialDesc.fragment.useStage)
	{
		std::vector<uint8_t> shaderByteCode = {};

		bool const shaderLoaded = loadShader(aInMaterialDesc.fragment.shaderFileFn, shaderByteCode);
		if (false == shaderLoaded)
		{
			std::cout << "Failed to load shaders for " << aMaterialName << ".\n";
			return false;
		}

		ID3D11PixelShader *shaderUnmanaged = nullptr;

		result = aDevice->CreatePixelShader(shaderByteCode.data(), shaderByteCode.size(), nullptr, &shaderUnmanaged);
		if (S_OK != result)
		{
			std::cout << "Failed to create fragment shader for triangle.\n";
			return false;
		}

		material.mPixelShader           = makeDirectXResourcePointer(shaderUnmanaged);
		material.fragmentShaderTextures = createTextureFn(aInMaterialDesc.fragment.textures);
	}
	
	aOutMaterial = material;

	return true;
}