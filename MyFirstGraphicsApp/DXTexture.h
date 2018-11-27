#pragma once

#include <iostream>
#include <stdint.h>
#include "DXCommon.h"

class DXTexture
{
public:
	static std::shared_ptr<ID3D11Texture2D> createTexture(
		std::shared_ptr<ID3D11Device>       aDevice,
		uint32_t                            aWidth,
		uint32_t                            aHeight,
		uint32_t                            aDepth,
		DXGI_FORMAT            const       &aFormat,
		D3D11_SUBRESOURCE_DATA const *const aInitialData)
	{
		D3D11_TEXTURE2D_DESC textureDescription{};
		textureDescription.Width = aWidth;
		textureDescription.Height = aHeight;
		textureDescription.Format = aFormat;
		textureDescription.ArraySize = aDepth;
		textureDescription.MipLevels = 1;
		textureDescription.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
		textureDescription.CPUAccessFlags = 0;
		textureDescription.MiscFlags = 0;
		textureDescription.SampleDesc.Count = 1;
		textureDescription.SampleDesc.Quality = 0;
		textureDescription.Usage = D3D11_USAGE_DEFAULT;

		ID3D11Texture2D *textureUnmanaged = nullptr;

		HRESULT result = aDevice->CreateTexture2D(&textureDescription, aInitialData, &textureUnmanaged);
		if (S_OK != result)
		{
			std::cout << "Failed to create texture.\n";
			return nullptr;
		}

		std::shared_ptr<ID3D11Texture2D> textureManaged = makeDirectXResourcePointer(textureUnmanaged);

		return textureManaged;
	}

	static std::shared_ptr<ID3D11ShaderResourceView> createTextureResourceView(
		std::shared_ptr<ID3D11Device>           aDevice,
		std::shared_ptr<ID3D11Texture2D>        aTexture,
		DXGI_FORMAT                      const &aFormat)
	{
		D3D11_SHADER_RESOURCE_VIEW_DESC textureViewDescription = {};
		textureViewDescription.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		textureViewDescription.Texture2D.MipLevels = 1;
		textureViewDescription.Texture2D.MostDetailedMip = 0;
		textureViewDescription.Format = aFormat;

		ID3D11ShaderResourceView *textureViewUnmanaged = nullptr;
		HRESULT result = aDevice->CreateShaderResourceView(aTexture.get(), &textureViewDescription, &textureViewUnmanaged);
		if (S_OK != result)
		{
			std::cout << "Failed to create texture view.\n";
			return nullptr;
		}

		std::shared_ptr<ID3D11ShaderResourceView> view = makeDirectXResourcePointer(textureViewUnmanaged);
		return view;
	}

	static std::shared_ptr<ID3D11RenderTargetView> createTextureRenderTargetView(
		std::shared_ptr<ID3D11Device>           aDevice,
		std::shared_ptr<ID3D11Texture2D>        aTexture,
		DXGI_FORMAT                      const &aFormat)
	{
		D3D11_RENDER_TARGET_VIEW_DESC textureViewDescription = {};
		textureViewDescription.ViewDimension      = D3D11_RTV_DIMENSION_TEXTURE2D;
		textureViewDescription.Texture2D.MipSlice = 0;
		textureViewDescription.Format             = aFormat;

		ID3D11RenderTargetView *textureViewUnmanaged = nullptr;
		HRESULT result = aDevice->CreateRenderTargetView(aTexture.get(), &textureViewDescription, &textureViewUnmanaged);
		if (S_OK != result)
		{
			std::cout << "Failed to create texture view.\n";
			return nullptr;
		}

		std::shared_ptr<ID3D11RenderTargetView> view = makeDirectXResourcePointer(textureViewUnmanaged);
		return view;
	}

	static std::shared_ptr<ID3D11SamplerState> createSampler(
		std::shared_ptr<ID3D11Device>        aDevice,
		D3D11_FILTER                  const &aFilter)
	{
		D3D11_SAMPLER_DESC samplerDescription = {};
		samplerDescription.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDescription.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDescription.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDescription.Filter   = aFilter;
		samplerDescription.ComparisonFunc = D3D11_COMPARISON_ALWAYS;

		ID3D11SamplerState *samplerUnmanaged = nullptr;

		HRESULT result = aDevice->CreateSamplerState(&samplerDescription, &samplerUnmanaged);
		if (S_OK != result)
		{
			std::cout << "Failed to create sampler state.\n";
		}

		return makeDirectXResourcePointer(samplerUnmanaged);
	}
};