#ifndef NET_MINECRAFT_CLIENT_RENDERER_ENTITY__ChickenRenderer_H__
#define NET_MINECRAFT_CLIENT_RENDERER_ENTITY__ChickenRenderer_H__

//package net.minecraft.client.renderer.entity;

#include "MobRenderer.h"

class Mob;

class ChickenRenderer: public MobRenderer
{
    typedef MobRenderer super;
public:
    ChickenRenderer(Model* model, float shadow);

    void render(Entity* mob, double x, double y, double z, float rot, float a) override;

protected:
    float getBob(Mob* mob_, float a) override;
};

#endif /*NET_MINECRAFT_CLIENT_RENDERER_ENTITY__ChickenRenderer_H__*/
