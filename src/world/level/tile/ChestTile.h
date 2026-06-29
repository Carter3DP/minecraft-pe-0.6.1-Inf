#ifndef NET_MINECRAFT_WORLD_LEVEL_TILE__ChestTile_H__
#define NET_MINECRAFT_WORLD_LEVEL_TILE__ChestTile_H__

#include "EntityTile.h"
class Level;
class LevelSource;
class Mob;

#include "../../../util/Random.h"

//package net.minecraft.world.level->tile;

class ChestTile: public EntityTile
{
	typedef EntityTile super;
public:
    static const int EVENT_SET_OPEN_COUNT = 1;

    ChestTile(int id);

    bool isSolidRender() override;

    /*@Override*/
    bool isCubeShaped() override;

    int getRenderShape() override;

	bool mayPlace(Level* level, int x, int y, int z, unsigned char face) override;
	void setPlacedBy(Level* level, int x, int y, int z, Mob* by) override;

	void onPlace(Level* level, int x, int y, int z) override;
	void onRemove(Level* level, int x, int y, int z) override;

    void recalcLockDir(Level* level, int x, int y, int z);

    int getTexture(LevelSource* level, int x, int y, int z, int face) override;
    int getTexture(int face) override;

	void neighborChanged(Level* level, int x, int y, int z, int type) override;

    bool use(Level* level, int x, int y, int z, Player* player) override;

    TileEntity* newTileEntity() override;

private:
	bool isFullChest(Level* level, int x, int y, int z);

	Random random;
};

#endif /*NET_MINECRAFT_WORLD_LEVEL_TILE__ChestTile_H__*/
