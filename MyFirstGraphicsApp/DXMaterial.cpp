#include "DXMaterial.h"

#include <string>
#include <vector>
#include <fstream>

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
		return false;
	}

	SMaterial material = {};
	material.mInputLayout    = makeDirectXResourcePointer(layoutUnmanaged);
	material.mVertexShader   = makeDirectXResourcePointer(vertexShaderUnmanaged);
	material.mPixelShader    = makeDirectXResourcePointer(pixelShaderUnmanaged);
	material.mGeometryShader = makeDirectXResourcePointer(geometryShaderUnmanaged);

	aOutMaterial = material;

	return true;
}