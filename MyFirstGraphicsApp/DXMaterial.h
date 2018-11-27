#pragma once

#include <string>

#include <iostream>

#include "DXCommon.h"
#include "DXMesh.h"

struct SMaterialTextureDesc
{
	std::string textureFilename;
};

struct SMaterialStageDesc
{
	std::string                       shaderFileFn;
	bool                              useStage;
	std::vector<SMaterialTextureDesc> textures;

	SMaterialStageDesc()
		: shaderFileFn("")
		, useStage(false)
	{}
};

struct SMaterialDesc
{
	SMaterialStageDesc vertex, geometry, fragment;
};

struct SMaterialTexture
{
	std::shared_ptr<ID3D11Texture2D>          mTexture;
	std::shared_ptr<ID3D11ShaderResourceView> mTextureResourceView;
	std::shared_ptr<ID3D11SamplerState>       mSampler;
};

struct SMaterial
{
	std::shared_ptr<ID3D11InputLayout>    mInputLayout;
	std::shared_ptr<ID3D11VertexShader>   mVertexShader;
	std::shared_ptr<ID3D11GeometryShader> mGeometryShader;
	std::shared_ptr<ID3D11PixelShader>    mPixelShader;

	std::vector<SMaterialTexture>
		vertexShaderTextures,
		geometryShaderTextures,
		fragmentShaderTextures;
};

class DXMaterial
{
public:
	static bool create(
		std::shared_ptr<ID3D11Device>        aDevice,
		SMesh                         const &aMesh,
		std::string                   const &aMaterialName,
		SMaterialDesc                 const &aInMaterialDesc,
		SMaterial                           &aOutMaterial);
};