#pragma once

// get current position, orientation, and colors of puyo unit
int GetCurrentUnit(lua_State* L);

// get puyo color at position x, y
int GetPuyoAt(lua_State* L);

// IDEA: Add functions for querying the other player's field or manipulating your queue to be exactly what you want it to be to enable literal cheating AI's.

// IDEA: Break the LUA AI stuff into multiple scripts! Make one all the common boilerplate stuff (moving the unit, utility functions to locate obstructions, etc.)
//		 then make separate files for each AI's particular strategy

