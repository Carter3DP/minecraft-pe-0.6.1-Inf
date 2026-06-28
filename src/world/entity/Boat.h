#ifndef NET_MINECRAFT_WORLD_ENTITY__Boat_H__
#define NET_MINECRAFT_WORLD_ENTITY__Boat_H__

//package net.minecraft.world.entity;

#include <string>
#include "Entity.h"
#include "EntityTypes.h"
#include "SynchedEntityData.h"

#include "../phys/AABB.h"

class CompoundTag;

class Level;
class CompundTag;

class EntityBoat: public Entity{
    typedef Entity super;

    public:
        EntityBoat(Level* lvl);
        virtual ~EntityBoat();
        int boatCurrentDamage;
        int boatTimeSinceHit;
        int boatRockDirection;
        //virtual void playerTouch(Player* player) override;
        AABB getCollisionBox(Entity* e);
        AABB getBoundingBox();
        virtual bool isPushable() override;
        virtual double getMountedYOffset() override;
        virtual bool hurt(Entity* e, int var2) override;
        virtual void animateHurt() override;
        virtual bool canBeCollidedWith();
        virtual void lerpTo(double x, double y, double z, float yRot, float xRot, int steps) override;
        virtual void push(double xa, double ya, double za) override;
        virtual void tick() override;
        virtual void updateRiderPosition() override;
        virtual float getShadowHeightOffs() override;
        virtual bool interact(Player* player) override;
        virtual int getEntityTypeId();
    private:
        int boatPosRotationIncrements;
        double boatX, boatY, boatZ;
        float  boatYaw, boatPitch; 
        double vx, vy, vz;  //velocity x, y, z
    protected:
        bool canTriggerWalking();
        //void entityInit();
        virtual bool save(CompoundTag* entityTag) override;
        virtual bool load(CompoundTag* entityTag) override;
};

#endif