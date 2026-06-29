#ifndef NET_MINECRAFT_WORLD_ENTITY_ITEM__ItemEntity_H__
#define NET_MINECRAFT_WORLD_ENTITY_ITEM__ItemEntity_H__

//package net.minecraft.world.entity.item;

#include "../../item/ItemInstance.h"
#include "../../entity/Entity.h"
#include "../../../SharedConstants.h"

class Level;
class ItemInstance;

class ItemEntity: public Entity
{
	typedef Entity super;

    static const int LIFETIME;
public:
	ItemEntity(Level* level);
	ItemEntity(Level* level, double x, double y, double z, const ItemInstance& item);
	~ItemEntity();

    void tick() override;
    bool isInWater() override;
    bool hurt(Entity* source, int damage) override;
    void playerTouch(Player* player) override;
	bool isItemEntity() override;
	int getEntityTypeId() const override;
	int getLifeTime() const;
protected:
	void burn(int dmg) override;

	void addAdditonalSaveData(CompoundTag* entityTag) override;
	void readAdditionalSaveData(CompoundTag* tag) override;
private:
    bool checkInTile(double x, double y, double z);

public:
	ItemInstance item;
	int age;
	int throwTime;
	float bobOffs;
private:
	int tickCount;
	int health;
	int lifeTime;
};

#endif /*NET_MINECRAFT_WORLD_ENTITY_ITEM__ItemEntity_H__*/
