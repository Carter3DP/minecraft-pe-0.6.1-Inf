#ifndef NET_MINECRAFT_WORLD_LEVEL_TILE__MelonTile_H__
#define NET_MINECRAFT_WORLD_LEVEL_TILE__MelonTile_H__
#include "Tile.h"
class MelonTile : public Tile {
	typedef Tile super;
public:
	MelonTile(int id);
	int getTexture(LevelSource* level, int x, int y, int z, int face) override;
	int getTexture(int face) override;
	int getResource(int data, Random* random) override;
	int getResourceCount(Random* random) override;
private:
	static const int TEX = 8 + 8 * 16;
	static const int TEX_TOP = 9 + 8 * 16;
};

#endif /* NET_MINECRAFT_WORLD_LEVEL_TILE__MelonTile_H__ */
