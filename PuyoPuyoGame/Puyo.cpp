#include "PuyoPuyoGamePCH.h"
#include "Puyo.h"
#include "PuyoValues.h"

Puyo::Puyo()
	: puyoColor(PUYO_COLOR::RED)
{
	//printf("Puyo Made!! \n");
}

Puyo::~Puyo()
{
	//printf("Puyo Destroyed!! \n");
}

void Puyo::SetRandomColor()
{
	puyoColor = static_cast<PUYO_COLOR>(rand() % USED_PUYO_COLORS);
}

void Puyo::SetChecked(bool value)
{
	checked = value;
}

bool Puyo::IsChecked() const
{
	return checked;
}
