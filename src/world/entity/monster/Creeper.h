#ifndef NET_MINECRAFT_WORLD_ENTITY_MONSTER__Creeper_H__
#define NET_MINECRAFT_WORLD_ENTITY_MONSTER__Creeper_H__

//package net.minecraft.world.entity.monster;

#include "Monster.h"
#include <string>

class Entity;

class Creeper: public Monster
{
	typedef Monster super;
public:
    Creeper(Level* level);

    /*@Override*/
    int getMaxHealth() override;

    void tick() override;

    float getSwelling(float a);

	virtual int getEntityTypeId() const override;
protected:
    int getDeathLoot() override;

	void checkCantSeeTarget(Entity* target, float d) override;
	void checkHurtTarget(Entity* target, float d) override;

	std::string getHurtSound() override;
	std::string getDeathSound() override;
private:
    int getSwellDir();
    void setSwellDir(int dir);

	int swell;
	int oldSwell;

	int swellDir;

	static const int DATA_SWELL_DIR = 16;
	static const int MAX_SWELL = 30;
};

#endif /*NET_MINECRAFT_WORLD_ENTITY_MONSTER__Creeper_H__*/
