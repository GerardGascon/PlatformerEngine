#include "../inc/physics.h"

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