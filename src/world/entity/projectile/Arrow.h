#ifndef NET_MINECRAFT_WORLD_ENTITY_PROJECTILE__Arrow_H__
#define NET_MINECRAFT_WORLD_ENTITY_PROJECTILE__Arrow_H__

//package net.minecraft.world.entity.projectile;

#include "../Entity.h"

class Level;
class Mob;
class CompoundTag;

class Arrow: public Entity
{
    typedef Entity super;
    // base damage, multiplied with velocity
    static const float ARROW_BASE_DAMAGE;

public:
    Arrow(Level* level);
    Arrow(Level* level, double x, double y, double z);
    Arrow(Level* level, Mob* mob, float power);

    void _init();

    void shoot(double xd, double yd, double zd, float pow, float uncertainty);
    
	void lerpMotion(double xd, double yd, double zd) override;

	void tick() override;

	int getEntityTypeId() const override;

    void addAdditonalSaveData(CompoundTag* tag) override;
    void readAdditionalSaveData(CompoundTag* tag) override;

    void playerTouch(Player* player) override;

    float getShadowHeightOffs() override;

	int getAuxData() override;
public:
    bool playerArrow;
    int shakeTime;
    int ownerId;
    bool critArrow;
private:
    int xTile;
    int yTile;
    int zTile;
    int lastTile;
    int lastData;
    bool inGround;
    int life;
    int flightTime;
};

#endif /*NET_MINECRAFT_WORLD_ENTITY_PROJECTILE__Arrow_H__*/
