#ifndef NET_MINECRAFT_CLIENT_GAMEMODE__SurvivalMode_H__
#define NET_MINECRAFT_CLIENT_GAMEMODE__SurvivalMode_H__

#include "GameMode.h"

class Abilities;
class Minecraft;

class SurvivalMode: public GameMode
{
	typedef GameMode super;
public:
	SurvivalMode(Minecraft* minecraft);

	bool destroyBlock(int x, int y, int z, int face) override;
	void startDestroyBlock(int x, int y, int z, int face) override;
	void continueDestroyBlock(int x, int y, int z, int face) override;
	void stopDestroyBlock() override;

	bool canHurtPlayer() override { return true; }

	bool isSurvivalType() override { return true; }

	void initAbilities( Abilities& abilities ) override;
private:
	int xDestroyBlock;
	int yDestroyBlock;
	int zDestroyBlock;
};

#endif /*NET_MINECRAFT_CLIENT_GAMEMODE__SurvivalMode_H__*/
