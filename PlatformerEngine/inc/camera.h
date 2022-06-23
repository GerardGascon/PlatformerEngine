#pragma once

#include <genesis.h>

#include "types.h"
#include "player.h"
#include "physics.h"
#include "levels.h"

struct cam {
	Vect2D_s16 position;

	Vect2D_u16 deadZoneCenter;
	struct {
		u16 width;
		u16 height;
	}deadZoneSize;
};

extern struct cam camera;

void setupCamera(Vect2D_u16 deadZoneCenter, u16 deadZoneWidth, u16 deadZoneHeight);
void updateCamera();

void setHorizontalPlayerPos(s16 direction);