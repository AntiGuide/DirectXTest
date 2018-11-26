#pragma once

#include <string>

#include <iostream>

#include "DXCommon.h"
#include "DXMesh.h"

struct SMaterial
{
	std::shared_ptr<ID3D11InputLayout>    mInputLayout;
	std::shared_ptr<ID3D11VertexShader>   mVertexShader;
	std::shared_ptr<ID3D11GeometryShader> mGeometryShader;
	std::shared_ptr<ID3D11PixelShader>    mPixelShader;

	std::shared_ptr<ID3D11Texture2D>          mTexture;
	std::shared_ptr<ID3D11ShaderResourceView> mTextureView;
	std::shared_ptr<ID3D11SamplerState>       mSampler;
};

class DXMaterial
{
public:
	static bool create(std::shared_ptr<ID3D11Device> aDevice, SMesh const &aMesh, std::string const &aMaterialName, SMaterial &aOutMaterial);
};