#include "../inc/physics.h"

//Used to get the left edge of a tile by inputting the tile position
u16 getTileLeftEdge(u8 x) {
	return (x << 4);
}
//Used to get the right edge of a tile by inputting the tile position
u16 getTileRightEdge(u8 x) {
	return (x << 4) + 16;
}
//Used to get the top edge of a tile by inputting the tile position
u16 getTileTopEdge(u8 y) {
	return (y << 4);
}
//Used to get the bottom edge of a tile by inputting the tile position
u16 getTileBottomEdge(u8 y) {
	return (y << 4) + 16;
}
//Used to get the bounds of a tile by inputting the tile position
AABB getTileBounds(int x, int y) {
	return newAABB((x << 4), (x << 4) + 16, (y << 4), (y << 4) + 16);
}

//Used to get the tile position out of a pixel position
Vect2D_u8 posToTile(Vect2D_s16 position) {
	return newVector2D_u8((position.x >> 4), (position.y >> 4));
}