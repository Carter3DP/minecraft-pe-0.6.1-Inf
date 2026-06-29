#ifndef NET_MINECRAFT_CLIENT_RENDERER_ENTITY__TntRenderer_H__
#define NET_MINECRAFT_CLIENT_RENDERER_ENTITY__TntRenderer_H__

//package net.minecraft.client.renderer.entity;

#include "EntityRenderer.h"
#include "../TileRenderer.h"

class TntRenderer: public EntityRenderer
{
public:
    TntRenderer();
	void render(Entity* tnt_, double x, double y, double z, float rot, float a) override;

	TileRenderer tileRenderer;
};

#endif /*NET_MINECRAFT_CLIENT_RENDERER_ENTITY__TntRenderer_H__*/
