#include "DXMaterial.h"
#include "DXMesh.h"

#include <string>
#include <vector>
#include <fstream>
#include <iostream>

static bool loadShader(
	std::string const &aFilename,
	std::vector<uint8_t> &aOutByteCode) 
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

DXMaterial::DXMaterial(std::shared_ptr<ID3D11Device> aDevice, SMesh const &aMesh, std::string const & aMaterialName)
{
	//
	// Input Layout
	//

	std::vector<uint8_t> vertexShaderByteCode = {};
	bool loaded = loadShader("Default_vs.cso", vertexShaderByteCode);
	if (!loaded) {
		std::cout << "Failed to load vertex shader for " << aMaterialName << ".\n";
		return;
	}

	ID3D11InputLayout *layoutUnmanaged = nullptr;
	HRESULT result = aDevice->CreateInputLayout(aMesh.mInputElements, 3, vertexShaderByteCode.data(), vertexShaderByteCode.size(), &layoutUnmanaged);
	if (S_OK != result) {
		std::cout << "Failed to create input layout for triangle.\n";
		return;
	}

	mInputLayout = makeDirectXResourcePointer(layoutUnmanaged);

	//
	// Vertex Shader
	//

	ID3D11VertexShader *vertexShaderUnmanaged = nullptr;
	result = aDevice->CreateVertexShader(vertexShaderByteCode.data(), vertexShaderByteCode.size(), nullptr, &vertexShaderUnmanaged);
	if (S_OK != result) {
		std::cout << "Failed to create vertex shader for triangle.\n";
		return;
	}

	mVertexShader = makeDirectXResourcePointer(vertexShaderUnmanaged);

	//
	// Fragment Shader
	//

	std::vector<uint8_t> fragmentShaderByteCode = {};
	loaded = loadShader("FragmentShader_fs.cso", fragmentShaderByteCode);
	if (!loaded) {
		std::cout << "Failed to load fragment shader for " << aMaterialName << ".\n";
		return;
	}

	ID3D11PixelShader *fragmentShaderUnmanaged = nullptr;
	result = aDevice->CreatePixelShader(fragmentShaderByteCode.data(), fragmentShaderByteCode.size(), nullptr, &fragmentShaderUnmanaged);
	if (S_OK != result) {
		std::cout << "Failed to create fragment shader for triangle.\n";
		return;
	}

	mFragmentShader = makeDirectXResourcePointer(fragmentShaderUnmanaged);
}
