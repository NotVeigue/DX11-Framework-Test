#pragma once
#include "DirectXIncludes.h"
#include "Puyo.h"

class PuyoGrid
{
private:

	Puyo* m_grid[6][13];

public:

	static const int kGridWidth;
	static const int kGridHeight;

	PuyoGrid();
	~PuyoGrid();

	// ToDo: Add public api for adding and clearing puyo's
	//		 as well as querying the grid for combos and other
	//		 information.

	void AddPuyo(Puyo* puyo, int x, int y);
	Puyo* RemovePuyo(int x, int y);
	Puyo* GetPuyoAt(int x, int y) const;
	bool CheckOpenSpace(int x, int y) const;
	DirectX::XMFLOAT2 PositionToCoordinates(float x, float y);
	// AddPuyo
	// RemovePuyo
	// CoordinatesToPosition
	// PositionToCoordinates
};

