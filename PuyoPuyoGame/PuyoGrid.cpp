#include "PuyoPuyoGamePCH.h"
#include "PuyoGrid.h"

using namespace DirectX;

const int PuyoGrid::kGridWidth = 6;
const int PuyoGrid::kGridHeight = 13;

PuyoGrid::PuyoGrid()
{
	for (int i = 0; i < kGridWidth; i++)
		for (int j = 0; j < kGridHeight; j++)
			m_grid[i][j] = nullptr;
}

PuyoGrid::~PuyoGrid()
{
}

void PuyoGrid::AddPuyo(Puyo* puyo, int x, int y)
{
	printf("Puyo was at position: %d, %d \n", x, y);
	assert(x < kGridWidth);
	assert(y < kGridHeight);
	assert(!m_grid[x][y]);

	puyo->transform.SetPosition(XMVectorSet(x, y, 0.0f, 1.0f));
	m_grid[x][y] = puyo;
}

Puyo* PuyoGrid::RemovePuyo(int x, int y)
{
	assert(x < kGridWidth);
	assert(y < kGridHeight);
	
	Puyo* puyo = m_grid[x][y];
	m_grid[x][y] = nullptr;

	return puyo;
}

Puyo* PuyoGrid::GetPuyoAt(int x, int y) const
{
	if (x < 0 || x > kGridWidth) return nullptr;
	if (y < 0 || y > kGridHeight) return nullptr;

	return m_grid[x][y];
}

bool PuyoGrid::CheckOpenSpace(int x, int y) const
{
	if (x < 0 || x > kGridWidth) return false;
	if (y < 0 || y > kGridHeight) return false;

	return m_grid[x][y] == nullptr;
}

XMFLOAT2 PuyoGrid::PositionToCoordinates(float x, float y)
{
	return XMFLOAT2(round(x), round(y));
}
