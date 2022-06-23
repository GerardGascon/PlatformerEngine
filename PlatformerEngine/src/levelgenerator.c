#include "../inc/levelgenerator.h"

#include "../res/resources.h"
#include "../inc/global.h"
#include "../inc/map.h"

u8** currentMap;

Map* bga;

void freeCollisionMap() {
	for (u16 i = 0; i < 14; i++) {
		MEM_free(currentMap[i]);
	}
	MEM_free(currentMap);
}

void generateCollisionMap(const u8 map[][48]) {
	roomSize = newAABB(0, 768, 0, 768);
	currentMap = (u8**)MEM_alloc(48 * sizeof(u8*));
	for (u16 i = 0; i < 48; i++) {
		currentMap[i] = (u8*)MEM_alloc(48);
		memcpy(currentMap[i], map[i], 48);
	}
}

u8 getTileValue(s16 x, s16 y) {
	if (x >= 48 || x < 0 || y < 0 || y >= 48)
		return 0;

	return currentMap[y][x];
}