#pragma once

#include <string>
#include <vector>

#include "DXCommon.h"

struct SOutputMode
{
	uint32_t    mWidth, mHeight;
	DXGI_FORMAT mFormat;
	double      mNumerator, mDenominator;
};

static std::ostream &operator<<(std::ostream &aStream, SOutputMode const &aMode)
{
	aStream
		<< "Width:  "      << aMode.mWidth     << ", "
		<< "Height: "      << aMode.mHeight    << ", "
		<< "Format: "      << aMode.mFormat    << ", "
		<< "Numerator: "   << aMode.mNumerator << ", "
		<< "Denominator: " << aMode.mDenominator;
	return aStream;
}

struct SAdapterOutput
{
	std::string              mName;
	uint32_t                 mWidth, mHeight;
	HMONITOR                 mMonitorHandle;
	std::vector<SOutputMode> mOutputModes;
};

static std::ostream &operator<<(std::ostream &aStream, SAdapterOutput const &aOutput)
{
	aStream
		<< "Name:   " << aOutput.mName   << ", "
		<< "Width:  " << aOutput.mWidth  << ", "
		<< "Height: " << aOutput.mHeight << ", ";
	return aStream;
}

struct SAdapter
{
	std::string                 mName;
	std::string                 mDescription;
	uint64_t                    mAvailableDeviceMemory;
	std::vector<SAdapterOutput> mOutputs;
};

static std::ostream &operator<<(std::ostream &aStream, SAdapter const &aAdapter)
{
	aStream
		<< "Name:          " << aAdapter.mName                  << ", "
		<< "Description:   " << aAdapter.mDescription  << ", "
		<< "Device Memory: " << aAdapter.mAvailableDeviceMemory << ", ";
	return aStream;
}

struct SGraphicsCardAdapters
{
	std::vector<SAdapter> mAdapters;
};

static bool determineGraphicsCardAdapters(SGraphicsCardAdapters &aOutAdapters);

class DirectXState
{
public:
	bool initialize(HWND const &aHwnd);
	bool deinitialize();

	inline std::shared_ptr<IDXGISwapChain>         swapChain    () { return mSwapChain;     }
	inline std::shared_ptr<ID3D11Device>           device       () { return mDevice;        }
	inline std::shared_ptr<ID3D11DeviceContext>    deviceContext() { return mDeviceContext; }
	inline std::shared_ptr<ID3D11RenderTargetView> backBufferRTV() { return mBackBufferRTV; }

private:
	std::shared_ptr<IDXGISwapChain>         mSwapChain;
	std::shared_ptr<ID3D11Device>           mDevice;
	std::shared_ptr<ID3D11DeviceContext>    mDeviceContext;
	std::shared_ptr<ID3D11RenderTargetView> mBackBufferRTV;
};