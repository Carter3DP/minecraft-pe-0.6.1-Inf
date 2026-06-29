#ifndef NET_MINECRAFT_WORLD_LEVEL_TILE__FurnaceTile_H__
#define NET_MINECRAFT_WORLD_LEVEL_TILE__FurnaceTile_H__

//package net.minecraft.world.level->tile;

#include "EntityTile.h"
#include "../../../util/Random.h"

class Level;
class Mob;
class Player;
class LevelSource;

class FurnaceTile: public EntityTile
{
    typedef EntityTile super;
public:
    FurnaceTile(int id, bool lit);

	int getTexture(int face) override;
    int getTexture(LevelSource* level, int x, int y, int z, int face) override;

    void animateTick(Level* level, int xt, int yt, int zt, Random* random) override;

    bool use(Level* level, int x, int y, int z, Player* player) override;
	int getResource(int data, Random* random/*, int playerBonusLevel*/) override;

    static void setLit(bool lit, Level* level, int x, int y, int z);

    TileEntity* newTileEntity() override;

    void setPlacedBy(Level* level, int x, int y, int z, Mob* by) override;

	void onPlace(Level* level, int x, int y, int z) override;
    void onRemove(Level* level, int x, int y, int z) override;
private:
    void recalcLockDir(Level* level, int x, int y, int z);

    Random random;
    const bool lit;
    static bool noDrop;
};

#endif /*NET_MINECRAFT_WORLD_LEVEL_TILE__FurnaceTile_H__*/
