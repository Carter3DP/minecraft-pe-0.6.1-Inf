#ifndef ServerPlayer_H__
#define ServerPlayer_H__

#include "../world/entity/player/Player.h"
#include "../world/inventory/BaseContainerMenu.h"

class Minecraft;
class FurnaceTileEntity;
class ItemInstance;
class FillingContainer;
class ChestTileEntity;

class ServerPlayer: public Player,
					public IContainerListener
{
    typedef Player super;
public:
    ServerPlayer(Minecraft* minecraft, Level* level);

	~ServerPlayer();

	void aiStep() override;
	void tick() override;
	void take(Entity* e, int orgCount) override;

	void hurtArmor(int dmg) override;

	void displayClientMessage(const std::string& messageId) override;

	void openContainer(ChestTileEntity* furnace) override;
	void openFurnace(FurnaceTileEntity* furnace) override;
	void closeContainer() override;
	void doCloseContainer();

	bool hasResource( int id ) override;
	//
	// IContainerListener
	//
	void setContainerData(BaseContainerMenu* menu, int id, int value) override;
	void slotChanged(BaseContainerMenu* menu, int slot, const ItemInstance& item, bool isResultSlot) override;
	void refreshContainer(BaseContainerMenu* menu, const std::vector<ItemInstance>& items) override;

	virtual void stopSleepInBed(bool forcefulWakeUp, bool updateLevelList, bool saveRespawnPoint) override;

	void completeUsingItem() override;
private:
	void nextContainerCounter();
	void setContainerMenu( BaseContainerMenu* menu );

	Minecraft* _mc;
    int _prevHealth;
	int _containerCounter;
};

#endif /*ServerPlayer_H__*/
