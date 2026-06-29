#ifndef NET_MINECRAFT_WORLD_LEVEL_TILE__TrapDoorTile_H__
#define NET_MINECRAFT_WORLD_LEVEL_TILE__TrapDoorTile_H__

#include "Tile.h"
class TrapDoorTile : public Tile {
	typedef Tile super;

public:
	TrapDoorTile(int id, const Material* material);
	bool blocksLight();
	bool isSolidRender() override;
	bool isCubeShaped() override;
	int getRenderLayer() override;
	//@Override
	//	public boolean isPathfindable(LevelSource level, int x, int y, int z) {
	//		return !isOpen(level.getData(x, y, z));
	//}
	int getRenderShape() override;
	AABB getTileAABB(Level* level, int x, int y, int z) override;
	AABB* getAABB(Level* level, int x, int y, int z) override;
	void updateShape(LevelSource* level, int x, int y, int z) override;
	void updateDefaultShape() override;
	void setShape(int data);
	void attack(Level* level, int x, int y, int z, Player* player) override;
	bool use(Level* level, int x, int y, int z, Player* player) override;
	void setOpen(Level* level, int x, int y, int z, bool shouldOpen);
	void neighborChanged(Level* level, int x, int y, int z, int type) override;
	HitResult clip(Level* level, int xt, int yt, int zt, const Vec3& a, const Vec3& b) override;
	int getDir(int dir);
	int getPlacedOnFaceDataValue(Level* level, int x, int y, int z, int face, float clickX, float clickY, float clickZ, int itemValue) override;
	bool mayPlace(Level* level, int x, int y, int z, unsigned char face) override;
	static bool isOpen(int data);
	static bool attachesTo(int id);
};

#endif /* NET_MINECRAFT_WORLD_LEVEL_TILE__TrapDoorTile_H__ */
