#include <iostream>

#include "DXMesh.h"

struct Vertex
{
	DirectX::XMVECTOR mPosition;
	DirectX::XMVECTOR mColor;
	DirectX::XMVECTOR mTexCoord;
};

bool DXMesh::createTriangle(std::shared_ptr<ID3D11Device> aDevice, SMesh & aOutMesh)
{
	Vertex vertex0 = { { -1.0, 0.0, 0.0, 1.0 }, { 1.0, 0.0, 0.0, 0.0 }, { 0.0, 1.0, 0.0, 0.0} };
	Vertex vertex1 = { {  1.0, 0.0, 0.0, 1.0 }, { 0.0, 1.0, 0.0, 0.0 }, { 1.0, 1.0, 0.0, 0.0} };
	Vertex vertex2 = { {  0.0, 2.0, 0.0, 1.0 }, { 0.0, 0.0, 1.0, 0.0 }, { 0.5, 0.0, 0.0, 0.0} };

	Vertex vertices[3] = { vertex0, vertex1, vertex2 };
	uint16_t indices[3] = { 0, 1, 2 };

	ID3D11Buffer *vertexBufferUnmanaged = nullptr;

	D3D11_BUFFER_DESC vertexBufferDescription = {};
	vertexBufferDescription.ByteWidth = 3 * sizeof(Vertex);
	vertexBufferDescription.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDescription.Usage = D3D11_USAGE_DEFAULT;
	vertexBufferDescription.MiscFlags = 0;
	vertexBufferDescription.CPUAccessFlags = 0;
	vertexBufferDescription.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA vertexBufferData = {};
	vertexBufferData.pSysMem = vertices;
	vertexBufferData.SysMemPitch = 0;
	vertexBufferData.SysMemSlicePitch = 0;

	HRESULT result = aDevice->CreateBuffer(&vertexBufferDescription, &vertexBufferData, &vertexBufferUnmanaged);
	if (S_OK != result)
	{
		std::cout << "Failed to create vertex buffer for triangle.\n";
	}


	ID3D11Buffer *indexBufferUnmanaged = nullptr;
	
	D3D11_BUFFER_DESC indexBufferDescription = {};
	indexBufferDescription.ByteWidth = 3 * sizeof(uint16_t);
	indexBufferDescription.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDescription.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDescription.MiscFlags = 0;
	indexBufferDescription.CPUAccessFlags = 0;
	indexBufferDescription.StructureByteStride = 0;
	
	D3D11_SUBRESOURCE_DATA indexBufferData = {};
	indexBufferData.pSysMem = indices;
	indexBufferData.SysMemPitch = 0;
	indexBufferData.SysMemSlicePitch = 0;

	result = aDevice->CreateBuffer(&indexBufferDescription, &indexBufferData, &indexBufferUnmanaged);
	if (S_OK != result)
	{
		std::cout << "Failed to create index buffer.\n";
	}

	D3D11_INPUT_ELEMENT_DESC positionElement = {};
	positionElement.SemanticName             = "POSITION";
	positionElement.SemanticIndex            = 0;
	positionElement.InputSlot                = 0;
	positionElement.InputSlotClass           = D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA;
	positionElement.Format                   = DXGI_FORMAT_R32G32B32A32_FLOAT;
	positionElement.AlignedByteOffset        = D3D11_APPEND_ALIGNED_ELEMENT; // 0 * sizeof(DirectX::XMVECTOR);
	positionElement.InstanceDataStepRate     = 0;
										     
	D3D11_INPUT_ELEMENT_DESC colorElement = {};
	colorElement.SemanticName             = "COLOR";
	colorElement.SemanticIndex            = 0;
	colorElement.InputSlot                = 0;
	colorElement.InputSlotClass           = D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA;
	colorElement.Format                   = DXGI_FORMAT_R32G32B32A32_FLOAT;
	colorElement.AlignedByteOffset        = D3D11_APPEND_ALIGNED_ELEMENT; // 1 * sizeof(DirectX::XMVECTOR);
	colorElement.InstanceDataStepRate     = 0;

	D3D11_INPUT_ELEMENT_DESC texCoordElement = {};
	texCoordElement.SemanticName   = "TEXCOORD";
	texCoordElement.SemanticIndex  = 0;
	texCoordElement.InputSlot      = 0;
	texCoordElement.InputSlotClass = D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA;
	texCoordElement.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	texCoordElement.AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT; // 2 * sizeof(DirectX::XMVECTOR);
	texCoordElement.InstanceDataStepRate = 0;

	D3D11_INPUT_ELEMENT_DESC inputElements[3] = 
	{
		positionElement,
		colorElement,
		texCoordElement
	};

	result = aDevice->CreateInputLayout(inputElements, 3, nullptr, 0, nullptr);
	if (S_OK != result)
	{
		std::cout << "Failed to create input layout for triangle.\n";
		return false;
	}

	SMesh mesh = {};
	mesh.mVertexBuffer = makeDirectXResourcePointer(vertexBufferUnmanaged);
	mesh.mIndexBuffer = makeDirectXResourcePointer(indexBufferUnmanaged);
	memcpy(&(mesh.mInputElements), inputElements, sizeof(D3D11_INPUT_ELEMENT_DESC) * 3);

	aOutMesh = mesh;

	return true;
}
