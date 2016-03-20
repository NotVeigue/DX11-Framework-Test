#pragma once
#include "DirectXIncludes.h"
#include "Puyo.h"
#include "PuyoValues.h"

class PuyoGrid
{
private:

	mutable Puyo* m_grid[GRID_WIDTH][GRID_HEIGHT] = {};
	void ResetCheckedPuyos() const;
	void CheckPuyo(int x, int y, PUYO_COLOR c, Puyo** comboStaging, int& comboSize) const;

public:
	PuyoGrid();
	~PuyoGrid();

	// ToDo: Add public api for adding and clearing puyo's
	//		 as well as querying the grid for combos and other
	//		 information.

	void AddPuyo(Puyo* puyo, int x, int y);
	Puyo* RemovePuyo(int x, int y);
	Puyo* GetPuyoAt(int x, int y) const;
	bool CheckOpenSpace(int x, int y) const;
	int FindCombos(Puyo** comboStaging) const;
};

