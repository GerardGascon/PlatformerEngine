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
			}else if (playerBody.jumping && playerBody.velocity.fixY < 0) {
				//If the button is released we remove half of the velocity
				playerBody.velocity.fixY = fix16Mul(playerBody.velocity.fixY, FIX16(.5));
			}
		}

		//Down and up buttons are only used when it is climbing stair
		//NOTE: Up direction is -1 and down direction is 1, this is because the Mega Drive draws the screen from top to bottom
		if (changed & BUTTON_DOWN) {
			if (state & BUTTON_DOWN) {
				playerBody.input.y = 1;
				if (playerBody.climbingStair) {
					playerBody.velocity.fixY = FIX16(playerBody.climbingSpeed);
				}else if (playerBody.onStair) {
					playerBody.velocity.fixY = FIX16(playerBody.climbingSpeed);
					playerBody.climbingStair = TRUE;
				}
			}else if (playerBody.climbingStair) {
				playerBody.velocity.fixY = 0;
			}
		}
		if (changed & BUTTON_UP) {
			if (state & BUTTON_UP) {
				playerBody.input.y = -1;
				if (playerBody.collidingAgainstStair && !playerBody.onStair) {
					playerBody.climbingStair = TRUE;
					playerBody.velocity.fixY = FIX16(-playerBody.climbingSpeed);
					KLog_U1("", playerBody.onStair);
				}else {
					climbStairPressed = TRUE;
				}
			}else {
				playerBody.input.y = 0;
				climbStairPressed = FALSE;
				if (playerBody.climbingStair) {
					playerBody.velocity.fixY = 0;
				}
			}
		}
	}
}

void updatePlayer() {
	//Check if the player wants to climb a stair
	if (climbStairPressed && playerBody.collidingAgainstStair && !playerBody.onStair) {
		playerBody.climbingStair = TRUE;
		playerBody.velocity.fixY = FIX16(-playerBody.climbingSpeed);
		climbStairPressed = FALSE;
	}

	//Check if player wants to jump by looking the coyote time and jump buffer
	if (currentCoyoteTime > 0 && currentJumpBufferTime > 0) {
		playerBody.jumping = TRUE;
		playerBody.velocity.fixY = FIX16(-playerBody.jumpSpeed);

		currentCoyoteTime = 0;
		currentJumpBufferTime = 0;
	}
	//The ground hasn't been checked yet so we only decrease the jump buffer time for now
	currentJumpBufferTime = clamp((currentJumpBufferTime - 1), 0, jumpBufferTime); //Clamp to avoid underflowing, it is unlikely to happen but can happen

	//If the player is climbing a stair, it only needs to go upward, if not, we apply horizontal movement
	if (playerBody.climbingStair) {
		playerBody.velocity.x = playerBody.velocity.fixX = 0;
		playerBody.globalPosition.x = stairLeftEdge - stairPositionOffset;
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

	//Apply gravity with a terminal velocity
	if (!playerBody.onGround && !playerBody.climbingStair) {
		if (fix16ToInt(playerBody.velocity.fixY) <= playerBody.maxFallSpeed) {
			playerBody.velocity.fixY = fix16Add(playerBody.velocity.fixY, gravityScale);
		}else {
			playerBody.velocity.fixY = FIX16(playerBody.maxFallSpeed);
		}
	}

	//Once all the input-related have been calculated, we apply the velocities to the global positions
	playerBody.globalPosition.x += playerBody.velocity.x;
	playerBody.globalPosition.y += fix16ToInt(playerBody.velocity.fixY);

	//Now we can check for collisions and correct those positions
	checkCollisions();

	//Now that the collisions have been checked, we know if the player is on a stair or not
	if (!playerBody.collidingAgainstStair && playerBody.climbingStair) {
		playerBody.climbingStair = FALSE;
		climbStairPressed = FALSE;
	}

	//Once the positions are correct, we position the player taking into account the camera position
	playerBody.position.x = playerBody.globalPosition.x - cameraPosition.x;
	playerBody.position.y = playerBody.globalPosition.y - cameraPosition.y;
	SPR_setPosition(playerBody.sprite, playerBody.position.x, playerBody.position.y);
	
	//Update the player animations
	updateAnimations();

	//Reset when falling off the screen
	if (falling) {
		dyingSteps++;
		if(dyingSteps > dieDelay){
			SYS_hardReset();
		}
	}
}

void updateAnimations() {
	//Sprite flip depending on the horizontal input
	if (playerBody.input.x > 0) {
		SPR_setHFlip(playerBody.sprite, TRUE);
		playerBody.facingDirection = 1;
	}else if (playerBody.input.x < 0) {
		SPR_setHFlip(playerBody.sprite, FALSE);
		playerBody.facingDirection = -1;
	}

	//If the player is on ground and not climbing the stair it can be idle or running
	if (playerBody.velocity.fixY == 0 && !playerBody.climbingStair) {
		if (playerBody.velocity.x != 0 && playerBody.runningAnim == FALSE && playerBody.onGround) {
			SPR_setAnim(playerBody.sprite, 1);
			playerBody.runningAnim = TRUE;
		}else if (playerBody.velocity.x == 0 && playerBody.onGround) {
			SPR_setAnim(playerBody.sprite, 0);
			playerBody.runningAnim = FALSE;
		}
	}

	//Climb animation
	if (playerBody.climbingStair) {
		SPR_setAnim(playerBody.sprite, 2);
	}
}

void checkCollisions() {
	//As we now have to check for collisions, we will later check if it is true or false, but for now it is false
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

	//We can see this variables as a way to avoid thinking that a ground tile is a wall tile
	s16 playerHeadPos = playerBody.aabb.min.y + playerBody.skinWidth + playerBody.globalPosition.y;
	s16 playerFeetPos = playerBody.aabb.max.y - playerBody.skinWidth + playerBody.globalPosition.y;

	//Positions in tiles
	Vect2D_u8 minTilePos = posToTile(newVector2D_s16(playerBounds.min.x, playerBounds.min.y));
	Vect2D_u8 maxTilePos = posToTile(newVector2D_s16(playerBounds.max.x, playerBounds.max.y));

	//Used to limit how many tiles we have to check for collision
	//The +1 is for ensuring that we check for ground tiles
	Vect2D_u16 tileBoundDifference = newVector2D_u16(maxTilePos.x - minTilePos.x, maxTilePos.y - minTilePos.y + 1);

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

	tileBoundDifference = newVector2D_u16(maxTilePos.x - minTilePos.x, maxTilePos.y - minTilePos.y);
	Vect2D_u16 stairPos = newVector2D_u16(0, 0);

	if (playerBody.velocity.fixY >= 0) {
		for (u16 i = 0; i <= tileBoundDifference.x; i++) {
			s16 x = minTilePos.x + i;
			u16 y = maxTilePos.y;

			u16 bottomTileValue = getTileValue(x, y);
			if (bottomTileValue == GROUND_TILE || bottomTileValue == ONE_WAY_PLATFORM_TILE) {
				if (getTileRightEdge(x) == levelLimits.min.x || getTileLeftEdge(x) == levelLimits.max.x)
					continue;

				u16 bottomEdgePos = getTileTopEdge(y);
				if (bottomEdgePos < levelLimits.max.y) {
					levelLimits.max.y = bottomEdgePos;
				}
			}else if (bottomTileValue == LADDER_TILE) {
				stairLeftEdge = getTileLeftEdge(x);
				playerBody.collidingAgainstStair = TRUE;

				u16 bottomEdgePos = getTileTopEdge(y);
				if (bottomEdgePos <= levelLimits.max.y && bottomEdgePos > playerFeetPos && !playerBody.climbingStair && getTileValue(x, y - 1) != LADDER_TILE) {
					stairPos = newVector2D_u16(x, y);
					levelLimits.max.y = bottomEdgePos;
					break;
				}
			}
		}
	}else {
		for (u16 i = 0; i <= tileBoundDifference.x; i++) {
			s16 x = minTilePos.x + i;
			u16 y = minTilePos.y;

			u16 topTileValue = getTileValue(x, y);
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

	if (levelLimits.min.y > playerBounds.min.y) {
		playerBody.globalPosition.y = levelLimits.min.y - playerBody.aabb.min.y;
		playerBody.velocity.fixY = 0;
	}

	if (levelLimits.max.y <= playerBounds.max.y) {
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
			playerBody.velocity.fixY = 0;
		}
	}else {
		playerBody.onStair = playerBody.onGround = FALSE;
		currentCoyoteTime--;
	}

	if (playerBody.velocity.fixY > FIX16(1)) {
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