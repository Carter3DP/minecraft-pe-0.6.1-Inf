#ifndef NET_MINECRAFT_WORLD_ENTITY__FlyingMob_H__
#define NET_MINECRAFT_WORLD_ENTITY__FlyingMob_H__

//package net.minecraft.world.entity;

#include "Mob.h"

class Level;


class FlyingMob: public Mob
{
    typedef Mob super;
public:
    FlyingMob(Level* level);

    void travel(double xa, double ya) override;

    bool onLadder() override;
protected:
    void causeFallDamage(float distance) override;
};

#endif /*NET_MINECRAFT_WORLD_ENTITY__FlyingMob_H__*/
