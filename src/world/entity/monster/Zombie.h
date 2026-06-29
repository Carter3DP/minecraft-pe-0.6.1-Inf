#ifndef NET_MINECRAFT_WORLD_ENTITY_MONSTER__Zombie_H__
#define NET_MINECRAFT_WORLD_ENTITY_MONSTER__Zombie_H__

//package net.minecraft.world.entity->monster;

#include "Monster.h"
#include <string>

class Level;

class Zombie: public Monster
{
	typedef Monster super;
public:
    Zombie(Level* level);

	~Zombie();

    /*@Override*/
    int getMaxHealth() override;

    void aiStep() override;
	virtual int getEntityTypeId() const override;
	void setUseNewAi(bool use);
	virtual void die(Entity* source) override;
	virtual int getAttackDamage(Entity* target) override;
protected:
	/*@Override*/
	int getArmorValue() override;

	const char* getAmbientSound() override;
    std::string getHurtSound() override;
    std::string getDeathSound() override;
	
	//@todo
    int getDeathLoot() override;

	virtual bool useNewAi() override;

	int fireCheckTick;
	bool _useNewAi;
};

#endif /*NET_MINECRAFT_WORLD_ENTITY_MONSTER__Zombie_H__*/
