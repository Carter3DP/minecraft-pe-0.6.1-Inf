#ifndef NET_MINECRAFT_WORLD_ENTITY_ITEM__TripodCamera_H__
#define NET_MINECRAFT_WORLD_ENTITY_ITEM__TripodCamera_H__

#include "../Mob.h"

class TripodCamera: public Mob
{
	typedef Mob super;
public:
    TripodCamera(Level* level, Player* owner_, double x, double y, double z);

	void tick() override;

	bool isPickable() override;
	bool isPushable() override;

	// id == 0 -> not possible to create via serialization (yet)
	int getEntityTypeId() const override { return 0; }

	bool interact(Player* player) override;
	bool interactPreventDefault() override;

    float getShadowHeightOffs() override;
public:
	int life;
protected:
	Player* owner;
	bool activated;
};


#endif /*NET_MINECRAFT_WORLD_ENTITY_ITEM__TripodCamera_H__*/
