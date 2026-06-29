#ifndef NET_MINECRAFT_WORLD_ENTITY__Painting_H__
#define NET_MINECRAFT_WORLD_ENTITY__Painting_H__
#include "Motive.h"
#include "HangingEntity.h"
class Painting : public HangingEntity {
	typedef HangingEntity super;
public:
	Painting(Level* level);
	Painting(Level* level, int xTile, int yTile, int zTile, int dir);
	Painting(Level* level, int x, int y, int z, int dir, const std::string& motiveName);

	void setRandomMotive( int dir );

	void addAdditonalSaveData(CompoundTag* tag) override;
	void readAdditionalSaveData(CompoundTag* tag) override;

	int getWidth() override;
	int getHeight() override;

	void dropItem() override;
	int getEntityTypeId() const override;
	bool isPickable() override;
public:
	const Motive* motive;
};

#endif /* NET_MINECRAFT_WORLD_ENTITY__Painting_H__ */
