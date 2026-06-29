#ifndef NET_MINECRAFT_WORLD_ENTITY_ITEM__PrimedTnt_H__
#define NET_MINECRAFT_WORLD_ENTITY_ITEM__PrimedTnt_H__

//package net.minecraft.world.entity.item;

#include "../Entity.h"
#include "../../level/Level.h"

class CompoundTag;


class PrimedTnt: public Entity
{
	typedef Entity super;
public:
    PrimedTnt(Level* level);
    PrimedTnt(Level* level, double x, double y, double z);

	void tick() override;
	
	bool isPickable() override;

	int getEntityTypeId() const override;

	float getShadowHeightOffs() override;
protected:
    void addAdditonalSaveData(CompoundTag* entityTag) override;
	void readAdditionalSaveData(CompoundTag* tag) override;
private:
    void explode();

public:
	int life;
};

#endif /*NET_MINECRAFT_WORLD_ENTITY_ITEM__PrimedTnt_H__*/
