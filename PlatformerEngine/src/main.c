#include <genesis.h>

#include "../inc/levels.h"
#include "../inc/camera.h"
#include "../inc/player.h"

void inGameJoyEvent(u16 joy, u16 changed, u16 state);

int main() {
	JOY_init();
	SPR_init();

	loadLevel();
	playerInit();
	setupCamera(newVector2D_u16(160, 112), 20, 20);

	JOY_setEventHandler(inGameJoyEvent);

	while (TRUE) {
		updatePlayer();
		updateCamera();

		KLog_U1("", SYS_getCPULoad());

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