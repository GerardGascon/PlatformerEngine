#pragma once

#include <genesis.h>

#include "physics.h"

#define TILEMAP_PLANE BG_A

#define PLAYER_PALETTE PAL0
#define ENEMY_PALETTE PAL2
#define BOX_PALETTE PAL1
#define SMOKE_PALETTE PAL0
#define JUMP_PAD_PALETTE PAL0
#define MOVING_PLATFORM_PALETTE PAL1
#define MINE_CART_PALETTE PAL0
#define UI_PALETTE PAL3
#define CURRENCY_PALETTE PAL3

#define GROUND_TILE 1
#define LADDER_TILE 2
#define ONE_WAY_PLATFORM_TILE 4

extern const fix16 gravityScale;

struct {
	u16 joy;
	u16 changed;
	u16 state;
}input;

extern Map* bga;

extern AABB levelBounds;
extern AABB roomSize;
extern AABB playerBounds;

extern u16 VDPTilesFilled;