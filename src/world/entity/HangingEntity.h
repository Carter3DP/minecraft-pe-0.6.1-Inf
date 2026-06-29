#ifndef NET_MINECRAFT_WORLD_ENTITY__HangingEntity_H__
#define NET_MINECRAFT_WORLD_ENTITY__HangingEntity_H__

#include "Entity.h"
class HangingEntity : public Entity {
	typedef Entity super;
public:
	HangingEntity(Level* level);
	HangingEntity(Level* level, int xTile, int yTile, int zTile, int dir);
	void init();
	void setDir(int dir);
	void setPosition(int x, int y, int z);
	virtual void tick() override;
	virtual bool survives();
	bool isPickable() override;
	bool interact(Player* player) override;
	void move(double xa, double ya, double za) override;
	void push(double xa, double ya, double za) override;
	virtual void addAdditonalSaveData(CompoundTag* tag) override;
	virtual void readAdditionalSaveData(CompoundTag* tag) override;
	virtual int getWidth() = 0;
	virtual int getHeight() = 0;
	virtual void dropItem() = 0;
	virtual bool isHangingEntity() override;
	virtual float getBrightness(float a) override;
	virtual bool hurt(Entity* source, int damage) override;
private:
	float offs(int w);
public:
	int dir;
	int xTile, yTile, zTile;
private:
	int checkInterval;
};

#endif /* NET_MINECRAFT_WORLD_ENTITY__HangingEntity_H__ */
