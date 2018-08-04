#pragma once

#include <memory>
#include "DXCommon.h"
#include "DXEnvironment.h"

struct SMesh
{
	std::shared_ptr<ID3D11Buffer>      mVertexBuffer;
	std::shared_ptr<ID3D11Buffer>      mIndexBuffer;
	D3D11_INPUT_ELEMENT_DESC           mInputElements[3];
};

struct Vertex
{
	DirectX::XMVECTOR mPosition;
	DirectX::XMVECTOR mColor;
	DirectX::XMVECTOR mTexCoord;
};

class DXMesh
{
public:
	static bool createTriangle(std::shared_ptr<ID3D11Device> aDevice, SMesh &aOutMesh);
};