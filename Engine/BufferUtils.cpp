#include "BufferUtils.h"

RenderTarget2D::RenderTarget2D()
	: width(0)
	, height(0)
	, format(DXGI_FORMAT_UNKNOWN)
	, numMipLevels(0)
	, multiSamples(0)
	, msQuality(0)
	, autoGenMipMaps(false)
{

}

void RenderTarget2D::Initialize(ID3D11Device* device,
								UINT width,
								UINT height,
								DXGI_FORMAT format,
								UINT numMipLevels,
								UINT multiSamples,
								UINT msQuality,
								bool autoGenMipMaps)
{

	D3D11_TEXTURE2D_DESC desc;
	desc.Width = width;
	desc.Height = height;
	desc.ArraySize = 1;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	desc.CPUAccessFlags = 0;
	desc.Format = format;
	desc.MipLevels = numMipLevels;
	desc.MiscFlags = (autoGenMipMaps && numMipLevels > 1) ? D3D11_RESOURCE_MISC_GENERATE_MIPS : 0;
	desc.SampleDesc.Count = multiSamples;
	desc.SampleDesc.Quality = msQuality;
	desc.Usage = D3D11_USAGE_DEFAULT;
	if (FAILED(device->CreateTexture2D(&desc, NULL, &texture)))
	{
		MessageBoxA(0,
			TEXT("Failed to Create Texture2D while creating RenderTarget2D."),
			TEXT("Graphics Error"),
			MB_OK);

		throw new std::exception("Failed to create Texture2D.");
	}

	D3D11_RENDER_TARGET_VIEW_DESC rtDesc;
	rtDesc.Format = format;
	rtDesc.ViewDimension = multiSamples > 1 ? D3D11_RTV_DIMENSION_TEXTURE2DMS : D3D11_RTV_DIMENSION_TEXTURE2D;
	rtDesc.Texture2D.MipSlice = 0;

	if (FAILED(device->CreateRenderTargetView(texture, &rtDesc, &rtView)))
	{
		MessageBoxA(0,
			TEXT("Failed to Create RenderTargetView while creating RenderTarget2D."),
			TEXT("Graphics Error"),
			MB_OK);

		throw new std::exception("Failed to create RenderTargetView.");
	}

	if (FAILED(device->CreateShaderResourceView(texture, NULL, &srView)))
	{
		MessageBoxA(0,
			TEXT("Failed to Create ShaderResourceView while creating RenderTarget2D."),
			TEXT("Graphics Error"),
			MB_OK);

		throw new std::exception("Failed to create ShaderResourceView.");
	}

	this->width = width;
	this->height = height;
	this->numMipLevels = numMipLevels;
	this->multiSamples = multiSamples;
	this->format = format;
	this->autoGenMipMaps = autoGenMipMaps;
}


DepthStencilBuffer::DepthStencilBuffer()
	: width(0)
	, height(0)
	, multiSamples(0)
	, msQuality(0)
	, format(DXGI_FORMAT_UNKNOWN)
{

}

void DepthStencilBuffer::Initialize(ID3D11Device* device,
									UINT width,
									UINT height,
									DXGI_FORMAT format,
									bool useAsShaderResource,
									UINT multiSamples,
									UINT msQuality)
{
	DXGI_FORMAT texFormat;
	if (!useAsShaderResource)
		texFormat = format;
	else if (format == DXGI_FORMAT_D16_UNORM)
		texFormat = DXGI_FORMAT_R16_TYPELESS;
	else if (format == DXGI_FORMAT_D24_UNORM_S8_UINT)
		texFormat = DXGI_FORMAT_R24G8_TYPELESS;
	else
		texFormat = DXGI_FORMAT_R32_TYPELESS;

	D3D11_TEXTURE2D_DESC desc;
	desc.Width = width;
	desc.Height = height;
	desc.ArraySize = 1;
	desc.BindFlags = useAsShaderResource ? D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE : D3D11_BIND_DEPTH_STENCIL;
	desc.CPUAccessFlags = 0;
	desc.Format = texFormat;
	desc.MipLevels = 1;
	desc.MiscFlags = 0;
	desc.SampleDesc.Count = multiSamples;
	desc.SampleDesc.Quality = msQuality;
	desc.Usage = D3D11_USAGE_DEFAULT;

	if (FAILED(device->CreateTexture2D(&desc, nullptr, &texture)))
	{
		MessageBoxA(0,
			TEXT("Failed to Create Texture2D while creating DepthStencilBuffer."),
			TEXT("Graphics Error"),
			MB_OK);

		throw new std::exception("Failed to create Texture2D.");
	}

	D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc;
	dsvDesc.Format = format;
	dsvDesc.ViewDimension = multiSamples > 1 ? D3D11_DSV_DIMENSION_TEXTURE2DMS : D3D11_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Texture2D.MipSlice = 0;
	dsvDesc.Flags = 0;
	
	if (FAILED(device->CreateDepthStencilView(texture, &dsvDesc, &dsView)))
	{
		MessageBoxA(0,
			TEXT("Failed to Create DepthStencilView while creating DepthStencilBuffer."),
			TEXT("Graphics Error"),
			MB_OK);

		throw new std::exception("Failed to create DepthStencilView.");
	}

	// TODO: Create a read only ds view here if necessary later on

	if (useAsShaderResource)
	{
		DXGI_FORMAT srvFormat;
		if (format == DXGI_FORMAT_D16_UNORM)
			srvFormat = DXGI_FORMAT_R16_UNORM;
		else if (format == DXGI_FORMAT_D24_UNORM_S8_UINT)
			srvFormat = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
		else
			srvFormat = DXGI_FORMAT_R32_FLOAT;

		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
		srvDesc.Format = srvFormat;
		srvDesc.ViewDimension = multiSamples > 1 ? D3D11_SRV_DIMENSION_TEXTURE2DMS : D3D11_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = 1;
		srvDesc.Texture2D.MostDetailedMip = 0;

		if (FAILED(device->CreateShaderResourceView(texture, &srvDesc, &srView)))
		{
			MessageBoxA(0,
				TEXT("Failed to Create ShaderResourceView while creating DepthStencilBuffer."),
				TEXT("Graphics Error"),
				MB_OK);

			throw new std::exception("Failed to create ShaderResourceView.");
		}
	}
	else
	{
		srView = nullptr;
	}

	this->width = width;
	this->height = height;
	this->multiSamples = multiSamples;
	this->format = format;
}