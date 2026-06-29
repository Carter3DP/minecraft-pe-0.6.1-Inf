#ifndef NET_MINECRAFT_CLIENT_PLAYER__LocalPlayer_H__
#define NET_MINECRAFT_CLIENT_PLAYER__LocalPlayer_H__

//package net.minecraft.client.player;

#include "input/IMoveInput.h"
#include "../../util/SmoothFloat.h"
#include "../../world/entity/player/Player.h"

class Minecraft;
class Stat;
class CompoundTag;

class LocalPlayer: public Player
{
	typedef Player super;
public:
	LocalPlayer(Minecraft* minecraft, Level* level, const std::string& username, int dimension, bool isCreative);
	~LocalPlayer();

	void _init();
	virtual void reset() override;

	void tick() override;

	float yBob, xBob;	// shredder added from b1.8/4j for the hand swaying animation
	float yBobO, xBobO; // shredder added from b1.8/4j for the hand swaying animation

    void move(double xa, double ya, double za) override;

    void aiStep() override;
    void updateAi() override;

	void setKey(int eventKey, bool eventKeyState);
    void releaseAllKeys();

    void addAdditonalSaveData(CompoundTag* entityTag) override;
    void readAdditionalSaveData(CompoundTag* entityTag) override;

    void closeContainer() override;

	void drop(ItemInstance* item, bool randomly) override;
    void take(Entity* e, int orgCount) override;

	void startCrafting(int x, int y, int z, int tableSize) override;
	void startStonecutting(int x, int y, int z) override;

	void openContainer(ChestTileEntity* container) override;
	void openFurnace(FurnaceTileEntity* e) override;

    bool isSneaking() override;

    void actuallyHurt(int dmg) override;
    void hurtTo(int newHealth);
	void die(Entity* source) override;

    void respawn();

    void animateRespawn() override {}
	float getFieldOfViewModifier();
	void chat(const std::string& message) {}
    void displayClientMessage(const std::string& messageId) override;

    void awardStat(Stat* stat, int count) {
        //minecraft->stats.award(stat, count);
        //minecraft->achievementPopup.popup("Achievement get!", stat.name);
    }
	void causeFallDamage( float distance ) override;

	virtual int startSleepInBed(int x, int y, int z) override;
	virtual void stopSleepInBed(bool forcefulWakeUp, bool updateLevelList, bool saveRespawnPoint) override;

	void swing() override;
	virtual void openTextEdit( TileEntity* tileEntity ) override;
	virtual float getWalkingSpeedModifier() override;
private:
	void calculateFlight(double xa, double ya, double za);
	bool isSolidTile(int x, int y, int z);
	void updateArmorTypeHash();
public:
	IMoveInput* input;
	bool autoJumpEnabled;
protected:
	Minecraft* minecraft;
	int jumpTriggerTime;
	int ascendTriggerTime;
	int descendTriggerTime;
	bool ascending, descending;
private:
    // local player fly
    // -----------------------
    double flyX, flyY, flyZ;

    // smooth camera settings
    SmoothFloat smoothFlyX;
    SmoothFloat smoothFlyY;
    SmoothFloat smoothFlyZ;

	int autoJumpTime;

	int sentInventoryItemId;
	int sentInventoryItemData;

	int armorTypeHash;

	// sprinting
	bool sprinting;
	int  sprintDoubleTapTimer;
	bool prevForwardHeld;
public:
	void setSprinting(bool sprint) { sprinting = sprint; }
};

#endif /*NET_MINECRAFT_CLIENT_PLAYER__LocalPlayer_H__*/
