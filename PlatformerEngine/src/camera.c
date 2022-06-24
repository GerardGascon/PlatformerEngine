#include "../inc/camera.h"

#include "../inc/global.h"

#include "../res/resources.h"

Vect2D_s16 cameraPosition;

AABB cameraDeadzone;

void setupCamera(Vect2D_u16 deadZoneCenter, u16 deadZoneWidth, u16 deadZoneHeight) {
	//Calculates the bounds of the deadZone
	cameraDeadzone.min.x = deadZoneCenter.x - (deadZoneWidth >> 1);
	cameraDeadzone.max.x = deadZoneCenter.x + (deadZoneWidth >> 1);
	cameraDeadzone.min.y = deadZoneCenter.y - (deadZoneHeight >> 1);
	cameraDeadzone.max.y = deadZoneCenter.y + (deadZoneHeight >> 1);

	updateCamera();
	MAP_scrollToEx(bga, cameraPosition.x, cameraPosition.y, TRUE);
}

void updateCamera() {
	Vect2D_s16 globalPosition = playerBody.globalPosition;
	Vect2D_u16 centerOffset = playerBody.centerOffset;

	if (globalPosition.x + centerOffset.x > cameraPosition.x + cameraDeadzone.max.x) {
		cameraPosition.x = globalPosition.x + centerOffset.x - cameraDeadzone.max.x;
	}else if (globalPosition.x + centerOffset.x < cameraPosition.x + cameraDeadzone.min.x) {
		cameraPosition.x = globalPosition.x + centerOffset.x - cameraDeadzone.min.x;
	}

	if (globalPosition.y + centerOffset.y > cameraPosition.y + cameraDeadzone.max.y) {
		cameraPosition.y = globalPosition.y + centerOffset.y - cameraDeadzone.max.y;
	}else if (globalPosition.y + centerOffset.y < cameraPosition.y + cameraDeadzone.min.y) {
		cameraPosition.y = globalPosition.y + centerOffset.y - cameraDeadzone.min.y;
	}

	cameraPosition.x = clamp(cameraPosition.x, 0, 448); // 768 - 320 = 448 (level width - screen width)
	cameraPosition.y = clamp(cameraPosition.y, 0, 544); // 768 - 224 = 544 (level height - screen height)

	MAP_scrollTo(bga, cameraPosition.x, cameraPosition.y);
}