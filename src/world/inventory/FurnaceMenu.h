#ifndef NET_MINECRAFT_WORLD_INVENTORY__FurnaceMenu_H__
#define NET_MINECRAFT_WORLD_INVENTORY__FurnaceMenu_H__

//package net.minecraft.world.inventory;

#include "BaseContainerMenu.h"
#include <vector>

class FurnaceTileEntity;
class ItemInstance;

class FurnaceMenu: public BaseContainerMenu
{
	typedef BaseContainerMenu super;
public:
    FurnaceMenu(FurnaceTileEntity* furnace);

    virtual ~FurnaceMenu() {}

    virtual void setSlot(int slot, ItemInstance* item) override;
	virtual void setData(int id, int value) override;
	virtual std::vector<ItemInstance> getItems() override;

	virtual void broadcastChanges() override;
	virtual void setListener(IContainerListener* listener) override;

	virtual bool tileEntityDestroyedIsInvalid( int tileEntityId ) override;

	FurnaceTileEntity* furnace;

	int furnaceTileEntityId;
private:
	int lastTickCount;
	int lastLitTime;
	int lastLitDuration;
};

#endif /*NET_MINECRAFT_WORLD_INVENTORY__FurnaceMenu_H__*/
