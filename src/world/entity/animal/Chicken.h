#ifndef NET_MINECRAFT_WORLD_ENTITY_ANIMAL__Chicken_H__
#define NET_MINECRAFT_WORLD_ENTITY_ANIMAL__Chicken_H__

//package net.minecraft.world.entity.animal;

#include "Animal.h"

class CompoundTag;
class Level;

class Chicken: public Animal
{
    typedef Animal super;
public:
    Chicken(Level* level);

	int getEntityTypeId() const override;

    /*@Override*/
    int getMaxHealth() override;

    void aiStep() override;

    void addAdditonalSaveData(CompoundTag* tag) override;
    void readAdditionalSaveData(CompoundTag* tag) override;
protected:
    void causeFallDamage(float distance) override;

    const char* getAmbientSound() override;
    std::string getHurtSound() override;
    std::string getDeathSound() override;

    //int getDeathLoot();

    //@Override
    void dropDeathLoot(/*bool wasKilledByPlayer, int playerBonusLevel*/) override;

    //@Override
    Animal* getBreedOffspring(Animal* target);
public:
	bool sheared;
	float flap;
	float flapSpeed;
	float oFlapSpeed, oFlap;
	float flapping;
	int eggTime;
};

#endif /*NET_MINECRAFT_WORLD_ENTITY_ANIMAL__Chicken_H__*/
