#pragma once
#include "Transform.h"

// TODO: Create some sort of static class that holds globals like this!
#define PUYO_SIZE 20
#define USED_PUYO_COLORS 4

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