#ifndef NET_MINECRAFT_WORLD_ENTITY_ANIMAL__Pig_H__
#define NET_MINECRAFT_WORLD_ENTITY_ANIMAL__Pig_H__

//package net.minecraft.world.entity.animal;

#include "Animal.h"

class Player;

class Pig: public Animal
{
	typedef Animal super;
    static const int DATA_SADDLE_ID = 16;

public:
    Pig(Level* level);

	int getEntityTypeId() const override;

    //void addAdditonalSaveData(CompoundTag* tag) {
    //    super::addAdditonalSaveData(tag);
    //    tag->putBoolean("Saddle", hasSaddle());
    //}

    //void readAdditionalSaveData(CompoundTag* tag) {
    //    super::readAdditionalSaveData(tag);
    //    setSaddle(tag->getBoolean("Saddle"));
    //}

    bool interact(Player* player) override;

	int getMaxHealth() override;

    bool hasSaddle();
    void setSaddle(bool value);

protected:
    const char* getAmbientSound() override;
    std::string getHurtSound() override;
    std::string getDeathSound() override;

	int getDeathLoot() override;

    //@Override
    Animal* getBreedOffspring(Animal* target);
};

#endif /*NET_MINECRAFT_WORLD_ENTITY_ANIMAL__Pig_H__*/
