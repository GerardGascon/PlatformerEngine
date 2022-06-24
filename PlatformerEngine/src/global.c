#include "../inc/global.h"

const fix16 gravityScale = FIX16(0.5);

Map* bga;

AABB roomSize;
AABB playerBounds;

u16 VDPTilesFilled = TILE_USER_INDEX;