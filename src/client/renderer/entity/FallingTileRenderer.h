#ifndef NET_MINECRAFT_CLIENT_RENDERER_ENTITY__FallingTileRenderer_H__
#define NET_MINECRAFT_CLIENT_RENDERER_ENTITY__FallingTileRenderer_H__

//package net.minecraft.client.renderer.entity;

#include "EntityRenderer.h"

class TileRenderer;

class FallingTileRenderer: public EntityRenderer
{
    typedef EntityRenderer super;
public:
    FallingTileRenderer();
    ~FallingTileRenderer();

    void render(Entity* e, double x, double y, double z, float rot, float a);
private:
    TileRenderer* tileRenderer;
};

#endif /*NET_MINECRAFT_CLIENT_RENDERER_ENTITY__FallingTileRenderer_H__*/
