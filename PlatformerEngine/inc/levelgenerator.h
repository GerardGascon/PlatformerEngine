#pragma once

#include <genesis.h>

#include "player.h"
#include "global.h"


u8 getTileValue(s16 x, s16 y);

void freeCollisionMap();
void generateCollisionMap(const u8 map[][48]);