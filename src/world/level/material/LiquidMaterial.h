#ifndef NET_MINECRAFT_WORLD_LEVEL_MATERIAL__LiquidMaterial_H__
#define NET_MINECRAFT_WORLD_LEVEL_MATERIAL__LiquidMaterial_H__

//package net.minecraft.world.level.material;
#include "Material.h"

class LiquidMaterial: public Material
{
public:
	LiquidMaterial() {
		replaceable();
	}

	bool isLiquid() const override {
        return true;
    }
    
    bool blocksMotion() const override {
        return false;
    }    
    
    bool isSolid() const override {
        return false;
    }
};

#endif /*NET_MINECRAFT_WORLD_LEVEL_MATERIAL__LiquidMaterial_H__*/
