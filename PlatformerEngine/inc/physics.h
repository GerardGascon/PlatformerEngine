#pragma once

#include <genesis.h>

#include "types.h"

u16 getTileLeftEdge(u8 x);
u16 getTileRightEdge(u8 x);
u16 getTileTopEdge(u8 y);
u16 getTileBottomEdge(u8 y);
AABB getTileBounds(int x, int y);
Vect2D_u8 posToTile(Vect2D_s16 position);