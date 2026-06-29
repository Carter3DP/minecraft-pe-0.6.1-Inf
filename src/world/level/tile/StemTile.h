#ifndef NET_MINECRAFT_WORLD_LEVEL_TILE__StemTile_H__
#define NET_MINECRAFT_WORLD_LEVEL_TILE__StemTile_H__
#include "Bush.h"
class StemTile : public Bush {
	typedef Bush super;
public:
	StemTile(int id, Tile* fruit);
	bool mayPlaceOn(int tile) override;
	void tick(Level* level, int x, int y, int z, Random* random) override;
	void growCropsToMax(Level* level, int x, int y, int z);
	int getColor(int data);
	int getColor(LevelSource* level, int x, int y, int z) override;
	int getTexture(int face, int data) override;
	void updateDefaultShape() override;
	void updateShape(LevelSource* level, int x, int y, int z) override;
	int getRenderShape() override;
	int getConnectDir(LevelSource* level, int x, int y, int z);
	void spawnResources(Level* level, int x, int y, int z, int data, float odds) override;
	int getResource(int data, Random* random) override;
	int getResourceCount(Random* random) override;
private:
	float getGrowthSpeed(Level* level, int x, int y, int z);

private:
	Tile* fruit;
};

#endif /* NET_MINECRAFT_WORLD_LEVEL_TILE__StemTile_H__ */
