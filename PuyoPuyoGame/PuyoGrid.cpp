#include "PuyoPuyoGamePCH.h"
#include "PuyoGrid.h"

using namespace DirectX;

PuyoGrid::PuyoGrid()
{
}

PuyoGrid::~PuyoGrid()
{
}

void PuyoGrid::Cleanup()
{
	for (int i = 0; i < GRID_WIDTH; i++)
	{
		for (int j = 0; j < GRID_HEIGHT; j++)
		{
			m_grid[i][j] = nullptr;
		}
	}
}

// ***************************************************************
// PRIVATE FUNCTIONS
// ***************************************************************

void PuyoGrid::ResetCheckedPuyos() const
{
	for (int i = 0; i < GRID_WIDTH; i++)
	{
		for (int j = 0; j < GRID_HEIGHT; j++)
		{
			if (!m_grid[i][j])
				continue;

			m_grid[i][j]->SetChecked(false);
		}
	}
}

void PuyoGrid::CheckPuyo(int x, int y, PUYO_COLOR c, Puyo** comboStaging, int& comboSize) const
{
	m_grid[x][y]->SetChecked(true);
	*(comboStaging + comboSize) = m_grid[x][y];
	comboSize++;

	Puyo* p;
	if ((p = GetPuyoAt(x + 1, y)) && p->puyoColor == c && !p->IsChecked()) CheckPuyo(x + 1, y, c, comboStaging, comboSize);
	if ((p = GetPuyoAt(x - 1, y)) && p->puyoColor == c && !p->IsChecked()) CheckPuyo(x - 1, y, c, comboStaging, comboSize);
	if ((p = GetPuyoAt(x, y + 1)) && p->puyoColor == c && !p->IsChecked()) CheckPuyo(x, y + 1, c, comboStaging, comboSize);
	if ((p = GetPuyoAt(x, y - 1)) && p->puyoColor == c && !p->IsChecked()) CheckPuyo(x, y - 1, c, comboStaging, comboSize);
}

// ***************************************************************
// PUBLIC FUNCTIONS
// ***************************************************************

void PuyoGrid::AddPuyo(Puyo* puyo, int x, int y)
{
	printf("Puyo was at position: %d, %d \n", x, y);
	assert(x < GRID_WIDTH);
	assert(y < GRID_HEIGHT);
	assert(!m_grid[x][y]);

	puyo->transform.SetPosition(XMVectorSet(x, y, 0.0f, 1.0f));
	m_grid[x][y] = puyo;
}

Puyo* PuyoGrid::RemovePuyo(int x, int y)
{
	assert(x < GRID_WIDTH);
	assert(y < GRID_HEIGHT);
	
	Puyo* puyo = m_grid[x][y];
	m_grid[x][y] = nullptr;

	return puyo;
}

Puyo* PuyoGrid::GetPuyoAt(int x, int y) const
{
	if (x < 0 || x >= GRID_WIDTH) return nullptr;
	if (y < 0 || y >= GRID_HEIGHT) return nullptr;

	return m_grid[x][y];
}

bool PuyoGrid::CheckOpenSpace(int x, int y) const
{
	if (x < 0 || x >= GRID_WIDTH) return false;
	if (y < 0 || y >= GRID_HEIGHT) return false;

	return m_grid[x][y] == nullptr;
}

int PuyoGrid::FindCombos(Puyo** comboStaging) const
{
	ResetCheckedPuyos();

	Puyo* p = nullptr;
	int comboSize = 0;
	int totalCombo = 0;
	for (int i = 0; i < GRID_WIDTH; i++)
	{
		for (int j = 0; j < GRID_HEIGHT; j++)
		{
			if ((p = m_grid[i][j]) == nullptr || p->IsChecked())
				continue;

			comboSize = 0;
			CheckPuyo(i, j, p->puyoColor, comboStaging, comboSize);

			if (comboSize >= MIN_COMBO_SIZE)
			{
				comboStaging += comboSize;
				totalCombo += comboSize;
			}
		}	
	}

	return totalCombo;
}
