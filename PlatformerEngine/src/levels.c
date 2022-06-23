#include "../inc/levels.h"

#include "../inc/levelgenerator.h"
#include "../inc/map.h"
#include "../inc/global.h"

#include "../res/resources.h"

void loadLevel() {
	VDPTilesFilled = TILE_USER_INDEX;

	PAL_setPalette(PAL1, level_palette.data, DMA);
	VDP_loadTileSet(&level_tileset, VDPTilesFilled, DMA);
	bga = MAP_create(&level_map, TILEMAP_PLANE, TILE_ATTR_FULL(PAL1, FALSE, FALSE, FALSE, VDPTilesFilled));
	MAP_scrollTo(bga, 0, 0);

	VDPTilesFilled += level_tileset.numTile;

	generateScreen();
}

void generateScreen() {
	generateCollisionMap(collisionMap);
}