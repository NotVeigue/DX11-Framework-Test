#pragma once
#include "Transform.h"

enum PUYO_COLOR
{
	RED,
	GREEN,
	BLUE,
	YELLOW,
	PURPLE,
	CLEAR,
	PUYO_COLOR_COUNT,
	NONE
};

class Puyo
{
public:
	Puyo();
	~Puyo();

	PUYO_COLOR puyoColor;
	Transform transform;

	void SetRandomColor();
};