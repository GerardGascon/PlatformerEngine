#include "../inc/levels.h"

#include "../inc/levelgenerator.h"
#include "../inc/map.h"
#include "../inc/global.h"

#include "../res/resources.h"

void loadLevel() {
	VDPTilesFilled = TILE_USER_INDEX;

	PAL_setPalette(LEVEL_PALETTE, level_palette.data, DMA);
	VDP_loadTileSet(&level_tileset, VDPTilesFilled, DMA);
	bga = MAP_create(&level_map, TILEMAP_PLANE, TILE_ATTR_FULL(LEVEL_PALETTE, FALSE, FALSE, FALSE, VDPTilesFilled));

	VDPTilesFilled += level_tileset.numTile;

	generateCollisionMap(collisionMap);
}