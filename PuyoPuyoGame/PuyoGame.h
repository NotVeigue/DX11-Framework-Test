#pragma once
#include "Singleton.h"
#include "RenderManager.h"
#include "ObjectPool.h"
#include "PuyoInstance.h"
#include "PlayerController.h"
#include "Puyo.h"
#include "BufferUtils.h"
#include <forward_list>


class PuyoGame : Singleton<PuyoGame>
{
private:

	// Resources for Drawing The Game
	static DirectX::XMFLOAT4 ms_PuyoColors[PUYO_COLOR_COUNT];
	Camera m_orthoCamera;
	RHANDLE m_puyoMesh;

	// Materials
	RHANDLE m_puyoMaterial;
	RHANDLE m_depthOnlyMaterial;
	RHANDLE m_subsurfaceMaterial;

	// Puyo Management
	ObjectPool<Puyo> m_puyoPool{ 256 };
	std::forward_list<Puyo*> m_activePuyoList;
	bool m_puyoListDirty; // If true, puyo list has changed and puyos must be sorted before being drawn

	// Player Instances
	PuyoInstance m_p1Instance;
	PuyoInstance m_p2Instance;

	// Controller Interfaces
	PlayerController m_p1Controller;
	PlayerController m_p2Controller;

	void LoadAssets();
	void Render();
	void RenderDepth();

	// We're gonna use a stencil for this just because we can!!
	DepthStencilBuffer m_gridStencil;
	void InitGridStencil();

	// This is used for the effect used to draw the puyos
	DepthStencilBuffer m_backfaceDepth;

	// Procedural voronoi texture used in the background of the puyo grids
	RenderTarget2D m_gridTexture;

	// Procedural texture around the UI elements
	RenderTarget2D m_overlayTexture;

public:
	PuyoGame();
	~PuyoGame();

	bool Update(double dt);

	// Obtains a puyo from the object pool, adds it to the active list, then returns it
	Puyo* AllocPuyo();

	// Removes a puyo from the active list, then returns its memory to the object pool
	void FreePuyo(Puyo*);

	const PuyoInstance* GetInstance(UINT8 playerNumber) const;

	static PuyoGame& GetSingleton();
	static PuyoGame* GetSingletonPtr();
};

