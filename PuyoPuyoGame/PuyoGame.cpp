#include "PuyoPuyoGamePCH.h"
#include "PuyoGame.h"
#include "SimpleVertexShader.h"
#include "UnlitPixelShader.h"

using namespace DirectX;

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
// Helper Functions
// ----------------------------------------------------------------------------------

PuyoGame::PuyoGame()
{
	LoadAssets();

	// Debug
	m_p1Instance.transform.SetPosition(XMVectorSet(-350.0f, -100.0f, (float)PUYO_SIZE, 1.0f));
	m_p2Instance.transform.SetPosition(XMVectorSet(200.0f, -100.0f, (float)PUYO_SIZE, 1.0f));
}

PuyoGame::~PuyoGame()
{
	// Debug!!
	for (Puyo* p : m_activePuyoList)
	{
		m_puyoPool.FreeObject(p);
	}
	m_activePuyoList.clear();
}

void PuyoGame::LoadAssets()
{
	// Setup the camera
	m_orthoCamera.SetMode(CAMERA_MODE::Orthographic);
	m_orthoCamera.SetOrthographic(800.0f, 600.0f, 0.1f, 100.0f);
	m_orthoCamera.transform.SetPosition(XMVectorSet(0.0f, 0.0f, -5.0f, 1.0f));
	m_orthoCamera.transform.LookAt(XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f), Transform::WORLD_AXES::UP);

	// Load Basic Materials
	RHANDLE vs		= RenderManager::GetSingleton().CreateVShaderResource(g_SimpleVertexShader, sizeof(g_SimpleVertexShader), VertexPositionNormalTexture::InputElements, VertexPositionNormalTexture::InputElementCount);
	RHANDLE ps		= RenderManager::GetSingleton().CreatePShaderResource(g_UnlitPixelShader, sizeof(g_UnlitPixelShader));
	RHANDLE vscb	= RenderManager::GetSingleton().CreateCBResource(sizeof(PerObjectConstantBufferData));
	RHANDLE pscb	= RenderManager::GetSingleton().CreateCBResource(sizeof(SingleColorConstantBufferData));
	m_puyoMesh		= RenderManager::GetSingleton().CreateMeshResource(Mesh::CreateSphere(RenderManager::GetSingleton().GetDeviceContext(), 1.0f, 10, false));
	m_puyoMaterial	= RenderManager::GetSingleton().CreateMaterial(vs, ps, vscb, pscb);
	
	// ToDo: Load any textures/sprite fonts here
}


bool PuyoGame::Update(double dt)
{
	bool result = m_p1Instance.Update(dt);
	
	Render();

	return result;
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
	RenderManager::GetSingleton().Clear(DirectX::Colors::Black, 1.0, 0);

	if (m_puyoListDirty)
	{
		m_activePuyoList.sort(puyo_compare);
	}

	Material& puyoMaterial = RenderManager::GetSingleton().GetMaterial(m_puyoMaterial);
	PUYO_COLOR currentColor = PUYO_COLOR::NONE;
	PerObjectConstantBufferData perObjectData;
	SingleColorConstantBufferData colorData;

	for (Puyo* puyo : m_activePuyoList)
	{
		if (currentColor != puyo->puyoColor)
		{
			currentColor = puyo->puyoColor;
			colorData.PixelColor = ms_PuyoColors[currentColor];
			RenderManager::GetSingleton().UpdateConstantBuffer(puyoMaterial.psCBHandle, &colorData);
		}

		XMMATRIX worldMatrix = puyo->transform.GetWorldMatrix();
		XMMATRIX viewMatrix = m_orthoCamera.GetViewMatrix();
		XMMATRIX projMatrix = m_orthoCamera.GetProjectionMatrix();
		perObjectData.WorldMatrix = worldMatrix;
		perObjectData.InverseTransposeWorldMatrix = XMMatrixTranspose(XMMatrixInverse(nullptr, worldMatrix));
		perObjectData.WorldViewProjectionMatrix = worldMatrix * viewMatrix * projMatrix;
		RenderManager::GetSingleton().UpdateConstantBuffer(puyoMaterial.vsCBHandle, &perObjectData);
		
		RenderManager::GetSingleton().DrawWithMaterial(m_puyoMesh, m_puyoMaterial);
	}

	RenderManager::GetSingleton().Present();
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


