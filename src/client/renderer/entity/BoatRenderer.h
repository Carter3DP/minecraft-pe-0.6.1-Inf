#ifndef NET_MINECRAFT_CLIENT_RENDERER_ENTITY__BoatRenderer_H__
#define NET_MINECRAFT_CLIENT_RENDERER_ENTITY__BoatRenderer_H__

//package net.minecraft.client.renderer.entity;

#include "EntityRenderer.h"
#include "../../model/BoatModel.h"

class EntityBoat;
class Model;
class Entity;

class BoatRenderer: public EntityRenderer
{
public:
    BoatRenderer(Model* model);

    void render(Entity* boat, double x, double y, double z, float rot, float a);
    void renderboat(EntityBoat* boat, double x, double y, double z, float rot, float a);

protected:
    Model* modelBoat;
};

#endif /*NET_MINECRAFT_CLIENT_RENDERER_ENTITY__ChickenRenderer_H__*/