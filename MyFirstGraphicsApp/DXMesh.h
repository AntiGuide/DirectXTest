#pragma once

#include <memory>
#include "DXEnvironment.h"

struct SMesh
{
	std::shared_ptr<ID3D11Buffer> mVertexBuffer;
	std::shared_ptr<ID3D11Buffer> mIndexBuffer;
	//std::shared_ptr<ID3D11InputLayout> mInputLayout;
	D3D11_INPUT_ELEMENT_DESC      mInputElements[3];
};

class DXMesh
{
public:
	static bool createTriangle(std::shared_ptr<ID3D11Device> aDevice, SMesh &aOutMesh);

private:

};