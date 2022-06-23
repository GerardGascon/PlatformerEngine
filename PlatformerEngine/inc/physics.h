#pragma once

#include <genesis.h>

#include "types.h"

typedef struct {
	Vect2D_s16 min;
	Vect2D_s16 max;
}AABB;

bool overlapX(const AABB object, const s16 positionX);
bool overlapY(const AABB object, const u16 positionY);
bool overlap(const AABB* object1, const AABB* object2);
AABB newAABB(s16 x1, s16 x2, s16 y1, s16 y2);
u16 getTileLeftEdge(u8 x);
u16 getTileRightEdge(u8 x);
u16 getTileTopEdge(u8 y);
u16 getTileBottomEdge(u8 y);
AABB getTileBounds(int x, int y);
Vect2D_u8 posToTile(Vect2D_s16 position);