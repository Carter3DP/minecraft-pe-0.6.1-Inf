#ifndef NET_MINECRAFT_WORLD_LEVEL_TILE__CropTile_H__
#define NET_MINECRAFT_WORLD_LEVEL_TILE__CropTile_H__

#include "Bush.h"

class CropTile : public Bush
{
	typedef Bush super;
public:
	CropTile(int id, int tex);
	bool mayPlaceOn(int tile) override;
	void tick(Level* level, int x, int y, int z, Random* random) override;

	int getTexture(LevelSource* level, int x, int y, int z, int face) override;
	int getTexture( int face, int data ) override;

	int getRenderShape() override;

	void spawnResources(Level* level, int x, int y, int z, int data, float odds) override;
	int getResource(int data, Random* random) override;
	int getResourceCount(Random* random) override;

	void growCropsToMax(Level* level, int x, int y, int z);
private:
	float getGrowthSpeed(Level* level, int x, int y, int z);
	
};

#endif /* NET_MINECRAFT_WORLD_LEVEL_TILE__CropTile_H__ */
