#include "../inc/camera.h"

#include "../inc/global.h"

#include "../res/resources.h"

struct cam camera;

AABB cameraDeadzone;

void setupCamera(Vect2D_u16 deadZoneCenter, u16 deadZoneWidth, u16 deadZoneHeight) {
	camera.position = newVector2D_s16(0, 0);
	camera.deadZoneCenter = deadZoneCenter;
	camera.deadZoneSize.width = deadZoneWidth;
	camera.deadZoneSize.height = deadZoneHeight;

	cameraDeadzone.min.x = deadZoneCenter.x - deadZoneWidth / 2;
	cameraDeadzone.max.x = deadZoneCenter.x + deadZoneWidth / 2;
	cameraDeadzone.min.y = deadZoneCenter.y - deadZoneHeight / 2;
	cameraDeadzone.max.y = deadZoneCenter.y + deadZoneHeight / 2;

	updateCamera();
	MAP_scrollToEx(bga, camera.position.x, camera.position.y, TRUE);
}

void updateCamera() {
	Vect2D_s16 globalPosition = playerBody.globalPosition;
	Vect2D_u16 centerOffset = playerBody.centerOffset;

	if (globalPosition.x + centerOffset.x > camera.position.x + cameraDeadzone.max.x) {
		camera.position.x = globalPosition.x + centerOffset.x - cameraDeadzone.max.x;
	}else if (globalPosition.x + centerOffset.x < camera.position.x + cameraDeadzone.min.x) {
		camera.position.x = globalPosition.x + centerOffset.x - cameraDeadzone.min.x;
	}

	if (globalPosition.y + centerOffset.y > camera.position.y + cameraDeadzone.max.y) {
		camera.position.y = globalPosition.y + centerOffset.y - cameraDeadzone.max.y;
	}else if (globalPosition.y + centerOffset.y < camera.position.y + cameraDeadzone.min.y) {
		camera.position.y = globalPosition.y + centerOffset.y - cameraDeadzone.min.y;
	}

	camera.position.x = clamp(camera.position.x, 0, 448); // 768 - 320 = 448
	camera.position.y = clamp(camera.position.y, 0, 544); // 768 - 224 = 544

	MAP_scrollTo(bga, camera.position.x, camera.position.y);
}

void setHorizontalPlayerPos(s16 direction) {
	if (direction > 0) {
		playerBody.globalPosition.x = 768 - playerBody.aabb.max.x;
	}else if (direction < 0) {
		playerBody.globalPosition.x = -playerBody.aabb.min.x;
	}
}