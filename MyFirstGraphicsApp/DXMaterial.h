#pragma once

#include <string>
#include "DXCommon.h"
#include "DXMesh.h"

class DXMaterial 
{
public:
	DXMaterial(std::shared_ptr<ID3D11Device> aDevice, SMesh const &aMesh, std::string const & aMaterialName);

private:
	std::shared_ptr<ID3D11InputLayout> mInputLayout;
	std::shared_ptr<ID3D11VertexShader> mVertexShader;
	std::shared_ptr<ID3D11PixelShader> mFragmentShader;
};