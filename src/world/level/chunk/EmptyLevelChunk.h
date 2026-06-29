#ifndef NET_MINECRAFT_WORLD_LEVEL_CHUNK__EmptyLevelChunk_H__
#define NET_MINECRAFT_WORLD_LEVEL_CHUNK__EmptyLevelChunk_H__

//package net.minecraft.world.level.chunk;

/* import java.util.* */

//#include "world/entity/Entity.h"
/* import net.minecraft.world.level.* */
//#include "world/level/tile/entity/TileEntity.h"
//#include "world/phys/AABB.h"

#include "LevelChunk.h"
#include "../tile/Tile.h"
#include "../Level.h"

class EmptyLevelChunk: public LevelChunk
{
public:
    EmptyLevelChunk(Level* level, int x, int z)
	:	LevelChunk(level, x, z)
	{
        dontSave = true;
    }

    EmptyLevelChunk(Level* level, unsigned char* blocks, int x, int z)
	:	LevelChunk(level, blocks, x, z)
	{
        dontSave = true;
    }

    bool isAt(int x, int z) override {
        return x == this->x && z == this->z;
    }

    int getHeightmap(int x, int z) override {
        return 0;
    }

    void recalcBlockLights() override {
    }

    void recalcHeightmapOnly() override {
        return;
    }

    void recalcHeightmap() override {
        return;
    }

    void lightLava() override {
        return;
    }

    int getTile(int x, int y, int z) override {
		//if (y <= 1)
		//{
		//	return Tile::unbreakable->id;
		//}
		//else if (y < 64)
		//{
		//	return Tile::water->id;
		//}
		//return 0;
        return 0;
    }

    bool setTileAndData(int x, int y, int z, int _tile, int _data) override {
        return true;
    }

    bool setTile(int x, int y, int z, int _tile) override {
        return true;
    }

    int getData(int x, int y, int z) override {
        return 0;
    }

    void setData(int x, int y, int z, int val) override {
        return;
    }

    int getBrightness(const LightLayer& layer, int x, int y, int z) override {
        return 7;
    }

    void setBrightness(const LightLayer& layer, int x, int y, int z, int brightness) override {
        return;
    }

    int getRawBrightness(int x, int y, int z, int skyDampen) override {
        return 7;
    }

    void addEntity(Entity* e) override {
        return;
    }

    void removeEntity(Entity* e) override {
        return;
    }

    void removeEntity(Entity* e, int yc) override {
        return;
    }

    bool isSkyLit(int x, int y, int z) override {
        return false;
    }

    void skyBrightnessChanged() override {
        return;
    }
/*
    TileEntity getTileEntity(int x, int y, int z) {
        return NULL;
    }

    void addTileEntity(TileEntity te) {
        return;
    }

    void setTileEntity(int x, int y, int z, TileEntity tileEntity) {
        return;
    }

    void removeTileEntity(int x, int y, int z) {
        return;
    }
	*/

    void load() override {
        return;
    }

    void unload() override {
        return;
    }

    void markUnsaved() override {
        return;
    }

	void getEntities(Entity* except, const AABB& bb, std::vector<Entity*>& es) override {
        return;
    }
	/*
    void getEntitiesOfClass(Class<? extends Entity> baseClass, AABB bb, List<Entity> es) {
        return;
    }

    int countEntities() {
        return 0;
    }

    bool shouldSave(bool force) {
        return false;
    }
*/

    void setBlocks(unsigned char* newBlocks, int sub) override {
        return;
    }

    int getBlocksAndData(unsigned char* data, int x0, int y0, int z0, int x1, int y1, int z1, int p) override {
        int xs = x1 - x0;
        int ys = y1 - y0;
        int zs = z1 - z0;

        int s = xs * ys * zs;
        int len = s + s / 2 * 3;

		memset(data + p, 0, len);
        return len;
    }

    int setBlocksAndData(unsigned char* data, int x0, int y0, int z0, int x1, int y1, int z1, int p) override {
        int xs = x1 - x0;
        int ys = y1 - y0;
        int zs = z1 - z0;

        int s = xs * ys * zs;
        return s + s / 2 * 3;
    }

    Random getRandom(long l) override {
        return /*new*/ Random((level->getSeed() + x * x * 4987142 + x * 5947611 + z * z * 4392871l + z * 389711) ^ l);
    }

    bool isEmpty() override {
        return true;
    }
};

#endif /*NET_MINECRAFT_WORLD_LEVEL_CHUNK__EmptyLevelChunk_H__*/
