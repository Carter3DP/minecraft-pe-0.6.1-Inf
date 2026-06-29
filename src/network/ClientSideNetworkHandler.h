#ifndef _MINECRAFT_NETWORK_CLIENTSIDENETWORKHANDLER_H_
#define _MINECRAFT_NETWORK_CLIENTSIDENETWORKHANDLER_H_


#include "NetEventCallback.h"
#include "../raknet/RakNetTypes.h"
#include "../world/level/LevelConstants.h"
#include "../world/level/ChunkPos.h"

#include <vector>
#include <set>

class Minecraft;
class Level;
class IRakNetInstance;

struct SBufferedBlockUpdate
{
	int x, z;
	unsigned char y;
	unsigned char blockId;
	unsigned char blockData;
	bool setData;
};
typedef std::vector<SBufferedBlockUpdate> BlockUpdateList;

typedef struct IntPair {
    int x, y;
} IntPair;

class ClientSideNetworkHandler : public NetEventCallback
{
public:
	ClientSideNetworkHandler(Minecraft* minecraft, IRakNetInstance* raknetInstance);
	virtual ~ClientSideNetworkHandler();

	virtual void levelGenerated(Level* level) override;
	void requestChunksAroundPlayer();

	virtual void onConnect(const RakNet::RakNetGUID& hostGuid) override;
	virtual void onUnableToConnect() override;
	virtual void onDisconnect(const RakNet::RakNetGUID& guid) override;

	virtual void handle(const RakNet::RakNetGUID& source, LoginStatusPacket* packet) override;
	virtual void handle(const RakNet::RakNetGUID& source, StartGamePacket* packet) override;
	virtual void handle(const RakNet::RakNetGUID& source, MessagePacket* packet) override;
	virtual void handle(const RakNet::RakNetGUID& source, SetTimePacket* packet) override;
	virtual void handle(const RakNet::RakNetGUID& source, AddItemEntityPacket* packet) override;
	virtual void handle(const RakNet::RakNetGUID& source, AddPaintingPacket* packet) override;
	virtual void handle(const RakNet::RakNetGUID& source, TakeItemEntityPacket* packet) override;
	virtual void handle(const RakNet::RakNetGUID& source, AddEntityPacket* packet) override;
	virtual void handle(const RakNet::RakNetGUID& source, AddMobPacket* packet) override;
	virtual void handle(const RakNet::RakNetGUID& source, AddPlayerPacket* packet) override;
	virtual void handle(const RakNet::RakNetGUID& source, RemoveEntityPacket* packet) override;
	virtual void handle(const RakNet::RakNetGUID& source, RemovePlayerPacket* packet) override;
	virtual void handle(const RakNet::RakNetGUID& source, RidePacket* packet) override;
	virtual void handle(const RakNet::RakNetGUID& source, MovePlayerPacket* packet) override;
	virtual void handle(const RakNet::RakNetGUID& source, MoveEntityPacket* packet) override;
	virtual void handle(const RakNet::RakNetGUID& source, UpdateBlockPacket* packet) override;
	virtual void handle(const RakNet::RakNetGUID& source, ExplodePacket* packet) override;
	virtual void handle(const RakNet::RakNetGUID& source, LevelEventPacket* packet) override;
	virtual void handle(const RakNet::RakNetGUID& source, TileEventPacket* packet) override;
	virtual void handle(const RakNet::RakNetGUID& source, EntityEventPacket* packet) override;
	virtual void handle(const RakNet::RakNetGUID& source, ChunkDataPacket* packet) override;
	virtual void handle(const RakNet::RakNetGUID& source, PlayerEquipmentPacket* packet) override;
	virtual void handle(const RakNet::RakNetGUID& source, PlayerArmorEquipmentPacket* packet) override;
	virtual void handle(const RakNet::RakNetGUID& source, InteractPacket* packet) override;
	virtual void handle(const RakNet::RakNetGUID& source, SetEntityDataPacket* packet) override;
	virtual void handle(const RakNet::RakNetGUID& source, SetEntityMotionPacket* packet) override;
	virtual void handle(const RakNet::RakNetGUID& source, SetHealthPacket* packet) override;
	virtual void handle(const RakNet::RakNetGUID& source, SetSpawnPositionPacket* packet) override;
	virtual void handle(const RakNet::RakNetGUID& source, AnimatePacket* packet) override;
	virtual void handle(const RakNet::RakNetGUID& source, UseItemPacket* packet) override;
	virtual void handle(const RakNet::RakNetGUID& source, HurtArmorPacket* packet) override;
	virtual void handle(const RakNet::RakNetGUID& source, RespawnPacket* packet) override;
	virtual void handle(const RakNet::RakNetGUID& source, ContainerOpenPacket* packet) override;
	virtual void handle(const RakNet::RakNetGUID& source, ContainerClosePacket* packet) override;
	virtual void handle(const RakNet::RakNetGUID& source, ContainerSetContentPacket* packet) override;
	virtual void handle(const RakNet::RakNetGUID& source, ContainerSetSlotPacket* packet) override;
	virtual void handle(const RakNet::RakNetGUID& source, ContainerSetDataPacket* packet) override;
	virtual void handle(const RakNet::RakNetGUID& source, ChatPacket* packet) override;
	virtual void handle(const RakNet::RakNetGUID& source, AdventureSettingsPacket* packet) override;
	virtual void handle(const RakNet::RakNetGUID& source, SignUpdatePacket* packet) override;
private:

	void requestNextChunk();
    void arrangeRequestChunkOrder();

	bool isChunkLoaded(int x, int z);
	bool areAllChunksLoaded();
	void clearChunksLoaded();
private:

	Minecraft* minecraft;
	Level* level;
	IRakNetInstance* raknetInstance;
	RakNet::RakPeerInterface* rakPeer;

	RakNet::RakNetGUID serverGuid;

	BlockUpdateList	bufferedBlockUpdates;
	static const int RequestChunkRadius = 4;
    
    int requestNextChunkIndex;
    std::vector<IntPair> requestNextChunkIndexList;
	std::set<ChunkPos> chunksLoaded;
	std::set<ChunkPos> chunksRequested;
	bool initialReadySent;
};

#endif
