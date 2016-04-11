#pragma once
#include "DirectXIncludes.h"
#include <comip.h>
#include <comdef.h>


// This is a very handy little utility that I wish I had known about when I was writing the RenderManager
_COM_SMARTPTR_TYPEDEF(ID3D11Texture2D, __uuidof(ID3D11Texture2D));
_COM_SMARTPTR_TYPEDEF(ID3D11RenderTargetView, __uuidof(ID3D11RenderTargetView));
_COM_SMARTPTR_TYPEDEF(ID3D11ShaderResourceView, __uuidof(ID3D11ShaderResourceView));
_COM_SMARTPTR_TYPEDEF(ID3D11DepthStencilView, __uuidof(ID3D11DepthStencilView));


struct RenderTarget2D
{
	ID3D11Texture2DPtr texture;
	ID3D11RenderTargetViewPtr rtView;
	ID3D11ShaderResourceViewPtr srView;
	UINT width;
	UINT height;
	UINT numMipLevels;
	UINT multiSamples;
	UINT msQuality;
	DXGI_FORMAT format;
	bool autoGenMipMaps;
	
	RenderTarget2D();

	void Initialize(ID3D11Device* device,
					UINT width,
				    UINT height,
					DXGI_FORMAT format,
					UINT numMipLevels = 1,
					UINT multiSamples = 1,
					UINT msQuality = 0,
					bool autoGenMipMaps  = false);
};

struct DepthStencilBuffer
{
	ID3D11Texture2DPtr texture;
	ID3D11DepthStencilViewPtr dsView;
	ID3D11ShaderResourceViewPtr srView;
	UINT width;
	UINT height;
	UINT multiSamples;
	UINT msQuality;
	DXGI_FORMAT format;

	DepthStencilBuffer();

	void Initialize(ID3D11Device* device,
					UINT width,
					UINT height,
					DXGI_FORMAT format = DXGI_FORMAT_D24_UNORM_S8_UINT,
					bool useAsShaderResource = false,
					UINT multiSamples = 1,
					UINT msQuality = 0);
};