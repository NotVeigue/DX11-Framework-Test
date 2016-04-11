#pragma once
#include "Singleton.h"
#include "DirectXIncludes.h"
#include "Mesh.h"
#include "Transform.h"
#include "Camera.h"
#include <unordered_map>

// Enums for easy reference while creating/setting preconfigured states for 
// various stages of the rendering pipeline.
enum RASTERIZER_STATE
{
	CULL_FRONT,
	CULL_BACK,
	CULL_NONE,
	WIREFRAME,
	RASTERIZER_STATE_COUNT
};

enum DEPTH_STENCIL_STATE
{
	READ_WRITE,
	READ_ONLY,
	WRITE_ONLY,
	DEPTH_NONE,
	STENCIL_WRITE,
	STENCIL_GT,
	STENCIL_LT,
	DEPTH_STENCIL_STATE_COUNT
};

enum BLEND_STATE
{
	SOLID,
	ALPHA_BLEND,
	PREMULTIPLIED_ALPHA,
	ADDITIVE_BLEND,
	NO_COLOR,
	BLEND_STATE_COUNT
};

enum SAMPLER_STATE
{
	POINT_WRAP,
	POINT_CLAMP,
	LINEAR_WRAP,
	LINEAR_CLAMP,
	ANISOTROPIC_WRAP,
	ANISOTROPIC_CLAMP,
	SAMPLER_STATE_COUNT
};

// For keeping track of references to created resources.
// The typedef just helps me clarify how functions and
// and values are meant to be used.
typedef unsigned short RHANDLE;

// A simple struct to tie together a vertex shader and pixel shader meant to be used together.
// Also includes space for constant buffers if any are needed.
struct Material
{
	RHANDLE vsHandle;
	RHANDLE psHandle;
	RHANDLE vsCBHandle;
	RHANDLE psCBHandle;

	Material()
		: vsHandle(0)
		, psHandle(0)
		, vsCBHandle(0)
		, psCBHandle(0)
	{}

	Material(RHANDLE vs, RHANDLE ps, RHANDLE vcb = 0, RHANDLE pcb = 0)
		: vsHandle(vs)
		, psHandle(ps)
		, vsCBHandle(vcb)
		, psCBHandle(pcb)
	{}
};
 
class RenderManager : public Singleton<RenderManager>
{
private:

	// The current viewport we are using in the rasterizer stage.
	D3D11_VIEWPORT m_viewport;

	// Incrementing unique ID values for each type of resource tracked by the RenderManager.
	RHANDLE m_meshID, m_vsID, m_psID, m_cbID, m_matID;

	// Tracking variables for settable resources. These are set whenever a resources is used
	// and are polled whenever a request to set another resource is received to see if we are
	// aready using it and, therefore, do no need to do any switching.
	RHANDLE m_curMesh, m_curVS, m_curPS, m_curMat;

	// Maps associating generated resources with their respective resource handles.
	std::unordered_map<RHANDLE, std::unique_ptr<Mesh>> m_meshMap;
	std::unordered_map<RHANDLE, Microsoft::WRL::ComPtr<ID3D11VertexShader>> m_vShaderMap;
	std::unordered_map<RHANDLE, Microsoft::WRL::ComPtr<ID3D11PixelShader>> m_pShaderMap;
	std::unordered_map<RHANDLE, Microsoft::WRL::ComPtr<ID3D11Buffer>> m_cBufferMap;
	std::unordered_map<RHANDLE, std::unique_ptr<Material>> m_materialMap;
	std::unordered_map<RHANDLE, Microsoft::WRL::ComPtr<ID3D11InputLayout>> m_vsInputLayoutMap; // RHANDLE is same as corresponding vShader

	// Direct3D device and swap chain.
	Microsoft::WRL::ComPtr<ID3D11Device> m_d3dDevice;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> m_d3dDeviceContext;
	Microsoft::WRL::ComPtr<IDXGISwapChain1> m_d3dSwapChain;

	// ToDo: Allow users to create and set RenderTargetViews, DepthStencilViews, and DepthStencilBuffers

	// Render target view for the back buffer of the swap chain.
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_d3dRenderTargetView;
	// Depth/stencil view for use as a depth buffer.
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> m_d3dDepthStencilView;
	// A texture to associate to the depth stencil view.
	Microsoft::WRL::ComPtr<ID3D11Texture2D> m_d3dDepthStencilBuffer;

	// Arrays of some simple pre-configured state objects for various stages of the rendering pipeline
	Microsoft::WRL::ComPtr<ID3D11RasterizerState> m_rasterizerStates[RASTERIZER_STATE_COUNT];
	Microsoft::WRL::ComPtr<ID3D11DepthStencilState> m_depthStencilStates[DEPTH_STENCIL_STATE_COUNT];
	Microsoft::WRL::ComPtr<ID3D11BlendState> m_blendStates[BLEND_STATE_COUNT];
	Microsoft::WRL::ComPtr<ID3D11SamplerState> m_samplerStates[SAMPLER_STATE_COUNT];

	// Present parameters used by the IDXGISwapChain1::Present1 method
	DXGI_PRESENT_PARAMETERS m_PresentParameters;

	// A material and geometry from blitting to the screen.
	// I imagine compute shaders may be able to be used to accomplish this more efficiently? I guess I will 
	RHANDLE m_blitMaterial;
	RHANDLE m_blitQuad;

	bool Initialize();
	bool InitStates();
	bool ResizeSwapChain(int width, int height);

	// Helper functions for creating pre-configured rendering pipeline states
	bool CreateRasterizerState(D3D11_CULL_MODE cullMode, D3D11_FILL_MODE fillMode, ID3D11RasterizerState** pResult);
	bool CreateDepthStencilState(bool enable, bool writeEnable, bool stencilEnable, bool stencilWriteEnable, D3D11_COMPARISON_FUNC stencilFunc, ID3D11DepthStencilState** pResult);
	bool CreateBlendState(D3D11_BLEND srcBlend, D3D11_BLEND destBlend, bool colorWriteEnable, ID3D11BlendState** pResult);
	bool CreateSamplerState(D3D11_FILTER filter, D3D11_TEXTURE_ADDRESS_MODE addressMode, ID3D11SamplerState** pResult);

public:
	RenderManager();
	~RenderManager();

	ID3D11Device* GetDevice() const;
	ID3D11DeviceContext* GetDeviceContext() const;
	ID3D11RenderTargetView** GetBackBufferRT() const;
	ID3D11DepthStencilView** GetDepthStencil() const;

	// Binds the default back buffer render target view and depth stencil view
	void ResetRenderTarget();

	// Resource creation functions
	RHANDLE CreateMeshResource(std::unique_ptr<Mesh> meshPtr);
	//RHANDLE CreateVShaderResource(const void* shaderBytecode, size_t bytecodeSize);
	RHANDLE CreateVShaderResource(const void* shaderBytecode, size_t bytecodeSize, const D3D11_INPUT_ELEMENT_DESC* elementDesc, UINT elementCount);
	// ToDo: Figure out a good way to tie sampler states and texture resources to pixel shaders.
	// This can probably be accomplished through separate function calls that set these values for now?
	RHANDLE CreatePShaderResource(const void* shaderBytecode, size_t bytecodeSize);
	RHANDLE CreateCBResource(size_t bufferSize);
	RHANDLE CreateMaterial(RHANDLE vShader, RHANDLE pShader, RHANDLE vCBuffer = 0, RHANDLE pCBuffer = 0);

	// Resource management functions
	void SetMaterial(RHANDLE materialHandle);
	void SetVertexShader(RHANDLE shaderHandle);
	void SetPixelShader(RHANDLE shaderHandle);
	void SetVSConstantBuffer(RHANDLE cbHandle);
	void SetPSConstantBuffer(RHANDLE cbHandle);
	void UpdateConstantBuffer(RHANDLE cbHandle, const void* cbData);
	void DrawWithMaterial(RHANDLE meshHandle, RHANDLE materialHandle);
	Material& GetMaterial(RHANDLE materialHandle);

	void SetRasterizerState(RASTERIZER_STATE state);
	void SetDepthStencilState(DEPTH_STENCIL_STATE state);
	void SetBlendState(BLEND_STATE state);

	// Blits the given source texture to the specified destination texture. If the destination is null, the contents of src will be drawn to 
	// the back buffer.
	void Blit(ID3D11ShaderResourceView* src, ID3D11RenderTargetView* dst, SAMPLER_STATE samplerState = SAMPLER_STATE::POINT_CLAMP);

	// Set the current viewport used by the rasterizer stage. Only really needs to be called when
	// resizing the window or doing something special like rendering split-screen. I will expand
	// this functionality if I ever have need.
	void SetViewport(float width, float height, float topLeftX = 0.0f, float topLeftY = 0.0f, float minDepth = 0.0f, float maxDepth = 1.0f);

	// Clear the contents of the back buffer, depth buffer, and stencil buffer.
	// This function is usually called before anything is rendered to the screen.
	void Clear(const FLOAT clearColor[4], FLOAT clearDepth, UINT8 clearStencil);

	// Swap the contents of the back buffer to the front.
	// This function is usually called after everything has been rendered.
	void Present();

	static RenderManager& GetSingleton(void);
	static RenderManager* GetSingletonPtr(void);
};

