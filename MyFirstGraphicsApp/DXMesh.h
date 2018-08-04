#pragma once

#include <memory>
#include "DXCommon.h"
#include "DXEnvironment.h"

using DirectX::XMVECTOR;

struct Vertex
{
	XMVECTOR mPosition;
	XMVECTOR mColor;
	XMVECTOR mTexCoord;
};

struct SMesh
{
	std::shared_ptr<ID3D11Buffer>      mVertexBuffer;
	std::shared_ptr<ID3D11Buffer>      mIndexBuffer;
	D3D11_INPUT_ELEMENT_DESC           mInputElements[3];
};

class DXMesh
{
public:
	static bool createTriangle(std::shared_ptr<ID3D11Device> aDevice, SMesh &aOutMesh);
};