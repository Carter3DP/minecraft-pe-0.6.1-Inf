#ifndef NET_MINECRAFT_WORLD_ENTITY_ANIMAL__Cow_H__
#define NET_MINECRAFT_WORLD_ENTITY_ANIMAL__Cow_H__

//package net.minecraft.world.entity.animal;

#include "Animal.h"

class Level;
class Player;
class CompoundTag;

class Cow: public Animal
{
	typedef Animal super;
public:
    Cow(Level* level);

	int getEntityTypeId() const override;

    /*@Override*/
    int getMaxHealth() override;

    void addAdditonalSaveData(CompoundTag* tag) override;
    void readAdditionalSaveData(CompoundTag* tag) override;

    bool interact(Player* player) override;
protected:
    const char* getAmbientSound() override;
    std::string getHurtSound() override;
    std::string getDeathSound() override;

    float getSoundVolume() override;

    int getDeathLoot() override;
    /*@Override*/
    void dropDeathLoot(/*bool wasKilledByPlayer, int playerBonusLevel*/) override;

    /*@Override*/
    Animal* getBreedOffspring(Animal* target);
};

#endif /*NET_MINECRAFT_WORLD_ENTITY_ANIMAL__Cow_H__*/
