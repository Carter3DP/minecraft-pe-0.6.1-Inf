#ifndef NET_MINECRAFT_WORLD_ENTITY_MONSTER__Skeleton_H__
#define NET_MINECRAFT_WORLD_ENTITY_MONSTER__Skeleton_H__

//package net.minecraft.world.entity->monster;

#include "Monster.h"
#include <string>
#include "../../item/ItemInstance.h"

class Level;
class Entity;

class Skeleton: public Monster
{
    typedef Monster super;
public:
    Skeleton(Level* level);

    /*@Override*/
    int getMaxHealth() override;

    void aiStep() override;

    int getDeathLoot() override;

    ItemInstance* getCarriedItem() override;

	virtual int getEntityTypeId() const override;
protected:
    const char* getAmbientSound() override;
    std::string getHurtSound() override;
    std::string getDeathSound() override;

    void checkHurtTarget(Entity* target, float d) override;
    /*@Override*/
    void dropDeathLoot(/*bool wasKilledByPlayer, int playerBonusLevel*/) override;
	virtual int getUseDuration();
private:
	ItemInstance bow;
	int fireCheckTick;
};

#endif /*NET_MINECRAFT_WORLD_ENTITY_MONSTER__Skeleton_H__*/
