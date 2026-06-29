#ifndef NET_MINECRAFT_WORLD_ENTITY_MONSTER__Spider_H__
#define NET_MINECRAFT_WORLD_ENTITY_MONSTER__Spider_H__

//package net.minecraft.world.entity->monster;

#include "Monster.h"
#include <string>

class Level;
class Entity;

class Spider: public Monster
{
    typedef Monster super;
	typedef SynchedEntityData::TypeChar DataFlagIdType;
    static const int DATA_FLAGS_ID = 16;
public:
    Spider(Level* level);

    /*@Override*/
    void aiStep() override;
    /*@Override*/
    void tick() override;

    /*@Override*/
    int getMaxHealth() override;

    /**
     * The the spiders act as if they're always on a ladder, which enables them
     * to climb walls.
     */
    /*@Override*/ //@todo
    bool onLadder() override;

    /*@Override*/
    void makeStuckInWeb() override;

    float getModelScale();

	bool isClimbing();
	void setClimbing(bool value);

	virtual int getEntityTypeId() const override;

protected:
    /*@Override*/
    bool makeStepSound();

    Entity* findAttackTarget() override;

    const char* getAmbientSound() override;
    std::string getHurtSound() override;
    std::string getDeathSound() override;

    void checkHurtTarget(Entity* target, float d) override;
	int fireCheckTick;
    int getDeathLoot() override;
    /*@Override*/
    //void dropDeathLoot();
};

#endif /*NET_MINECRAFT_WORLD_ENTITY_MONSTER__Spider_H__*/
