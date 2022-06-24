#include <genesis.h>

#include "../inc/levels.h"
#include "../inc/camera.h"
#include "../inc/player.h"

void inGameJoyEvent(u16 joy, u16 changed, u16 state);

int main(u16 resetType) {
	//Soft resets don't clear RAM, this can bring some bugs so we hard reset every time we detect a soft reset
	if (!resetType)
		SYS_hardReset();

	JOY_init();
	SPR_init();

	loadLevel();
	playerInit();
	setupCamera(newVector2D_u16(160, 112), 20, 20); //We have to setup always after the player, it directly depends on player's data

	JOY_setEventHandler(inGameJoyEvent);

	while (TRUE) {
		updatePlayer();
		updateCamera();

		SYS_doVBlankProcess();
		SPR_update();
	}
	
	return 0;
}

void inGameJoyEvent(u16 joy, u16 changed, u16 state) {
	input.joy = joy;
	input.changed = changed;
	input.state = state;

	playerInputChanged();
}