#ifndef NET_MINECRAFT_WORLD_ENTITY_MONSTER__PigZombie_H__
#define NET_MINECRAFT_WORLD_ENTITY_MONSTER__PigZombie_H__

#include "Zombie.h"
class ItemInstance;
class PigZombie : public Zombie {
	typedef Zombie super;
public:
	PigZombie(Level* level);
	bool useNewAi() override;
	void tick() override;
	std::string getTexture() override;
	bool canSpawn() override;
	void addAdditonalSaveData(CompoundTag* entityTag) override;
	void readAdditionalSaveData(CompoundTag* tag) override;
	bool hurt(Entity* sourceEntity, int dmg) override;
	bool interact(Player* player) override;
	int getEntityTypeId() const override;
	virtual int getAttackTime() override;
	ItemInstance* getCarriedItem() override;
protected:
	Entity* findAttackTarget() override;
	const char* getAmbientSound() override;
	std::string getHurtSound() override;
	std::string getDeathSound() override;
	void dropDeathLoot() override;
	int getDeathLoot() override;
private:
	void alert(Entity* target);
	
private:
	int angerTime;
	int playAngrySoundIn;
	int stunedTime;
	ItemInstance weapon;
};

#endif /* NET_MINECRAFT_WORLD_ENTITY_MONSTER__PigZombie_H__ */
