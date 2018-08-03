#pragma once

#include <memory>

#include <dxgi.h>
#include <d3d.h>
#include <d3d11.h>
#include <d3dcommon.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>

#pragma comment (lib, "dxgi.lib")
#pragma comment (lib, "d3d10.lib")
#pragma comment (lib, "d3d11.lib")
#pragma comment (lib, "dxguid.lib")
#pragma comment (lib, "d3dcompiler.lib")

template <typename T>
struct DirectXResourceDelete
{
	void operator()(T *aPointer)
	{
		if (nullptr != aPointer)
		{
			aPointer->Release();
		}
	}
};

template <typename T>
static std::shared_ptr<T> 
	makeDirectXResourcePointer(T *aPointer)
{
	return std::shared_ptr<T>(aPointer, DirectXResourceDelete<T>());
}
