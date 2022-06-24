#include "../inc/player.h"

#include "../inc/global.h"
#include "../inc/map.h"

#include "../res/resources.h"

struct pBody playerBody;

void checkCollisions();
void updateAnimations();

const s16 coyoteTime = 10;
s16 currentCoyoteTime;
const s16 jumpBufferTime = 10;
s16 currentJumpBufferTime;

bool climbStairPressed;

bool falling;
u16 dyingSteps;
const u16 dieDelay = 10;

s16 stairLeftEdge;
const u16 stairPositionOffset = 4;

void playerInit() {
	//Create the sprite and palette for the player
	playerBody.sprite = SPR_addSprite(&player_sprite, levelStartPos.x, levelStartPos.y, TILE_ATTR(PLAYER_PALETTE, FALSE, FALSE, FALSE));
	PAL_setPalette(PLAYER_PALETTE, player_sprite.palette->data, DMA);

	//Set the global position of the player, the local position will be updated once we are in the main loop
	playerBody.globalPosition = levelStartPos;

	//We set collider size of the player
	playerBody.aabb = newAABB(4, 20, 4, 24);
	//This collider is thinner because if the width is >=16 it could interfere with the lateral walls
	playerBody.climbingStairAABB = newAABB(8, 20, playerBody.aabb.min.y, playerBody.aabb.max.y);
	//Skin width is used to avoid some wrong physics calculations because of the fall velocity
	playerBody.skinWidth = 7;

	//Calculate where's the center point of the player
	playerBody.centerOffset.x = ((playerBody.aabb.min.x + playerBody.aabb.max.x) >> 1);
	playerBody.centerOffset.y = ((playerBody.aabb.min.y + playerBody.aabb.max.y) >> 1);

	//Default movement values
	playerBody.speed = 2;
	playerBody.climbingSpeed = 1;
	playerBody.maxFallSpeed = 6;
	playerBody.jumpSpeed = 7;
	playerBody.facingDirection = 1;
	playerBody.acceleration = FIX16(.25);
	playerBody.deceleration = FIX16(.2);
}

void playerInputChanged() {
	u16 joy = input.joy;
	u16 state = input.state;
	u16 changed = input.changed;

	//We only read data from the joypad 1
	if (joy == JOY_1) {
		//Update x velocity
		if (state & BUTTON_RIGHT) {
			playerBody.input.x = 1;
		}else if (state & BUTTON_LEFT) {
			playerBody.input.x = -1;
		}else if ((changed & BUTTON_RIGHT) | (changed & BUTTON_LEFT)) {
			playerBody.input.x = 0;
		}

		//Jump button via jumpbuffer
		//Also used to stop climbing the stairs
		if (changed & (BUTTON_A | BUTTON_B | BUTTON_C)) {
			if (state & (BUTTON_A | BUTTON_B | BUTTON_C)) {
				if (playerBody.climbingStair) {
					playerBody.climbingStair = FALSE;
					climbStairPressed = FALSE;
				}else {
					currentJumpBufferTime = jumpBufferTime;
				}
			}else if (playerBody.jumping && playerBody.velocity.y < 0) {
				//If the button is released we remove half of the velocity
				playerBody.velocity.y = fix16Mul(playerBody.velocity.y, FIX16(.5));
			}
		}

		//Down button is only used when it is climbing stair
		if (changed & BUTTON_DOWN) {
			if (state & BUTTON_DOWN) {
				playerBody.input.y = 1;
				if (playerBody.climbingStair) {
					playerBody.velocity.y = FIX16(playerBody.climbingSpeed);
				}else if (playerBody.onStair) {
					playerBody.velocity.y = FIX16(playerBody.climbingSpeed);
					playerBody.climbingStair = TRUE;
				}
			}else if (playerBody.climbingStair) {
				playerBody.velocity.y = 0;
			}
		}
		if (changed & BUTTON_UP) {
			if (state & BUTTON_UP) {
				playerBody.input.y = -1;
				if (playerBody.collidingAgainstStair && !playerBody.onStair) {
					playerBody.climbingStair = TRUE;
					playerBody.velocity.y = FIX16(-playerBody.climbingSpeed);
				}else {
					climbStairPressed = TRUE;
				}
			}else {
				playerBody.input.y = 0;
				climbStairPressed = FALSE;
				if (playerBody.climbingStair) {
					playerBody.velocity.y = 0;
				}
			}
		}
	}
}

void updatePlayer() {
	if (climbStairPressed && playerBody.collidingAgainstStair && !playerBody.onStair) {
		playerBody.climbingStair = TRUE;
		playerBody.velocity.y = FIX16(-playerBody.climbingSpeed);
		climbStairPressed = FALSE;
	}

	if (currentCoyoteTime > 0 && currentJumpBufferTime > 0) {
		playerBody.jumping = TRUE;
		playerBody.velocity.y = FIX16(-playerBody.jumpSpeed);

		currentCoyoteTime = 0;
		currentJumpBufferTime = 0;
	}
	currentJumpBufferTime--;

	if (playerBody.climbingStair) {
		playerBody.velocity.x = playerBody.velocity.fixX = 0;
	}else {
		if (playerBody.input.x > 0) {
			if (playerBody.velocity.x != playerBody.speed)
				playerBody.velocity.fixX += playerBody.acceleration;
		}else if (playerBody.input.x < 0) {
			if (playerBody.velocity.x != -playerBody.speed)
				playerBody.velocity.fixX -= playerBody.acceleration;
		}else if (playerBody.onGround) {
			if (playerBody.velocity.x > 0)
				playerBody.velocity.fixX -= playerBody.deceleration;
			else if (playerBody.velocity.x < 0)
				playerBody.velocity.fixX += playerBody.deceleration;
			else
				playerBody.velocity.fixX = 0;
		}
		playerBody.velocity.x = fix16ToInt(playerBody.velocity.fixX);
		playerBody.velocity.x = clamp(playerBody.velocity.x, -playerBody.speed, playerBody.speed);
	}

	if (playerBody.climbingStair) {
		playerBody.globalPosition.x = stairLeftEdge - stairPositionOffset;
	}

	if (playerBody.onGround == FALSE && playerBody.climbingStair == FALSE) {
		if (fix16ToInt(playerBody.velocity.y) <= playerBody.maxFallSpeed) {
			playerBody.velocity.y = fix16Add(playerBody.velocity.y, gravityScale);
		}else {
			playerBody.velocity.y = FIX16(playerBody.maxFallSpeed);
		}
	}

	playerBody.globalPosition.x += playerBody.velocity.x;
	playerBody.globalPosition.y += fix16ToInt(playerBody.velocity.y);

	checkCollisions();

	playerBody.position.x = playerBody.globalPosition.x - cameraPosition.x;
	playerBody.position.y = playerBody.globalPosition.y - cameraPosition.y;

	if (!playerBody.collidingAgainstStair && playerBody.climbingStair) {
		playerBody.climbingStair = FALSE;
		climbStairPressed = FALSE;
	}

	updateAnimations();

	SPR_setPosition(playerBody.sprite, playerBody.position.x, playerBody.position.y);

	if (falling) {
		dyingSteps++;
		if(dyingSteps > dieDelay){
			SYS_hardReset(); //Reset when falling off the screen
		}
	}
}

void updateAnimations() {
	if (playerBody.input.x > 0) {
		SPR_setHFlip(playerBody.sprite, TRUE);
		playerBody.facingDirection = 1;
	}else if (playerBody.input.x < 0) {
		SPR_setHFlip(playerBody.sprite, FALSE);
		playerBody.facingDirection = -1;
	}

	if (playerBody.velocity.s8y == 0 && !playerBody.climbingStair) {
		if (playerBody.velocity.x != 0 && playerBody.runningAnim == FALSE && playerBody.onGround) {
			SPR_setAnim(playerBody.sprite, 1);
			playerBody.runningAnim = TRUE;
		}else if (playerBody.velocity.x == 0 && playerBody.onGround) {
			SPR_setAnim(playerBody.sprite, 0);
			playerBody.runningAnim = FALSE;
		}
	}

	if (playerBody.climbingStair) {
		SPR_setAnim(playerBody.sprite, 2);
	}
}

void checkCollisions() {
	playerBody.collidingAgainstStair = FALSE;

	//Create level limits
	AABB levelLimits = roomSize;

	//Easy access to the bounds in global pos
	if (playerBody.climbingStair) {
		playerBounds = newAABB(
			playerBody.globalPosition.x + playerBody.climbingStairAABB.min.x,
			playerBody.globalPosition.x + playerBody.climbingStairAABB.max.x,
			playerBody.globalPosition.y + playerBody.climbingStairAABB.min.y,
			playerBody.globalPosition.y + playerBody.climbingStairAABB.max.y
		);
	}else {
		playerBounds = newAABB(
			playerBody.globalPosition.x + playerBody.aabb.min.x,
			playerBody.globalPosition.x + playerBody.aabb.max.x,
			playerBody.globalPosition.y + playerBody.aabb.min.y,
			playerBody.globalPosition.y + playerBody.aabb.max.y
		);
	}
	s16 playerHeadPos = playerBody.aabb.min.y + playerBody.skinWidth + playerBody.globalPosition.y;
	s16 playerFeetPos = playerBody.aabb.max.y - playerBody.skinWidth + playerBody.globalPosition.y;

	//Positions in tiles
	Vect2D_u8 minTilePos = posToTile(newVector2D_s16(playerBounds.min.x, playerBounds.min.y));
	Vect2D_u8 maxTilePos = posToTile(newVector2D_s16(playerBounds.max.x, playerBounds.max.y));

	Vect2D_u8 tileBoundDifference = newVector2D_u8(maxTilePos.x - minTilePos.x, maxTilePos.y - minTilePos.y);

	int yVelocity = fix16ToInt(playerBody.velocity.y);
	playerBody.velocity.s8y = yVelocity;

	bool anyOnFloor = FALSE;
	for (u8 i = 0; i <= tileBoundDifference.x; i++) {
		if (getTileValue(minTilePos.x + i, maxTilePos.y) == 1 || getTileValue(minTilePos.x + i, maxTilePos.y) == 3) {
			if (getTileTopEdge(maxTilePos.y) >= playerFeetPos) {
				anyOnFloor = TRUE;
			}
		}
	}
	if (anyOnFloor == FALSE)
		tileBoundDifference.y++;

	for (u8 i = 0; i < tileBoundDifference.y; i++) {
		u8 rx = maxTilePos.x;
		u8 ry = minTilePos.y + i;

		u8 rTileValue = getTileValue(rx, ry);
		if (rTileValue == GROUND_TILE) {
			AABB tileBounds = getTileBounds(rx, ry);
			if (tileBounds.min.x < levelLimits.max.x && tileBounds.min.y < playerFeetPos && tileBounds.max.y > playerHeadPos) {
				levelLimits.max.x = tileBounds.min.x;
				break;
			}
		}else if (rTileValue == LADDER_TILE) {
			stairLeftEdge = getTileLeftEdge(rx);
			playerBody.collidingAgainstStair = TRUE;
		}

		s8 lx = minTilePos.x;
		s8 ly = minTilePos.y + i;

		u8 lTileValue = getTileValue(lx, ly);
		if (lTileValue == GROUND_TILE) {
			AABB tileBounds = getTileBounds(lx, ly);
			if (tileBounds.max.x > levelLimits.min.x && tileBounds.min.y < playerFeetPos && tileBounds.max.y > playerHeadPos) {
				levelLimits.min.x = tileBounds.max.x;
				break;
			}
		}else if (lTileValue == LADDER_TILE) {
			stairLeftEdge = getTileLeftEdge(lx);
			playerBody.collidingAgainstStair = TRUE;
		}
	}

	if (levelLimits.min.x > playerBounds.min.x) {
		if (levelLimits.min.x == 0) {
			playerBody.velocity.x = playerBody.velocity.fixX = 0;
			playerBody.globalPosition.x = levelLimits.min.x - playerBody.aabb.min.x;
		}else {
			playerBody.globalPosition.x = levelLimits.min.x - playerBody.aabb.min.x;
			playerBody.velocity.x = playerBody.velocity.fixX = 0;
		}
	}
	if (levelLimits.max.x < playerBounds.max.x) {
		if (levelLimits.max.x == 768) {
			playerBody.velocity.x = playerBody.velocity.fixX = 0;
			playerBody.globalPosition.x = levelLimits.max.x - playerBody.aabb.max.x;
		}else {
			playerBody.globalPosition.x = levelLimits.max.x - playerBody.aabb.max.x;
			playerBody.velocity.x = playerBody.velocity.fixX = 0;
		}
	}

	if (playerBody.climbingStair) {
		playerBounds = newAABB(
			playerBody.globalPosition.x + playerBody.climbingStairAABB.min.x,
			playerBody.globalPosition.x + playerBody.climbingStairAABB.max.x,
			playerBody.globalPosition.y + playerBody.climbingStairAABB.min.y,
			playerBody.globalPosition.y + playerBody.climbingStairAABB.max.y
		);
	}
	else {
		playerBounds = newAABB(
			playerBody.globalPosition.x + playerBody.aabb.min.x,
			playerBody.globalPosition.x + playerBody.aabb.max.x,
			playerBody.globalPosition.y + playerBody.aabb.min.y,
			playerBody.globalPosition.y + playerBody.aabb.max.y
		);
	}

	minTilePos = posToTile(newVector2D_s16(playerBounds.min.x, playerBounds.min.y));
	maxTilePos = posToTile(newVector2D_s16(playerBounds.max.x - 1, playerBounds.max.y));

	tileBoundDifference = newVector2D_u8(maxTilePos.x - minTilePos.x, maxTilePos.y - minTilePos.y);
	Vect2D_u16 stairPos = newVector2D_u16(0, 0);

	if (yVelocity != 0) {
		if (yVelocity > 0) {
			for (u8 i = 0; i <= tileBoundDifference.x; i++) {
				u8 x = minTilePos.x + i;
				u8 y = maxTilePos.y;

				u8 bottomTileValue = getTileValue(x, y);
				if (bottomTileValue == GROUND_TILE || bottomTileValue == ONE_WAY_PLATFORM_TILE) {
					if (getTileRightEdge(x) == levelLimits.min.x || getTileLeftEdge(x) == levelLimits.max.x)
						continue;

					u16 bottomEdgePos = getTileTopEdge(y);
					if (bottomEdgePos < levelLimits.max.y) {
						levelLimits.max.y = bottomEdgePos;
						break;
					}
				}else if (bottomTileValue == LADDER_TILE) {
					stairLeftEdge = getTileLeftEdge(x);
					playerBody.collidingAgainstStair = TRUE;

					u16 bottomEdgePos = getTileTopEdge(y);
					if (bottomEdgePos < levelLimits.max.y && bottomEdgePos > playerFeetPos && !playerBody.climbingStair && getTileValue(x, y - 1) != 2) {
						stairPos = newVector2D_u16(x, y);
						levelLimits.max.y = bottomEdgePos;
						break;
					}
				}
			}
		}else {
			for (u8 i = 0; i <= tileBoundDifference.x; i++) {
				s8 x = minTilePos.x + i;
				u8 y = minTilePos.y;

				u8 topTileValue = getTileValue(x, y);
				if (topTileValue == GROUND_TILE) {
					if (getTileRightEdge(x) == levelLimits.min.x || getTileLeftEdge(x) == levelLimits.max.x)
						continue;

					u16 upperEdgePos = getTileBottomEdge(y);
					if (upperEdgePos < levelLimits.max.y) {
						levelLimits.min.y = upperEdgePos;
						break;
					}
				}else if (topTileValue == LADDER_TILE) {
					stairLeftEdge = getTileLeftEdge(x);
					playerBody.collidingAgainstStair = TRUE;
				}
			}
		}
	}

	if (levelLimits.min.y > playerBounds.min.y) {
		playerBody.globalPosition.y = levelLimits.min.y - playerBody.aabb.min.y;
		playerBody.velocity.s8y = playerBody.velocity.y = 0;
	}

	if (levelLimits.max.y < playerBounds.max.y) {
		if (levelLimits.max.y == 768) {
			playerBody.onGround = FALSE;
			falling = TRUE;
		}else {
			if (stairPos.x || stairPos.y) {
				playerBody.onStair = TRUE;
			}else {
				playerBody.onStair = FALSE;
			}

			playerBody.onGround = TRUE;
			playerBody.climbingStair = FALSE;
			currentCoyoteTime = coyoteTime;
			playerBody.jumping = FALSE;
			playerBody.globalPosition.y = levelLimits.max.y - playerBody.aabb.max.y;
			playerBody.velocity.s8y = playerBody.velocity.y = 0;
		}
	}else {
		playerBody.onStair = playerBody.onGround = FALSE;
		currentCoyoteTime--;
	}

	if (playerBody.velocity.y > FIX16(1)) {
		playerBody.jumping = TRUE;
	}

	if (playerBody.climbingStair) {
		playerBounds = newAABB(
			playerBody.globalPosition.x + playerBody.climbingStairAABB.min.x,
			playerBody.globalPosition.x + playerBody.climbingStairAABB.max.x,
			playerBody.globalPosition.y + playerBody.climbingStairAABB.min.y,
			playerBody.globalPosition.y + playerBody.climbingStairAABB.max.y
		);
	}else {
		playerBounds = newAABB(
			playerBody.globalPosition.x + playerBody.aabb.min.x,
			playerBody.globalPosition.x + playerBody.aabb.max.x,
			playerBody.globalPosition.y + playerBody.aabb.min.y,
			playerBody.globalPosition.y + playerBody.aabb.max.y
		);
	}
}