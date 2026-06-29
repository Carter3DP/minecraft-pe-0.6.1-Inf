#ifndef NET_MINECRAFT_WORLD_LEVEL_TILE__WebTile_H__
#define NET_MINECRAFT_WORLD_LEVEL_TILE__WebTile_H__

//package net.minecraft.world.level.tile;

#include "../../entity/Entity.h"
#include "../../item/Item.h"
#include "../Level.h"
#include "../material/Material.h"
#include "../../phys/AABB.h"

class WebTile: public Tile
{
    typedef Tile super;
public:
    WebTile(int id, int tex)
	:	super(id, tex, Material::web)
	{
    }

	int getRenderLayer() override {
		return RENDERLAYER_ALPHATEST;
	}

    /*@Override*/
    void entityInside(Level* level, int x, int y, int z, Entity* entity) override {
        entity->makeStuckInWeb();
    }

    /*@Override*/
    bool isSolidRender() override {
        return false;
    }

    /*@Override*/
    AABB* getAABB(Level* level, int x, int y, int z) override {
        return NULL;
    }

    /*@Override*/
    int getRenderShape() override {
        return Tile::SHAPE_CROSS_TEXTURE;
    }

    bool blocksLight() {
        return false;
    }

    bool isCubeShaped() override {
        return false;
    }

    /*@Override*/
    int getResource(int data, Random* random) override {
        return Item::string->id;
    }
};

#endif /*NET_MINECRAFT_WORLD_LEVEL_TILE__WebTile_H__*/
