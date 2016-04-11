#pragma once
#include "Singleton.h"
#include "RenderManager.h"
#include "ObjectPool.h"
#include "PuyoInstance.h"
#include "PuyoKeyboardController.h"
#include "Puyo.h"
#include "BufferUtils.h"
#include <forward_list>


class PuyoGame : Singleton<PuyoGame>
{
private:

	// Resources for Drawing The Game
	static DirectX::XMFLOAT4 ms_PuyoColors[PUYO_COLOR_COUNT];
	RHANDLE m_puyoMesh;
	RHANDLE m_puyoMaterial;
	Camera m_orthoCamera;

	// Puyo Management
	ObjectPool<Puyo> m_puyoPool{ 256 };
	std::forward_list<Puyo*> m_activePuyoList;
	bool m_puyoListDirty; // If true, puyo list has changed and puyos must be sorted before being drawn

	// Player Instances
	PuyoInstance m_p1Instance;
	PuyoInstance m_p2Instance;

	// Controller Interfaces
	PuyoKeyboardController m_p1Controller;
	PuyoKeyboardController m_p2Controller;

	void LoadAssets();
	void Render();

	// We're gonna use a depth stencil for this just because we can!!
	DepthStencilBuffer m_gridStencil;
	void InitDepthStencil();

public:
	PuyoGame();
	~PuyoGame();

	bool Update(double dt);

	// Obtains a puyo from the object pool, adds it to the active list, then returns it
	Puyo* AllocPuyo();

	// Removes a puyo from the active list, then returns its memory to the object pool
	void FreePuyo(Puyo*);

	static PuyoGame& GetSingleton();
	static PuyoGame* GetSingletonPtr();
};

