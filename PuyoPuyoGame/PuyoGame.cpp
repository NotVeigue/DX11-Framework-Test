#include "PuyoPuyoGamePCH.h"
#include "PuyoGame.h"
#include "PuyoValues.h"
#include "XMExtensions.h"

// Shader Includes
#include "SimpleVertexShader.h"
#include "UnlitPixelShader.h"
#include "DepthOnlyPS.h"
#include "DepthOnlyVS.h"
#include "SubsurfacePuyoPS.h"
#include "GridBackgroundPS.h"
#include "GridBackgroundVoronoiPS.h"


using namespace DirectX;
using namespace XMExtensions;

// DEBUG

//RHANDLE backgroundPS;

// Define array of puyo colors
DirectX::XMFLOAT4 PuyoGame::ms_PuyoColors[PUYO_COLOR_COUNT] = {
	XMFLOAT4(0.89f, 0.15f, 0.12f, 1.0f), // RED
	XMFLOAT4(0.02f, 0.67f, 0.15f, 1.0f), // GREEN
	XMFLOAT4(0.00f, 0.08f, 0.70f, 1.0f), // BLUE
	XMFLOAT4(0.95f, 0.42f, 0.04f, 1.0f), // YELLOW
	XMFLOAT4(0.31f, 0.06f, 0.60f, 1.0f), // PURPLE
	XMFLOAT4(1.00f, 1.00f, 1.00f, 0.0f)  // CLEAR
};

// Constant Buffer Data for the shaders
struct PerObjectConstantBufferData
{
	XMMATRIX WorldMatrix;
	XMMATRIX InverseTransposeWorldMatrix;
	XMMATRIX WorldViewProjectionMatrix;
};

struct SingleColorConstantBufferData
{
	XMFLOAT4 PixelColor;
};

struct DepthOnlyVSBufferData
{
	XMMATRIX WorldViewProjectionMatrix;
};

struct SubsurfacePuyoPSBufferData
{
	XMFLOAT4 Color;
	XMFLOAT3 LightPos;
	float Near;
	float Far;
	int padding[3]; // Gotta make sure this aligns to a 16 byte boundary
};

struct VoronoiConstantBufferData
{
	XMFLOAT3 Color;
	float Scale;
	float TileScale;
	int padding[3]; // Same here!
};

// ----------------------------------------------------------------------------------
// Singleton Stuff
// ----------------------------------------------------------------------------------
template<> PuyoGame* Singleton<PuyoGame>::msSingleton = 0;
PuyoGame& PuyoGame::GetSingleton()
{
	assert(msSingleton);
	return *msSingleton;
}

PuyoGame* PuyoGame::GetSingletonPtr()
{
	return msSingleton;
}

// ----------------------------------------------------------------------------------
// Implementation
// ----------------------------------------------------------------------------------

PuyoGame::PuyoGame()
	: m_p1Controller(KEY::A, KEY::D, KEY::W, KEY::S)
	, m_p2Controller(KEY::LEFT, KEY::RIGHT, KEY::UP, KEY::DOWN)
	, m_p1Instance(false)
	, m_p2Instance(true)
{
	// Debug
	m_p1Instance.transform.SetPosition(XMVectorSet(k_leftGridX, k_gridY, 0.0f, 1.0f));
	m_p2Instance.transform.SetPosition(XMVectorSet(k_rightGridX, k_gridY, 0.0f, 1.0f));

	LoadAssets();

	m_p1Instance.Initialize(&m_p1Controller);
	m_p2Instance.Initialize(&m_p2Controller);
}

PuyoGame::~PuyoGame()
{
	// Free all active puyos
	for (Puyo* p : m_activePuyoList)
	{
		m_puyoPool.FreeObject(p);
	}
	m_activePuyoList.clear();
}

void PuyoGame::LoadAssets()
{
	// Setup the camera
	//---------------------------------------------------------------
	m_orthoCamera.SetMode(CAMERA_MODE::Orthographic);
	m_orthoCamera.SetOrthographic(800.0f, 600.0f, 1.0f, 100.0f);
	m_orthoCamera.transform.SetPosition(XMVectorSet(0.0f, 0.0f, -15.0f, 1.0f));
	m_orthoCamera.transform.LookAt(XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f), Transform::WORLD_AXES::UP);

	// Load Basic Materials
	//---------------------------------------------------------------
	
	// Basic Puyo Material
	RHANDLE vs		= RenderManager::GetSingleton().CreateVShaderResource(g_SimpleVertexShader, sizeof(g_SimpleVertexShader), VertexPositionNormalTexture::InputElements, VertexPositionNormalTexture::InputElementCount);
	RHANDLE ps		= RenderManager::GetSingleton().CreatePShaderResource(g_UnlitPixelShader, sizeof(g_UnlitPixelShader));
	RHANDLE vscb	= RenderManager::GetSingleton().CreateCBResource(sizeof(PerObjectConstantBufferData));
	RHANDLE pscb	= RenderManager::GetSingleton().CreateCBResource(sizeof(SingleColorConstantBufferData));
	m_puyoMesh		= RenderManager::GetSingleton().CreateMeshResource(Mesh::CreateSphere(RenderManager::GetSingleton().GetDeviceContext(), 1.0f, 10, false));
	m_puyoMaterial	= RenderManager::GetSingleton().CreateMaterial(vs, ps, vscb, pscb);

	// Subsurface Material
	ps						= RenderManager::GetSingleton().CreatePShaderResource(g_SubsurfacePuyoPS, sizeof(g_SubsurfacePuyoPS));
	pscb					= RenderManager::GetSingleton().CreateCBResource(sizeof(SubsurfacePuyoPSBufferData));
	m_subsurfaceMaterial	= RenderManager::GetSingleton().CreateMaterial(vs, ps, vscb, pscb);

	// Depth Only Material
	vs					= RenderManager::GetSingleton().CreateVShaderResource(g_DepthOnlyVS, sizeof(g_DepthOnlyVS), VertexPositionNormalTexture::InputElements, VertexPositionNormalTexture::InputElementCount);
	ps					= RenderManager::GetSingleton().CreatePShaderResource(g_DepthOnlyPS, sizeof(g_DepthOnlyPS));
	vscb				= RenderManager::GetSingleton().CreateCBResource(sizeof(DepthOnlyVSBufferData));
	m_depthOnlyMaterial = RenderManager::GetSingleton().CreateMaterial(vs, ps, vscb);

	
	// ToDo: Load any textures/sprite fonts here
	//---------------------------------------------------------------
	InitGridStencil();
	m_backfaceDepth.Initialize(RenderManager::GetSingleton().GetDevice(), 800, 600, DXGI_FORMAT_D32_FLOAT, true, 1, 0);

	// DEBUG
	m_gridTexture.Initialize(RenderManager::GetSingleton().GetDevice(), 800, 600, DXGI_FORMAT_R8G8B8A8_UNORM);
	pscb                 = RenderManager::GetSingleton().CreateCBResource(sizeof(VoronoiConstantBufferData));
	RHANDLE backgroundPS = RenderManager::GetSingleton().CreatePShaderResource(g_GridBackgroundVoronoiPS, sizeof(g_GridBackgroundVoronoiPS));
	VoronoiConstantBufferData cbd;
	cbd.Color = XMFLOAT3(0.9, 0.5, 0.3) * 0.6f;
	cbd.Scale = 30.0f;
	cbd.TileScale = 6.5f;
	RenderManager::GetSingleton().UpdateConstantBuffer(pscb, &cbd);
	RenderManager::GetSingleton().SetPSConstantBuffer(pscb);
	RenderManager::GetSingleton().RenderFullscreen(backgroundPS, m_gridTexture.rtView);

	m_overlayTexture.Initialize(RenderManager::GetSingleton().GetDevice(), 800, 600, DXGI_FORMAT_R8G8B8A8_UNORM);
	cbd.Color = XMFLOAT3(0.9, 0.5, 0.3);
	cbd.Scale = 15.0f;
	cbd.TileScale = 9.0f;
	RenderManager::GetSingleton().UpdateConstantBuffer(pscb, &cbd);
	RenderManager::GetSingleton().RenderFullscreen(backgroundPS, m_overlayTexture.rtView);
}

void PuyoGame::InitGridStencil()
{
	ID3D11DeviceContext* context = RenderManager::GetSingleton().GetDeviceContext();

	m_gridStencil.Initialize(RenderManager::GetSingleton().GetDevice(), 800, 600, DXGI_FORMAT_D24_UNORM_S8_UINT, true, 1, 0);
	context->ClearDepthStencilView(m_gridStencil.dsView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	ID3D11RenderTargetView* nullRenderTarget[1] = { nullptr };
	context->OMSetRenderTargets(1, nullRenderTarget, m_gridStencil.dsView);
	
	RHANDLE quadMesh = RenderManager::GetSingleton().CreateMeshResource(Mesh::CreateQuad(context));
	Material& puyoMaterial = RenderManager::GetSingleton().GetMaterial(m_puyoMaterial);
	PerObjectConstantBufferData perObjectData;

	XMMATRIX leftPosition = XMMatrixTranslation(k_leftGridX + PUYO_SIZE * 2.5f, k_gridY + PUYO_SIZE * 5.5f, -10.0f);
	XMMATRIX rightPosition = XMMatrixTranslation(k_rightGridX + PUYO_SIZE * 2.5f, k_gridY + PUYO_SIZE * 5.5f, -10.0f);
	XMMATRIX gridScale = XMMatrixScaling(PUYO_SIZE * 6.0f, PUYO_SIZE * 12.0f, 1.0f);
	XMMATRIX queueScale = XMMatrixScaling(PUYO_SIZE * 4.0f, PUYO_SIZE * 2.0f, 1.0f);

	// Draw the overlay area
	RenderManager::GetSingleton().SetDepthStencilState(DEPTH_STENCIL_STATE::STENCIL_WRITE, 3U);
	perObjectData.WorldMatrix = XMMatrixScaling(800.0f, 600.0f, 0.0f);
	perObjectData.WorldViewProjectionMatrix = perObjectData.WorldMatrix * m_orthoCamera.GetViewMatrix() * m_orthoCamera.GetProjectionMatrix();
	RenderManager::GetSingleton().UpdateConstantBuffer(puyoMaterial.vsCBHandle, &perObjectData);
	RenderManager::GetSingleton().DrawWithMaterial(quadMesh, m_puyoMaterial);

	// Draw the two play areas
	RenderManager::GetSingleton().SetDepthStencilState(DEPTH_STENCIL_STATE::STENCIL_WRITE, 1U);
	perObjectData.WorldMatrix = gridScale * leftPosition;
	perObjectData.WorldViewProjectionMatrix = perObjectData.WorldMatrix * m_orthoCamera.GetViewMatrix() * m_orthoCamera.GetProjectionMatrix();
	RenderManager::GetSingleton().UpdateConstantBuffer(puyoMaterial.vsCBHandle, &perObjectData);
	RenderManager::GetSingleton().DrawWithMaterial(quadMesh, m_puyoMaterial);

	perObjectData.WorldMatrix = gridScale * rightPosition;
	perObjectData.WorldViewProjectionMatrix = perObjectData.WorldMatrix * m_orthoCamera.GetViewMatrix() * m_orthoCamera.GetProjectionMatrix();
	RenderManager::GetSingleton().UpdateConstantBuffer(puyoMaterial.vsCBHandle, &perObjectData);
	RenderManager::GetSingleton().DrawWithMaterial(quadMesh, m_puyoMaterial);

	// Draw the cutouts for the queues
	RenderManager::GetSingleton().SetDepthStencilState(DEPTH_STENCIL_STATE::STENCIL_WRITE, 2U);
	perObjectData.WorldMatrix = queueScale * XMMatrixTranslation(k_leftGridX + PUYO_SIZE * 8.0f, k_gridY + PUYO_SIZE * 13.5f, -10.0f);
	perObjectData.WorldViewProjectionMatrix = perObjectData.WorldMatrix * m_orthoCamera.GetViewMatrix() * m_orthoCamera.GetProjectionMatrix();
	RenderManager::GetSingleton().UpdateConstantBuffer(puyoMaterial.vsCBHandle, &perObjectData);
	RenderManager::GetSingleton().DrawWithMaterial(quadMesh, m_puyoMaterial);

	perObjectData.WorldMatrix = queueScale * XMMatrixTranslation(k_rightGridX - PUYO_SIZE * 3.0f, k_gridY + PUYO_SIZE * 13.5f, -10.0f);;
	perObjectData.WorldViewProjectionMatrix = perObjectData.WorldMatrix * m_orthoCamera.GetViewMatrix() * m_orthoCamera.GetProjectionMatrix();
	RenderManager::GetSingleton().UpdateConstantBuffer(puyoMaterial.vsCBHandle, &perObjectData);
	RenderManager::GetSingleton().DrawWithMaterial(quadMesh, m_puyoMaterial);

	RenderManager::GetSingleton().GetDeviceContext()->OMSetRenderTargets(1, RenderManager::GetSingleton().GetBackBufferRT(), m_gridStencil.dsView);
}


bool PuyoGame::Update(double dt)
{
	if(!m_p1Instance.Update(dt)) return false;
	if(!m_p2Instance.Update(dt)) return false;

	// Clear Depth and Render Targets
	ID3D11DeviceContext* context = RenderManager::GetSingleton().GetDeviceContext();
	context->ClearDepthStencilView(m_gridStencil.dsView, D3D11_CLEAR_DEPTH, 1.0f, 0U);
	context->ClearDepthStencilView(m_backfaceDepth.dsView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0U);
	context->ClearRenderTargetView(*RenderManager::GetSingleton().GetBackBufferRT(), DirectX::Colors::Black);
	
	// Render the grid texture
	/*RenderManager::GetSingleton().SetDepthStencilState(DEPTH_STENCIL_STATE::READONLY_STENCIL_EQ, 1U);
	RenderManager::GetSingleton().Blit(m_gridTexture.srView, nullptr, SAMPLER_STATE::LINEAR_WRAP, true);*/

	// Render the overlay texture
	RenderManager::GetSingleton().SetDepthStencilState(DEPTH_STENCIL_STATE::READONLY_STENCIL_EQ, 3U);
	RenderManager::GetSingleton().Blit(m_overlayTexture.srView, nullptr, SAMPLER_STATE::LINEAR_WRAP, true);

	// Render Puyo backface depth
	ID3D11RenderTargetView* nullRenderTarget[1] = { nullptr };
	context->OMSetRenderTargets(1, nullRenderTarget, m_backfaceDepth.dsView);
	RenderManager::GetSingleton().SetDepthStencilState(DEPTH_STENCIL_STATE::READ_WRITE);
	RenderManager::GetSingleton().SetRasterizerState(RASTERIZER_STATE::CULL_FRONT);
	RenderDepth();

	//RenderManager::GetSingleton().SetRasterizerState(RASTERIZER_STATE::CULL_BACK);
	//RenderManager::GetSingleton().Blit(m_backfaceDepth.srView, nullptr, SAMPLER_STATE::POINT_WRAP);
	
	//RenderManager::GetSingleton().Blit(m_gridStencil.srView, nullptr, SAMPLER_STATE::POINT_WRAP);

	// Render Puyo frontfaces using backface depth
	RenderManager::GetSingleton().SetRasterizerState(RASTERIZER_STATE::CULL_BACK);
	RenderManager::GetSingleton().SetBlendState(BLEND_STATE::ALPHA_BLEND);
	RenderManager::GetSingleton().GetDeviceContext()->OMSetRenderTargets(1, RenderManager::GetSingleton().GetBackBufferRT(), m_gridStencil.dsView);
	RenderManager::GetSingleton().SetDepthStencilState(DEPTH_STENCIL_STATE::STENCIL_GT, 3U);

	//RenderManager::GetSingleton().Clear(DirectX::Colors::AliceBlue, 1.0, 0);
	//RenderManager::GetSingleton().Blit(m_gridStencil.srView, nullptr, SAMPLER_STATE::POINT_WRAP);
	Render();

	RenderManager::GetSingleton().Present();

	return true;
}

/*
// Irrelevant because all puyos will be drawn in orthographic projection mode from head on,
// so there is no chance of overlap.
bool puyo_compare(const Puyo& a, const Puyo& b)	
{
	Camera& camera = PuyoGame::GetSingleton().GetCamera();

	float dist_a =	XMVectorGetX(
					XMVector3Length(
					XMVectorSubtract(a.transform.GetPosition(), camera.transform.GetPosition())));

	float dist_b =	XMVectorGetX(
					XMVector3Length(
					XMVectorSubtract(b.transform.GetPosition(), camera.transform.GetPosition())));

	return dist_a > dist_b;
}*/

bool puyo_compare(const Puyo* a, const Puyo* b)
{
	return a->puyoColor < b->puyoColor;
}

void PuyoGame::Render()
{
	if (m_puyoListDirty)
	{
		m_activePuyoList.sort(puyo_compare);
	}

	ID3D11DeviceContext* context = RenderManager::GetSingleton().GetDeviceContext();
	ID3D11ShaderResourceView* backfaceResource[1] = { m_backfaceDepth.srView };
	context->PSSetShaderResources(0U, 1U, backfaceResource);
	RenderManager::GetSingleton().SetSamplerState(SAMPLER_STATE::POINT_WRAP);

	Material& puyoMaterial = RenderManager::GetSingleton().GetMaterial(m_subsurfaceMaterial);
	PUYO_COLOR currentColor = PUYO_COLOR::NONE;
	PerObjectConstantBufferData perObjectData;
	SubsurfacePuyoPSBufferData ssData;
	ssData.LightPos = XMExtensions::normalize(XMFLOAT3(1.0f, 1.0f, -4.0f));
	ssData.Near = 1.0f;
	ssData.Far = 100.0f;

	for (Puyo* puyo : m_activePuyoList)
	{
		if (currentColor != puyo->puyoColor)
		{
			currentColor = puyo->puyoColor;
			ssData.Color = ms_PuyoColors[currentColor];
			RenderManager::GetSingleton().UpdateConstantBuffer(puyoMaterial.psCBHandle, &ssData);
		}

		XMMATRIX worldMatrix = puyo->transform.GetWorldMatrix();
		XMMATRIX viewMatrix = m_orthoCamera.GetViewMatrix();
		XMMATRIX projMatrix = m_orthoCamera.GetProjectionMatrix();
		perObjectData.WorldMatrix = worldMatrix;
		perObjectData.InverseTransposeWorldMatrix = XMMatrixTranspose(XMMatrixInverse(nullptr, worldMatrix));
		perObjectData.WorldViewProjectionMatrix = worldMatrix * viewMatrix * projMatrix;
		
		/*printf("--------------------------------------------------------");
		for (int i = 0; i < 16; i++)
		{
			printf("|%f|", perObjectData.InverseTransposeWorldMatrix.r[i / 4].m128_f32[i % 4]);
			if ((i + 1) % 4 != 0)
				continue;
			printf("\n");
		}
		printf("--------------------------------------------------------\n");*/

		RenderManager::GetSingleton().UpdateConstantBuffer(puyoMaterial.vsCBHandle, &perObjectData);

		RenderManager::GetSingleton().DrawWithMaterial(m_puyoMesh, m_subsurfaceMaterial);
	}
}

//void PuyoGame::Render()
//{
//	if (m_puyoListDirty)
//	{
//		m_activePuyoList.sort(puyo_compare);
//	}
//
//	Material& puyoMaterial = RenderManager::GetSingleton().GetMaterial(m_puyoMaterial);
//	PUYO_COLOR currentColor = PUYO_COLOR::NONE;
//	PerObjectConstantBufferData perObjectData;
//	SingleColorConstantBufferData colorData;
//
//	for (Puyo* puyo : m_activePuyoList)
//	{
//		if (currentColor != puyo->puyoColor)
//		{
//			currentColor = puyo->puyoColor;
//			colorData.PixelColor = ms_PuyoColors[currentColor];
//			RenderManager::GetSingleton().UpdateConstantBuffer(puyoMaterial.psCBHandle, &colorData);
//		}
//
//		XMMATRIX worldMatrix = puyo->transform.GetWorldMatrix();
//		XMMATRIX viewMatrix = m_orthoCamera.GetViewMatrix();
//		XMMATRIX projMatrix = m_orthoCamera.GetProjectionMatrix();
//		perObjectData.WorldMatrix = worldMatrix;
//		perObjectData.InverseTransposeWorldMatrix = XMMatrixTranspose(XMMatrixInverse(nullptr, worldMatrix));
//		perObjectData.WorldViewProjectionMatrix = worldMatrix * viewMatrix * projMatrix;
//		RenderManager::GetSingleton().UpdateConstantBuffer(puyoMaterial.vsCBHandle, &perObjectData);
//
//		RenderManager::GetSingleton().DrawWithMaterial(m_puyoMesh, m_puyoMaterial);
//	}
//}

void PuyoGame::RenderDepth()
{
	Material& depthOnlyMaterial = RenderManager::GetSingleton().GetMaterial(m_depthOnlyMaterial);
	DepthOnlyVSBufferData vsData;

	for (Puyo* puyo : m_activePuyoList)
	{
		XMMATRIX worldMatrix = puyo->transform.GetWorldMatrix();
		XMMATRIX viewMatrix = m_orthoCamera.GetViewMatrix();
		XMMATRIX projMatrix = m_orthoCamera.GetProjectionMatrix();
		vsData.WorldViewProjectionMatrix = worldMatrix * viewMatrix * projMatrix;
		RenderManager::GetSingleton().UpdateConstantBuffer(depthOnlyMaterial.vsCBHandle, &vsData);
		RenderManager::GetSingleton().DrawWithMaterial(m_puyoMesh, m_depthOnlyMaterial);
	}
}

Puyo* PuyoGame::AllocPuyo()
{
	// Get and initialize a new puyo object
	Puyo* newPuyo = m_puyoPool.AllocObject();
	newPuyo->SetRandomColor();
	m_activePuyoList.push_front(newPuyo);
	
	// Because we added a new puyo to the list, it needs to be re-sorted.
	m_puyoListDirty = true;

	return newPuyo;
}

void PuyoGame::FreePuyo(Puyo* puyo)
{
	m_activePuyoList.remove(puyo);
	m_puyoPool.FreeObject(puyo);
}

const PuyoInstance* PuyoGame::GetInstance(UINT8 playerNumber) const
{
	return playerNumber == 0U ? &m_p1Instance : &m_p2Instance;
}
