#include "RenderManager.h"
#include "WindowsManager.h"
#include "BlitVertexShader.h"
#include "BlitPixelShader.h"

using namespace DirectX;

// ----------------------------------------------------------------------------------
// Singleton Stuff
// ----------------------------------------------------------------------------------
template<> RenderManager* Singleton<RenderManager>::msSingleton = 0;
RenderManager& RenderManager::GetSingleton(void)
{
	assert(msSingleton);
	return *msSingleton;
}

RenderManager* RenderManager::GetSingletonPtr(void)
{
	return msSingleton;
}

// ----------------------------------------------------------------------------------
// Helper Functions
// ----------------------------------------------------------------------------------

// This function was inspired by:
// http://www.rastertek.com/dx11tut03.html
DXGI_RATIONAL QueryRefreshRate()
{
	DXGI_RATIONAL refreshRate = { 0, 1 };
	if (WindowsManager::GetSingleton().GetVSync())
	{
		Microsoft::WRL::ComPtr<IDXGIFactory2> factory;
		Microsoft::WRL::ComPtr<IDXGIAdapter> adapter;
		Microsoft::WRL::ComPtr<IDXGIOutput> adapterOutput;

		DXGI_MODE_DESC* displayModeList;

		// Create a DirectX graphics interface factory.
		HRESULT hr = CreateDXGIFactory(__uuidof(IDXGIFactory2), &factory);
		if (FAILED(hr))
		{
			MessageBoxA(0,
				TEXT("Could not create DXGIFactory instance."),
				TEXT("Query Refresh Rate"),
				MB_OK);

			throw new std::exception("Failed to create DXGIFactory.");
		}

		hr = factory->EnumAdapters(0, &adapter);
		if (FAILED(hr))
		{
			MessageBoxA(0,
				TEXT("Failed to enumerate adapters."),
				TEXT("Query Refresh Rate"),
				MB_OK);

			throw new std::exception("Failed to enumerate adapters.");
		}

		hr = adapter->EnumOutputs(0, &adapterOutput);
		if (FAILED(hr))
		{
			MessageBoxA(0,
				TEXT("Failed to enumerate adapter outputs."),
				TEXT("Query Refresh Rate"),
				MB_OK);

			throw new std::exception("Failed to enumerate adapter outputs.");
		}

		UINT numDisplayModes;
		hr = adapterOutput->GetDisplayModeList(DXGI_FORMAT_B8G8R8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numDisplayModes, nullptr);
		if (FAILED(hr))
		{
			MessageBoxA(0,
				TEXT("Failed to query display mode list."),
				TEXT("Query Refresh Rate"),
				MB_OK);

			throw new std::exception("Failed to query display mode list.");
		}

		displayModeList = new DXGI_MODE_DESC[numDisplayModes];
		assert(displayModeList);

		hr = adapterOutput->GetDisplayModeList(DXGI_FORMAT_B8G8R8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numDisplayModes, displayModeList);
		if (FAILED(hr))
		{
			MessageBoxA(0,
				TEXT("Failed to query display mode list."),
				TEXT("Query Refresh Rate"),
				MB_OK);

			throw new std::exception("Failed to query display mode list.");
		}

		// Now store the refresh rate of the monitor that matches the width and height of the requested screen.
		for (UINT i = 0; i < numDisplayModes; ++i)
		{
			if (displayModeList[i].Width == WindowsManager::GetSingleton().GetClientWidth() &&
				displayModeList[i].Height == WindowsManager::GetSingleton().GetClientHeight())
			{
				refreshRate = displayModeList[i].RefreshRate;
			}
		}

		delete[] displayModeList;
	}

	return refreshRate;
}

// ----------------------------------------------------------------------------------
// Implementation
// ----------------------------------------------------------------------------------

// Constant buffer struct definition for blitting
//-------------------------------------------------------
struct BlitConstantBufferVertex
{
	XMMATRIX WorldViewProjectionMatrix;
};

RenderManager::RenderManager() 
	: m_d3dDevice(nullptr)
	, m_d3dDeviceContext(nullptr)
	, m_d3dSwapChain(nullptr)
	, m_d3dRenderTargetView(nullptr)
	, m_d3dDepthStencilView(nullptr)
	, m_d3dDepthStencilBuffer(nullptr)
	, m_meshID(0)
	, m_vsID(0)
	, m_psID(0)
	, m_cbID(0)
	, m_matID(0)
	, m_curMesh(0)
	, m_curVS(0)
	, m_curPS(0)
	, m_curMat(0)
{
	Initialize();
	InitStates();
}

RenderManager::~RenderManager()
{
}

bool RenderManager::ResizeSwapChain(int width, int height)
{
	// Don't allow for 0 size swap chain buffers.
	if (width <= 0) width = 1;
	if (height <= 0) height = 1;

	m_d3dDeviceContext->OMSetRenderTargets(0, nullptr, nullptr);

	// First release the render target and depth/stencil views.
	m_d3dRenderTargetView.Reset();
	m_d3dDepthStencilView.Reset();
	m_d3dDepthStencilBuffer.Reset();

	// Resize the swap chain buffers.
	m_d3dSwapChain->ResizeBuffers(1, width, height, DXGI_FORMAT_R8G8B8A8_UNORM, 0);

	// Next initialize the back buffer of the swap chain and associate it to a 
	// render target view.
	Microsoft::WRL::ComPtr<ID3D11Texture2D> backBuffer;
	HRESULT hr = m_d3dSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), &backBuffer);
	if (FAILED(hr))
	{
		MessageBoxA(WindowsManager::GetSingleton().GetWindowHandle(), 
			"Failed to retrieve the swap chain back buffer.", "Error", MB_OK | MB_ICONERROR);
		return false;
	}

	hr = m_d3dDevice->CreateRenderTargetView(backBuffer.Get(), nullptr, &m_d3dRenderTargetView);
	if (FAILED(hr))
	{
		MessageBoxA(WindowsManager::GetSingleton().GetWindowHandle(), 
			"Failed to create the RenderTargetView.", "Error", MB_OK | MB_ICONERROR);
		return false;
	}

	backBuffer.Reset();

	// Create the depth buffer for use with the depth/stencil view.
	D3D11_TEXTURE2D_DESC depthStencilBufferDesc;
	ZeroMemory(&depthStencilBufferDesc, sizeof(D3D11_TEXTURE2D_DESC));

	depthStencilBufferDesc.ArraySize = 1;
	depthStencilBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthStencilBufferDesc.CPUAccessFlags = 0; // No CPU access required.
	depthStencilBufferDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilBufferDesc.Width = width;
	depthStencilBufferDesc.Height = height;
	depthStencilBufferDesc.MipLevels = 1;
	depthStencilBufferDesc.SampleDesc.Count = 1;
	depthStencilBufferDesc.SampleDesc.Quality = 0;
	depthStencilBufferDesc.Usage = D3D11_USAGE_DEFAULT;

	hr = m_d3dDevice->CreateTexture2D(&depthStencilBufferDesc, nullptr, &m_d3dDepthStencilBuffer);
	if (FAILED(hr))
	{
		MessageBoxA(WindowsManager::GetSingleton().GetWindowHandle(), 
			"Failed to create the Depth/Stencil texture.", "Error", MB_OK | MB_ICONERROR);
		return false;
	}

	hr = m_d3dDevice->CreateDepthStencilView(m_d3dDepthStencilBuffer.Get(), nullptr, &m_d3dDepthStencilView);
	if (FAILED(hr))
	{
		MessageBoxA(WindowsManager::GetSingleton().GetWindowHandle(), 
			"Failed to create DepthStencilView.", "Error", MB_OK | MB_ICONERROR);
		return false;
	}

	return true;
}


bool RenderManager::Initialize()
{
	// Check for DirectX Math library support.
	if (!DirectX::XMVerifyCPUSupport())
	{
		MessageBoxA(WindowsManager::GetSingleton().GetWindowHandle(), 
			"Failed to verify DirectX Math library support.", "Error", MB_OK | MB_ICONERROR);
		return false;
	}

	HRESULT hr = 0;
	UINT createDeviceFlags = 0;

	// These are the feature levels that we will accept.
	D3D_FEATURE_LEVEL featureLevels[] =
	{
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
		D3D_FEATURE_LEVEL_9_3,
		D3D_FEATURE_LEVEL_9_2,
		D3D_FEATURE_LEVEL_9_1
	};

	// This will be the feature level that 
	// is used to create our device and swap chain.
	D3D_FEATURE_LEVEL featureLevel;

	hr = D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE,
		nullptr, createDeviceFlags, featureLevels, _countof(featureLevels),
		D3D11_SDK_VERSION, &m_d3dDevice, &featureLevel, &m_d3dDeviceContext);

	// If 11.1 failed, try using 11.0
	if (hr == E_INVALIDARG)
	{
		hr = D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE,
			nullptr, createDeviceFlags, &featureLevels[1], _countof(featureLevels) - 1,
			D3D11_SDK_VERSION, &m_d3dDevice, &featureLevel, &m_d3dDeviceContext);
	}

	if (FAILED(hr))
	{
		MessageBoxA(WindowsManager::GetSingleton().GetWindowHandle(), 
			"Failed to create DirectX 11 Device.", "Error", MB_OK | MB_ICONERROR);
		return false;
	}

	Microsoft::WRL::ComPtr<IDXGIFactory2> factory;
	hr = CreateDXGIFactory(__uuidof(IDXGIFactory2), &factory);
	if (FAILED(hr))
	{
		MessageBoxA(WindowsManager::GetSingleton().GetWindowHandle(), 
			"Failed to create IDXGIFactory2.", "Error", MB_OK | MB_ICONERROR);
		return false;
	}

	DXGI_SWAP_CHAIN_DESC1 swapChainDesc;
	ZeroMemory(&swapChainDesc, sizeof(DXGI_SWAP_CHAIN_DESC1));

	swapChainDesc.BufferCount = 1;
	swapChainDesc.Width = WindowsManager::GetSingleton().GetClientWidth();
	swapChainDesc.Height = WindowsManager::GetSingleton().GetClientHeight();
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH; // Use Alt-Enter to switch between full screen and windowed mode.

	DXGI_SWAP_CHAIN_FULLSCREEN_DESC swapChainFullScreenDesc;
	ZeroMemory(&swapChainFullScreenDesc, sizeof(DXGI_SWAP_CHAIN_FULLSCREEN_DESC));

	swapChainFullScreenDesc.RefreshRate = QueryRefreshRate();
	swapChainFullScreenDesc.Windowed = WindowsManager::GetSingleton().GetWindowed();

	hr = factory->CreateSwapChainForHwnd(m_d3dDevice.Get(), WindowsManager::GetSingleton().GetWindowHandle(),
		&swapChainDesc, &swapChainFullScreenDesc, nullptr, &m_d3dSwapChain);

	if (FAILED(hr))
	{
		MessageBoxA(WindowsManager::GetSingleton().GetWindowHandle(), 
			"Failed to create swap chain.", "Error", MB_OK | MB_ICONERROR);
		return false;
	}

	if (!ResizeSwapChain(WindowsManager::GetSingleton().GetClientWidth(), WindowsManager::GetSingleton().GetClientHeight()))
	{
		MessageBoxA(WindowsManager::GetSingleton().GetWindowHandle(), 
			"Failed to resize the swap chain.", "Error", MB_OK | MB_ICONERROR);
		return false;
	}

// START DEBUG
	SetViewport((float)WindowsManager::GetSingleton().GetClientWidth(),
				(float)WindowsManager::GetSingleton().GetClientHeight());

	SetRasterizerState(CULL_BACK);
	SetDepthStencilState(DEPTH_STENCIL_STATE::READ_WRITE);
	
	m_d3dDeviceContext->OMSetRenderTargets(1, m_d3dRenderTargetView.GetAddressOf(), m_d3dDepthStencilView.Get());
// END DEBUG

	ZeroMemory(&m_PresentParameters, sizeof(DXGI_PRESENT_PARAMETERS));

	// Asset Creation!
	RHANDLE blitVS = CreateVShaderResource(g_BlitVertexShader, sizeof(g_BlitVertexShader), VertexPositionNormalTexture::InputElements, VertexPositionNormalTexture::InputElementCount);
	RHANDLE blitPS = CreatePShaderResource(g_BlitPixelShader, sizeof(g_BlitPixelShader));
	//RHANDLE blitCB = CreateCBResource(sizeof(BlitConstantBufferVertex));
	m_blitMaterial = CreateMaterial(blitVS, blitPS);
	m_blitQuad	   = CreateMeshResource(Mesh::CreateQuad(m_d3dDeviceContext.Get(), 2.0f, 2.0f));
	m_blitVS	   = blitVS;

	return true;
}

bool RenderManager::InitStates()
{
	return
		CreateRasterizerState(D3D11_CULL_FRONT, D3D11_FILL_SOLID, &m_rasterizerStates[CULL_FRONT]) &&
		CreateRasterizerState(D3D11_CULL_BACK, D3D11_FILL_SOLID, &m_rasterizerStates[CULL_BACK]) &&
		CreateRasterizerState(D3D11_CULL_NONE, D3D11_FILL_SOLID, &m_rasterizerStates[CULL_NONE]) &&
		CreateRasterizerState(D3D11_CULL_NONE, D3D11_FILL_WIREFRAME, &m_rasterizerStates[WIREFRAME]) &&

		CreateDepthStencilState(true, true, false, false, D3D11_COMPARISON_ALWAYS, &m_depthStencilStates[READ_WRITE]) &&
		CreateDepthStencilState(true, false, false, false, D3D11_COMPARISON_ALWAYS, &m_depthStencilStates[READ_ONLY]) &&
		CreateDepthStencilState(false, true, false, false, D3D11_COMPARISON_ALWAYS, &m_depthStencilStates[WRITE_ONLY]) &&
		CreateDepthStencilState(false, false, false, false, D3D11_COMPARISON_ALWAYS, &m_depthStencilStates[DEPTH_NONE]) &&
		CreateDepthStencilState(true, false, true, true, D3D11_COMPARISON_ALWAYS, &m_depthStencilStates[STENCIL_WRITE]) &&
		CreateDepthStencilState(true, true, true, false, D3D11_COMPARISON_GREATER, &m_depthStencilStates[STENCIL_GT]) &&
		CreateDepthStencilState(true, true, true, false, D3D11_COMPARISON_LESS, &m_depthStencilStates[STENCIL_LT]) &&
		CreateDepthStencilState(true, true, true, false, D3D11_COMPARISON_EQUAL, &m_depthStencilStates[STENCIL_EQ]) &&
		CreateDepthStencilState(false, false, true, false, D3D11_COMPARISON_GREATER, &m_depthStencilStates[READONLY_STENCIL_GT]) &&
		CreateDepthStencilState(false, false, true, false, D3D11_COMPARISON_LESS, &m_depthStencilStates[READONLY_STENCIL_LT]) &&
		CreateDepthStencilState(false, false, true, false, D3D11_COMPARISON_EQUAL, &m_depthStencilStates[READONLY_STENCIL_EQ]) &&

		CreateBlendState(D3D11_BLEND_ONE, D3D11_BLEND_ZERO, true, &m_blendStates[SOLID]) &&
		CreateBlendState(D3D11_BLEND_SRC_ALPHA, D3D11_BLEND_INV_SRC_ALPHA, true, &m_blendStates[ALPHA_BLEND]) &&
		CreateBlendState(D3D11_BLEND_ONE, D3D11_BLEND_INV_SRC_ALPHA, true, &m_blendStates[PREMULTIPLIED_ALPHA]) &&
		CreateBlendState(D3D11_BLEND_SRC_ALPHA, D3D11_BLEND_ONE, true, &m_blendStates[ADDITIVE_BLEND]) &&
		CreateBlendState(D3D11_BLEND_SRC_ALPHA, D3D11_BLEND_ONE, false, &m_blendStates[NO_COLOR]) &&

		CreateSamplerState(D3D11_FILTER_MIN_MAG_MIP_POINT, D3D11_TEXTURE_ADDRESS_CLAMP, &m_samplerStates[POINT_CLAMP]) &&
		CreateSamplerState(D3D11_FILTER_MIN_MAG_MIP_POINT, D3D11_TEXTURE_ADDRESS_WRAP, &m_samplerStates[POINT_WRAP]) &&
		CreateSamplerState(D3D11_FILTER_MIN_MAG_MIP_LINEAR, D3D11_TEXTURE_ADDRESS_CLAMP, &m_samplerStates[LINEAR_CLAMP]) &&
		CreateSamplerState(D3D11_FILTER_MIN_MAG_MIP_LINEAR, D3D11_TEXTURE_ADDRESS_WRAP, &m_samplerStates[LINEAR_WRAP]) &&
		CreateSamplerState(D3D11_FILTER_ANISOTROPIC, D3D11_TEXTURE_ADDRESS_CLAMP, &m_samplerStates[ANISOTROPIC_CLAMP]) &&
		CreateSamplerState(D3D11_FILTER_ANISOTROPIC, D3D11_TEXTURE_ADDRESS_WRAP, &m_samplerStates[ANISOTROPIC_WRAP]);
}

bool RenderManager::CreateRasterizerState(D3D11_CULL_MODE cullMode, D3D11_FILL_MODE fillMode, ID3D11RasterizerState** pResult)
{
	// Setup rasterizer state.
	D3D11_RASTERIZER_DESC rasterizerDesc;
	ZeroMemory(&rasterizerDesc, sizeof(D3D11_RASTERIZER_DESC));

	rasterizerDesc.CullMode = cullMode;
	rasterizerDesc.DepthClipEnable = TRUE;
	rasterizerDesc.FillMode = fillMode;
	rasterizerDesc.MultisampleEnable = FALSE;

	// Create the rasterizer state object.
	HRESULT hr = m_d3dDevice->CreateRasterizerState(&rasterizerDesc, pResult);
	if (FAILED(hr))
	{
		MessageBoxA(WindowsManager::GetSingleton().GetWindowHandle(),
			"Failed to create a RasterizerState object.", "Error", MB_OK | MB_ICONERROR);

		return false;
	}

	return true;
}

bool RenderManager::CreateDepthStencilState(bool enable, bool writeEnable, bool stencilEnable, bool stencilWriteEnable, D3D11_COMPARISON_FUNC stencilFunc, ID3D11DepthStencilState** pResult)
{
	D3D11_DEPTH_STENCIL_DESC depthStencilStateDesc;
	ZeroMemory(&depthStencilStateDesc, sizeof(D3D11_DEPTH_STENCIL_DESC));

	depthStencilStateDesc.DepthEnable = enable;
	depthStencilStateDesc.DepthWriteMask = writeEnable ? D3D11_DEPTH_WRITE_MASK_ALL : D3D11_DEPTH_WRITE_MASK_ZERO;
	depthStencilStateDesc.DepthFunc = D3D11_COMPARISON_LESS;
	depthStencilStateDesc.StencilEnable = stencilEnable;
	depthStencilStateDesc.StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK;
	depthStencilStateDesc.StencilWriteMask = stencilWriteEnable ? 0xFF : 0;
	depthStencilStateDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilStateDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilStateDesc.FrontFace.StencilPassOp = stencilWriteEnable ? D3D11_STENCIL_OP_REPLACE : D3D11_STENCIL_OP_KEEP;
	depthStencilStateDesc.FrontFace.StencilFunc = stencilFunc;
	depthStencilStateDesc.BackFace = depthStencilStateDesc.FrontFace;

	HRESULT hr = m_d3dDevice->CreateDepthStencilState(&depthStencilStateDesc, pResult);
	if (FAILED(hr))
	{
		MessageBoxA(WindowsManager::GetSingleton().GetWindowHandle(),
			"Failed to create a DepthStencilState object.", "Error", MB_OK | MB_ICONERROR);

		return false;
	}

	return true;
}

bool RenderManager::CreateBlendState(D3D11_BLEND srcBlend, D3D11_BLEND destBlend, bool colorWriteEnable, ID3D11BlendState** pResult)
{
	D3D11_BLEND_DESC blendStateDesc;
	ZeroMemory(&blendStateDesc, sizeof(blendStateDesc));

	blendStateDesc.RenderTarget[0].BlendEnable = (srcBlend != D3D11_BLEND_ONE) || (destBlend != D3D11_BLEND_ZERO);
	blendStateDesc.RenderTarget[0].SrcBlend = blendStateDesc.RenderTarget[0].SrcBlendAlpha = srcBlend;
	blendStateDesc.RenderTarget[0].DestBlend = blendStateDesc.RenderTarget[0].DestBlendAlpha = destBlend;
	blendStateDesc.RenderTarget[0].BlendOp = blendStateDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blendStateDesc.RenderTarget[0].RenderTargetWriteMask = colorWriteEnable ? D3D11_COLOR_WRITE_ENABLE_ALL : 0;

	HRESULT hr = m_d3dDevice->CreateBlendState(&blendStateDesc, pResult);
	if (FAILED(hr))
	{
		MessageBoxA(WindowsManager::GetSingleton().GetWindowHandle(),
			"Failed to create a BlendState object.", "Error", MB_OK | MB_ICONERROR);

		return false;
	}

	return true;
}

bool RenderManager::CreateSamplerState(D3D11_FILTER filter, D3D11_TEXTURE_ADDRESS_MODE addressMode, ID3D11SamplerState** pResult)
{
	D3D11_SAMPLER_DESC samplerStateDesc;
	ZeroMemory(&samplerStateDesc, sizeof(samplerStateDesc));

	samplerStateDesc.Filter = filter;
	samplerStateDesc.AddressU = addressMode;
	samplerStateDesc.AddressV = addressMode;
	samplerStateDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerStateDesc.MaxAnisotropy = 1;//(m_d3dDevice->GetFeatureLevel() > D3D_FEATURE_LEVEL_9_1) ? 16 : 2;
	samplerStateDesc.MipLODBias = 0.0f;
	samplerStateDesc.MinLOD = -FLT_MAX;
	samplerStateDesc.MaxLOD = FLT_MAX;
	samplerStateDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;

	HRESULT hr = m_d3dDevice->CreateSamplerState(&samplerStateDesc, pResult);
	if (FAILED(hr))
	{
		MessageBoxA(WindowsManager::GetSingleton().GetWindowHandle(),
			"Failed to create a SamplerState object.", "Error", MB_OK | MB_ICONERROR);

		return false;
	}

	return true;
}

ID3D11Device* RenderManager::GetDevice() const
{
	return m_d3dDevice.Get();
}

ID3D11DeviceContext* RenderManager::GetDeviceContext() const
{
	return m_d3dDeviceContext.Get();
}

ID3D11RenderTargetView** RenderManager::GetBackBufferRT() const
{
	return const_cast<ID3D11RenderTargetView**>(m_d3dRenderTargetView.GetAddressOf());
}

ID3D11DepthStencilView** RenderManager::GetDepthStencil() const
{
	return const_cast<ID3D11DepthStencilView**>(m_d3dDepthStencilView.GetAddressOf());
}


void RenderManager::ResetRenderTarget()
{
	m_d3dDeviceContext->OMSetRenderTargets(1, m_d3dRenderTargetView.GetAddressOf(), m_d3dDepthStencilView.Get());
}

void RenderManager::SetRasterizerState(RASTERIZER_STATE state)
{
	m_d3dDeviceContext->RSSetState(m_rasterizerStates[state].Get());
}

void RenderManager::SetDepthStencilState(DEPTH_STENCIL_STATE state, UINT depthStencilWriteValue)
{
	m_d3dDeviceContext->OMSetDepthStencilState(m_depthStencilStates[state].Get(), depthStencilWriteValue);
}

void RenderManager::SetBlendState(BLEND_STATE state)
{
	m_d3dDeviceContext->OMSetBlendState(m_blendStates[state].Get(), nullptr, 0xFFFFFFFF);
}

void RenderManager::SetSamplerState(SAMPLER_STATE state, UINT startSlot, UINT numSamplers)
{
	m_d3dDeviceContext->PSSetSamplers(startSlot, numSamplers, m_samplerStates[state].GetAddressOf());
}

void RenderManager::Blit(ID3D11ShaderResourceView* src, ID3D11RenderTargetView* dst, SAMPLER_STATE samplerState, bool useDepth)
{
	// Save the previous render targets
	ID3D11RenderTargetView* rt;
	ID3D11DepthStencilView* ds;
	m_d3dDeviceContext->OMGetRenderTargets(1, &rt, &ds);

	m_d3dDeviceContext->OMSetRenderTargets(1, dst == nullptr ? m_d3dRenderTargetView.GetAddressOf() : &dst, useDepth ? ds : nullptr);
	m_d3dDeviceContext->PSSetShaderResources(0, 1, &src);
	m_d3dDeviceContext->PSSetSamplers(0, 1, m_samplerStates[samplerState].GetAddressOf());

	DrawWithMaterial(m_blitQuad, m_blitMaterial);

	// Restor the original render targets
	m_d3dDeviceContext->OMSetRenderTargets(1, &rt, ds);
}

void RenderManager::RenderFullscreen(RHANDLE pShader, ID3D11RenderTargetView* dst)
{
	// Save the previous render targets
	ID3D11RenderTargetView* rt;
	ID3D11DepthStencilView* ds;
	m_d3dDeviceContext->OMGetRenderTargets(1, &rt, &ds);

	m_d3dDeviceContext->OMSetRenderTargets(1, dst == nullptr ? m_d3dRenderTargetView.GetAddressOf() : &dst, nullptr);
	SetVertexShader(m_blitVS);
	SetPixelShader(pShader);

	m_meshMap[m_blitQuad]->Draw(m_d3dDeviceContext.Get());

	m_d3dDeviceContext->OMSetRenderTargets(1, &rt, ds);
}

void RenderManager::SetViewport(float width, float height, float topLeftX, float topLeftY, float minDepth, float maxDepth)
{
	m_viewport.TopLeftX = static_cast<FLOAT>(topLeftX);
	m_viewport.TopLeftY = static_cast<FLOAT>(topLeftY);
	m_viewport.Width = static_cast<FLOAT>(width);
	m_viewport.Height = static_cast<FLOAT>(height);
	m_viewport.MinDepth = static_cast<FLOAT>(minDepth);
	m_viewport.MaxDepth = static_cast<FLOAT>(maxDepth);

	m_d3dDeviceContext->RSSetViewports(1, &m_viewport);
}

void RenderManager::Clear(const FLOAT clearColor[4], FLOAT clearDepth, UINT8 clearStencil)
{
	assert(m_d3dDeviceContext);
	ID3D11RenderTargetView* rt = nullptr;
	ID3D11DepthStencilView* dsv = nullptr;
	m_d3dDeviceContext->OMGetRenderTargets(1, &rt, &dsv);
	m_d3dDeviceContext->ClearRenderTargetView(rt, clearColor);
	m_d3dDeviceContext->ClearDepthStencilView(dsv, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, clearDepth, clearStencil);
}

void RenderManager::Present()
{
	if (WindowsManager::GetSingleton().GetVSync())
	{
		m_d3dSwapChain->Present1(1, 0, &m_PresentParameters);
	}
	else
	{
		m_d3dSwapChain->Present1(0, 0, &m_PresentParameters);
	}
}

// ---------------------------------------------------------------------------------------------------------------
// Resource Creation and Management functions
// ---------------------------------------------------------------------------------------------------------------

// Aliases for readability
typedef std::unordered_map<RHANDLE, std::unique_ptr<Mesh>> MeshMap;
typedef std::unordered_map<RHANDLE, Microsoft::WRL::ComPtr<ID3D11VertexShader>> VSMap;
typedef std::unordered_map<RHANDLE, Microsoft::WRL::ComPtr<ID3D11PixelShader>> PSMap;
typedef std::unordered_map<RHANDLE, Microsoft::WRL::ComPtr<ID3D11Buffer>> CBMap;
typedef std::unordered_map<RHANDLE, std::unique_ptr<Material>> MaterialMap;

RHANDLE RenderManager::CreateMeshResource(std::unique_ptr<Mesh> mesh)
{
	m_meshMap.insert(MeshMap::value_type(++m_meshID, move(mesh)));
	return m_meshID;
}

/*
RHANDLE RenderManager::CreateVShaderResource(const void* shaderBytecode, size_t bytecodeSize)
{
	HRESULT hr = 0;
	RHANDLE handle = ++m_vsID;

	hr = m_d3dDevice->CreateVertexShader(shaderBytecode, bytecodeSize, nullptr, &m_vShaderMap[handle]);
	if (FAILED(hr))
	{
		MessageBoxA(WindowsManager::GetSingleton().GetWindowHandle(),
			"Failed to create Vertex Shader.", "Error", MB_OK | MB_ICONERROR);
		return -1;
	}
	
	std::printf("Vertex Shader Null: %d \n", m_vShaderMap[handle].Get() == nullptr);
	return handle;
}
*/

RHANDLE RenderManager::CreateVShaderResource(const void* shaderBytecode, size_t bytecodeSize, const D3D11_INPUT_ELEMENT_DESC* elementDesc, UINT elementCount )
{
	HRESULT hr = 0;
	RHANDLE handle = ++m_vsID;

	hr = m_d3dDevice->CreateVertexShader(shaderBytecode, bytecodeSize, nullptr, &m_vShaderMap[handle]);
	if (FAILED(hr))
	{
		MessageBoxA(WindowsManager::GetSingleton().GetWindowHandle(),
			"Failed to create Vertex Shader.", "Error", MB_OK | MB_ICONERROR);
		return -1;
	}

	hr = m_d3dDevice->CreateInputLayout(elementDesc, elementCount, shaderBytecode, bytecodeSize, &m_vsInputLayoutMap[handle]);
	if (FAILED(hr))
	{
		MessageBoxA(WindowsManager::GetSingleton().GetWindowHandle(),
			"Failed to create Vertex Shader Input Layout.", "Error", MB_OK | MB_ICONERROR);
		return false;
	}

	return handle;
}

RHANDLE RenderManager::CreatePShaderResource(const void* shaderBytecode, size_t bytecodeSize)
{
	HRESULT hr = 0;
	RHANDLE handle = ++m_psID;

	hr = m_d3dDevice->CreatePixelShader(shaderBytecode, bytecodeSize, nullptr, &m_pShaderMap[handle]);
	if (FAILED(hr))
	{
		MessageBoxA(WindowsManager::GetSingleton().GetWindowHandle(),
			"Failed to create Pixel Shader.", "Error", MB_OK | MB_ICONERROR);
		return -1;
	}
	
	return handle;
}

RHANDLE RenderManager::CreateCBResource(size_t bufferSize)
{
	HRESULT hr = 0;
	RHANDLE handle = ++m_cbID;
	D3D11_BUFFER_DESC constantBufferDesc;
	ZeroMemory(&constantBufferDesc, sizeof(D3D11_BUFFER_DESC));

	constantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	constantBufferDesc.ByteWidth = bufferSize;
	constantBufferDesc.CPUAccessFlags = 0;
	constantBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	
	hr = m_d3dDevice->CreateBuffer(&constantBufferDesc, nullptr, &m_cBufferMap[handle]);
	if (FAILED(hr))
	{
		MessageBoxA(WindowsManager::GetSingleton().GetWindowHandle(),
			"Failed to create Constant Buffer.", "Error", MB_OK | MB_ICONERROR);
		return false;
	}

	return handle;
}

RHANDLE RenderManager::CreateMaterial(RHANDLE vShader, RHANDLE pShader, RHANDLE vCBuffer, RHANDLE pCBuffer)
{
	assert(vShader > 0 && vShader <= m_vsID);
	assert(pShader > 0 && vShader <= m_psID);
	//assert(vCBuffer > 0 && vCBuffer <= m_cbID);
	//assert(pCBuffer > 0 && pCBuffer <= m_cbID);

	RHANDLE handle = ++m_matID;
	std::unique_ptr<Material> temp(new Material(vShader, pShader, vCBuffer, pCBuffer));
	m_materialMap.insert(MaterialMap::value_type(handle, move(temp)));

	return handle;
}

void RenderManager::SetMaterial(RHANDLE materialHandle)
{
	if (m_curMat == materialHandle)
		return;

	assert(materialHandle > 0 && materialHandle <= m_matID);

	Material* material = m_materialMap[materialHandle].get();
	SetVertexShader(material->vsHandle);
	SetPixelShader(material->psHandle);
	if(material->vsCBHandle != 0)
		SetVSConstantBuffer(material->vsCBHandle);
	if(material->psCBHandle != 0)
		SetPSConstantBuffer(material->psCBHandle);
	
	m_curMat = materialHandle;
}

void RenderManager::SetVertexShader(RHANDLE shaderHandle)
{
	if (m_curVS == shaderHandle)
		return;

	assert(shaderHandle > 0 && shaderHandle <= m_vsID);

	m_d3dDeviceContext->VSSetShader(m_vShaderMap[shaderHandle].Get(), nullptr, 0);
	m_d3dDeviceContext->IASetInputLayout(m_vsInputLayoutMap[shaderHandle].Get());

	m_curVS = shaderHandle;
}


void RenderManager::SetPixelShader(RHANDLE shaderHandle)
{
	if (m_curPS == shaderHandle)
		return;

	assert(shaderHandle > 0 && shaderHandle <= m_psID);

	m_d3dDeviceContext->PSSetShader(m_pShaderMap[shaderHandle].Get(), nullptr, 0);
	
	m_curPS = shaderHandle;
}

void RenderManager::SetVSConstantBuffer(RHANDLE cbHandle)
{
	//assert(cbHandle > 0 && cbHandle <= m_cbID);
	if (cbHandle == 0 || cbHandle > m_cbID)
		return;

	m_d3dDeviceContext->VSSetConstantBuffers(0, 1, m_cBufferMap[cbHandle].GetAddressOf());
}

void RenderManager::SetPSConstantBuffer(RHANDLE cbHandle)
{
	//assert(cbHandle > 0 && cbHandle <= m_cbID);
	if (cbHandle == 0 || cbHandle > m_cbID)
		return;

	m_d3dDeviceContext->PSSetConstantBuffers(0, 1, m_cBufferMap[cbHandle].GetAddressOf());
}

void RenderManager::UpdateConstantBuffer(RHANDLE cbHandle, const void* cbData)
{
	assert(cbHandle > 0 && cbHandle <= m_cbID);

	m_d3dDeviceContext->UpdateSubresource(m_cBufferMap[cbHandle].Get(), 0, nullptr, cbData, 0, 0);
}

void RenderManager::DrawWithMaterial(RHANDLE meshHandle, RHANDLE materialHandle)
{
	assert(meshHandle > 0 && meshHandle <= m_meshID);
	assert(materialHandle > 0 && materialHandle <= m_matID);

	SetMaterial(materialHandle);

	// Draw Mesh
	// ToDo: Break mesh drawing into two parts: Setting Resources and Calling "DrawIndexed"
	m_meshMap[meshHandle]->Draw(m_d3dDeviceContext.Get());
}

Material& RenderManager::GetMaterial(RHANDLE materialHandle)
{
	assert(materialHandle > 0 && materialHandle <= m_matID);

	return *m_materialMap[materialHandle];
}
