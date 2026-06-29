#ifndef NET_MINECRAFT_WORLD_ENTITY_ITEM__FallingTile_H__
#define NET_MINECRAFT_WORLD_ENTITY_ITEM__FallingTile_H__

//package net.minecraft.world.entity.item;

#include "../Entity.h"

class Level;
class CompoundTag;

class FallingTile: public Entity
{
public:
    FallingTile(Level* level);
    FallingTile(Level* level, double x, double y, double z, int tile, int data = 0);

    void init();

    bool isPickable() override;

    void tick() override;

	int getEntityTypeId() const override;

	float getShadowHeightOffs() override;

    Level* getLevel();

protected:
	void addAdditonalSaveData(CompoundTag* tag) override;
    void readAdditionalSaveData(CompoundTag* tag) override;

public:
	int tile;
    int data;

    int time;
};

#endif /*NET_MINECRAFT_WORLD_ENTITY_ITEM__FallingTile_H__*/
