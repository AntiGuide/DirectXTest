#include <iostream>
#include <codecvt>

#include "DXEnvironment.h"


static std::string narrow(std::wstring const &aInput)
{
	if (aInput.empty())
	{
		return std::string();
	}

	std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> convert;
	std::string const narrowed = convert.to_bytes(aInput);

	return std::move(narrowed);
}

bool determineGraphicsCardAdapters(std::shared_ptr<IDXGIFactory> aFactory, SGraphicsCardAdapters &aOutAdapters)
{
	HRESULT result = S_OK;

	SGraphicsCardAdapters adapters = {};

	uint32_t      adapterIndex = 0;
	uint32_t      outputIndex  = 0;
	uint32_t      modeIndex    = 0; 

	IDXGIAdapter *adapter = nullptr;
	IDXGIOutput  *output  = nullptr;

	while (DXGI_ERROR_NOT_FOUND != (result = aFactory->EnumAdapters(adapterIndex, &adapter)))
	{
		DXGI_ADAPTER_DESC adapterDescription = {};
		adapter->GetDesc(&adapterDescription);

		SAdapter adapterOutput = {};
		adapterOutput.mName                  = adapterDescription.DeviceId;
		adapterOutput.mDescription           = narrow(adapterDescription.Description);
		adapterOutput.mAvailableDeviceMemory = adapterDescription.DedicatedVideoMemory;

		while (DXGI_ERROR_NOT_FOUND != (result = adapter->EnumOutputs(outputIndex, &output)))
		{
			DXGI_OUTPUT_DESC outputDescription = {};
			output->GetDesc(&outputDescription);

			SAdapterOutput outputOutput = {};
			outputOutput.mName          = narrow(outputDescription.DeviceName);
			outputOutput.mWidth         = (outputDescription.DesktopCoordinates.right   - outputDescription.DesktopCoordinates.left);
			outputOutput.mHeight        = (outputDescription.DesktopCoordinates.bottom - outputDescription.DesktopCoordinates.top);
			outputOutput.mMonitorHandle = outputDescription.Monitor;

			uint32_t    numModesFound  = 0;
			DXGI_FORMAT requiredFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
			result = output->GetDisplayModeList(
				requiredFormat, 
				DXGI_ENUM_MODES_INTERLACED, 
				&numModesFound,
				nullptr);
			if (S_OK != result)
			{
				std::cout << "Failed to get display mode list size.\n";
				output->Release();
				adapter->Release();
				return false;
			}

			std::vector<DXGI_MODE_DESC> modes(numModesFound);

			result = output->GetDisplayModeList(
				requiredFormat,
				DXGI_ENUM_MODES_INTERLACED,
				&numModesFound, 
				modes.data());
			if (S_OK != result)
			{
				std::cout << "Failed to get display mode list.\n";
				output->Release();
				adapter->Release();
				return false;
			}

			for (uint32_t k = 0; k < numModesFound; ++k)
			{
				DXGI_MODE_DESC const &mode = modes.at(k);

				SOutputMode modeOutput  = {};
				modeOutput.mFormat      = mode.Format;
				modeOutput.mWidth       = mode.Width;
				modeOutput.mHeight      = mode.Height;
				modeOutput.mNumerator   = mode.RefreshRate.Numerator;
				modeOutput.mDenominator = mode.RefreshRate.Denominator;

				outputOutput.mOutputModes.push_back(modeOutput);
			}

			adapterOutput.mOutputs.push_back(outputOutput);
			output->Release();
			++outputIndex;
		}

		if (0 == outputIndex)
		{
			std::cout << "Warning: NO outputs on this adapter (Index: " << adapterIndex << ")\n";
		}

		adapters.mAdapters.push_back(adapterOutput);
		adapter->Release();
		++adapterIndex;
	}

	if (0 == adapterIndex)
	{
		std::cout << "No adapters found...\n";
		return false;
	}

	aOutAdapters = adapters;
	return true;
}

static bool determineLargestMode(
	SGraphicsCardAdapters const &aAdapters, 
	SAdapter                    &aOutSelectedAdapter,
	SAdapterOutput              &aOutSelectedOutput,
	SOutputMode                 &aOutSelectedMode)
{
	uint32_t currentAdapterMaxMemory = 0;
	uint32_t currentOutputMaxWidth   = 0;
	uint32_t currentModeMaxWidth     = 0;

	SAdapter       selectedAdapter = {};
	SAdapterOutput selectedOutput  = {};
	SOutputMode    selectedMode    = {};

	for (auto const&adapter : aAdapters.mAdapters)
	{
		if (adapter.mAvailableDeviceMemory > currentAdapterMaxMemory)
		{
			currentAdapterMaxMemory = adapter.mAvailableDeviceMemory;
			selectedAdapter = adapter;
		}
	}

	for (auto const&output : selectedAdapter.mOutputs)
	{
		if (output.mWidth > currentOutputMaxWidth)
		{
			currentOutputMaxWidth = output.mWidth;
			selectedOutput = output;
		}
	}

	for (auto const&mode : selectedOutput.mOutputModes)
	{
		if (mode.mWidth > currentModeMaxWidth)
		{
			currentModeMaxWidth = mode.mWidth;
			selectedMode = mode;
		}
	}

	aOutSelectedAdapter = selectedAdapter;
	aOutSelectedOutput  = selectedOutput;
	aOutSelectedMode    = selectedMode;

	return true;
}

bool DirectXState::initialize(HWND const &aHwnd)
{
	// Create DXGI (Direct X Graphics Interface) factory for all subsequent operations.
	IDXGIFactory                 *factoryUnmanaged = nullptr;
	std::shared_ptr<IDXGIFactory> factoryManaged = nullptr;

	HRESULT result = CreateDXGIFactory(__uuidof(IDXGIFactory), (void **)&factoryUnmanaged);
	if (S_OK != result)
	{
		std::cout << "Failed to create DXGI factory.\n";
		return false;
	}

	factoryManaged = makeDirectXResourcePointer(factoryUnmanaged);

	SGraphicsCardAdapters availableAdapters = {};
	bool const adaptersFound = determineGraphicsCardAdapters(factoryManaged, availableAdapters);

	SAdapter       adapter = {};
	SAdapterOutput output  = {};
	SOutputMode    mode    = {};
	bool const modeFound = determineLargestMode(availableAdapters, adapter, output, mode);

	std::cout
		<< "Selected\n"
		<< "    Adapter: " << adapter << "\n"
		<< "    Output:  " << output  << "\n"
		<< "    Mode:    " << mode    << "\n";

	// Describe SwapChain next. We use double buffering, i.e. one front buffer, one back buffer.
	DXGI_MODE_DESC bufferDescriptor = {};
	bufferDescriptor.Width                   = mode.mWidth;
	bufferDescriptor.Height                  = mode.mHeight;
	bufferDescriptor.Format                  = mode.mFormat;
	bufferDescriptor.RefreshRate.Numerator   = 0; // mode.mNumerator;
	bufferDescriptor.RefreshRate.Denominator = 1; // mode.mDenominator;

	DXGI_SWAP_CHAIN_DESC swapChainDescriptor = {};
	swapChainDescriptor.BufferCount        = 1;
	swapChainDescriptor.BufferDesc         = bufferDescriptor;
	swapChainDescriptor.BufferUsage        = DXGI_USAGE_BACK_BUFFER | DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDescriptor.SampleDesc.Count   = 1;
	swapChainDescriptor.SampleDesc.Quality = 0;
	swapChainDescriptor.Windowed           = 1;
	swapChainDescriptor.OutputWindow       = aHwnd;
	swapChainDescriptor.SwapEffect         = DXGI_SWAP_EFFECT_DISCARD;

	D3D_FEATURE_LEVEL featureLevelRequested = D3D_FEATURE_LEVEL_11_0;

	IDXGISwapChain      *swapChainUnmanaged              = nullptr;
	ID3D11Device        *deviceUnmanaged                 = nullptr;
	ID3D11DeviceContext *deviceImmediateContextUnmanaged = nullptr;

	result =
		D3D11CreateDeviceAndSwapChain(
			nullptr,                  // Adapter - Leave nullptr for the first available
			D3D_DRIVER_TYPE_HARDWARE, // Driver Type
			0,                        // SW Module Handle - Unused
			D3D11_CREATE_DEVICE_DEBUG,// Enable Debug layer
			&featureLevelRequested,   // Which feature level si required?
			1,                        // Number of feature levels in array.
			D3D11_SDK_VERSION,        // Which DX11 SDK version to use?
			&swapChainDescriptor,     // Pointer to Swapchain Descriptor
			&swapChainUnmanaged,      
			&deviceUnmanaged,
			nullptr,
			&deviceImmediateContextUnmanaged);
	if (S_OK != result)
	{
		std::cout << "Failed to create device and swapchain.\n";
		return false;
	}

	mSwapChain     = makeDirectXResourcePointer(swapChainUnmanaged);
	mDevice        = makeDirectXResourcePointer(deviceUnmanaged);
	mDeviceContext = makeDirectXResourcePointer(deviceImmediateContextUnmanaged);

	// Fetch the backbuffer RTVs.
	ID3D11Texture2D        *backBufferTextureUnmanaged = nullptr;
	ID3D11RenderTargetView *backBufferRTVUnmanaged     = nullptr;

	result = swapChainUnmanaged->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&backBufferTextureUnmanaged);
	if (S_OK != result)
	{
		std::cout << "Failed to fetch swap chain back buffer texture.\n";
		return false;
	}

	D3D11_RENDER_TARGET_VIEW_DESC backBufferRTVDescription = {};
	backBufferRTVDescription.ViewDimension      = D3D11_RTV_DIMENSION_TEXTURE2D;
	backBufferRTVDescription.Texture2D.MipSlice = 0;
	backBufferRTVDescription.Format             = swapChainDescriptor.BufferDesc.Format;

	result = deviceUnmanaged->CreateRenderTargetView(backBufferTextureUnmanaged, &backBufferRTVDescription, &backBufferRTVUnmanaged);
	if (S_OK != result)
	{
		std::cout << "Failed to create back buffer RTV.\n";
		backBufferTextureUnmanaged->Release();
		return false;
	}

	backBufferTextureUnmanaged->Release();
	mBackBufferRTV = makeDirectXResourcePointer(backBufferRTVUnmanaged);
	
	return true;
}

bool DirectXState::deinitialize()
{
	mBackBufferRTV = nullptr;
	mDeviceContext->ClearState();
	mDeviceContext = nullptr;
	mDevice        = nullptr;
	mSwapChain     = nullptr;

	return true;
}