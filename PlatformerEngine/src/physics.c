#include "../inc/physics.h"

#include "../inc/global.h"

#define IN_BETWEEN 0x01
#define OVERLAPPED 0x02
#define TO_THE_LEFT 0x04
#define TO_THE_RIGHT 0x08
#define ABOVE 0x10
#define UNDER 0x20

u8 axisXBoxRelativePos(const AABB* object1, const AABB* object2);
u8 axisXPxRelativePos(s16 x_px, const AABB* object2);
u8 axisYBoxRelativePos(const AABB* object1, const AABB* object2);
u8 axisYPxRelativePos(s16 y_px, const AABB* object2);

bool overlapX(const AABB object, const s16 positionX) {
	return (object.min.x <= positionX && object.max.x >= positionX);
}
bool overlapY(const AABB object, const u16 positionY) {
	return (object.min.y <= positionY && object.max.y >= positionY);
}

bool overlap(const AABB* object1, const AABB* object2) {
	return (OVERLAPPED & axisXBoxRelativePos(object1, object2)) && (OVERLAPPED & axisYBoxRelativePos(object1, object2));
}

u8 axisXBoxRelativePos(const AABB* object1, const AABB* object2) {
	if (TO_THE_RIGHT & axisXPxRelativePos(object1->min.x, object2))
		return TO_THE_RIGHT;
	if (TO_THE_LEFT & axisXPxRelativePos(object1->max.x, object2))
		return TO_THE_LEFT;
	return OVERLAPPED;
}
u8 axisXPxRelativePos(s16 x_px, const AABB* object2) {
	if (x_px < object2->min.x)
		return TO_THE_LEFT;
	if (x_px > object2->max.x)
		return TO_THE_RIGHT;
	return IN_BETWEEN;
}
u8 axisYBoxRelativePos(const AABB* object1, const AABB* object2) {
	if (UNDER & axisYPxRelativePos(object1->min.y, object2))
		return UNDER;
	if (ABOVE & axisYPxRelativePos(object1->max.y, object2))
		return ABOVE;
	return OVERLAPPED;
}
u8 axisYPxRelativePos(s16 y_px, const AABB* object2) {
	if (y_px < object2->min.y)
		return ABOVE;
	if (y_px > object2->max.y)
		return UNDER;
	return IN_BETWEEN;
}

AABB newAABB(s16 x1, s16 x2, s16 y1, s16 y2) {
	return (AABB) { {x1, y1}, { x2, y2 } };
}

u16 getTileLeftEdge(u8 x) {
	return (x << 4);
}
u16 getTileRightEdge(u8 x) {
	return (x << 4) + 16;
}
u16 getTileTopEdge(u8 y) {
	return (y << 4);
}
u16 getTileBottomEdge(u8 y) {
	return (y << 4) + 16;
}
AABB getTileBounds(int x, int y) {
	return newAABB((x << 4), (x << 4) + 16, (y << 4), (y << 4) + 16);
}

Vect2D_u8 posToTile(Vect2D_s16 position) {
	return newVector2D_u8((position.x >> 4), (position.y >> 4));
}