#pragma once

#include <genesis.h>

#include "physics.h"
#include "types.h"
#include "camera.h"
#include "levelgenerator.h"

struct pBody {
	Sprite* sprite;
	AABB aabb;
	AABB climbingStairAABB;

	int facingDirection;
	int speed;
	fix16 acceleration;
	fix16 deceleration;
	int climbingSpeed;
	u16 maxFallSpeed;
	u16 jumpSpeed;

	u16 currentAnimation;

	bool onGround;
	bool onStair;
	bool jumping;
	bool collidingAgainstStair;
	bool climbingStair;

	bool runningAnim;

	Vect2D_s16 position;
	Vect2D_s16 globalPosition;
	Vect2D_u16 centerOffset;

	u16 skinWidth;

	struct {
		fix16 fixX;
		s16 x;
		fix16 y;
		s16 s8y;
	}velocity;

	Vect2D_s16 input;
};

extern struct pBody playerBody;

void playerInputChanged();
void playerInit();
void updatePlayer();

void setPlayerAnimation(u16 index);
//Returns true if last frame, useful to prevent looping
bool updatePlayerAnimations();