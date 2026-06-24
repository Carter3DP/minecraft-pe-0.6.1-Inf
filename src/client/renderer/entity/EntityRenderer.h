#ifndef NET_MINECRAFT_CLIENT_RENDERER_ENTITY__EntityRenderer_H__
#define NET_MINECRAFT_CLIENT_RENDERER_ENTITY__EntityRenderer_H__

//package net.minecraft.client.renderer.entity;

#include <string>
#include "../../model/HumanoidModel.h"

class Textures;
class Tesselator;
class EntityRenderDispatcher;
class Entity;
class AABB;
class Font;
class Tile;
class Level;

class EntityRenderer
{
protected:
	EntityRenderer();
public:
    virtual ~EntityRenderer() {}
	void init(EntityRenderDispatcher* entityRenderDispatcher);

	virtual void render(Entity* entity, double x, double y, double z, float rot, float a) = 0;
    static void render(const AABB& bb, double xo, double yo, double zo);
    static void renderFlat(const AABB& bb);
	void renderShadow(Entity* e, double x, double y, double z, float pow, float a);
	void renderTileShadow(Tile* tt, double x, double y, double z, int xt, int yt, int zt, float pow, float r, double xo, double yo, double zo);
	void renderFlame(Entity* e, double x, double y, double z, float a);
	void postRender(Entity* entity, double x, double y, double z, float rot, float a);
	Level* getLevel();

	Font* getFont();

	virtual void onGraphicsReset() {}

protected:
	void bindTexture(const std::string& resourceName);

	float shadowRadius;
	float shadowStrength;
	static EntityRenderDispatcher* entityRenderDispatcher;
};

#endif /*NET_MINECRAFT_CLIENT_RENDERER_ENTITY__EntityRenderer_H__*/
