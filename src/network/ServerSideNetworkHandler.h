#ifndef _MINECRAFT_NETWORK_SERVERSIDENETWORKHANDLER_H_
#define _MINECRAFT_NETWORK_SERVERSIDENETWORKHANDLER_H_


#include "NetEventCallback.h"
#include "../raknet/RakNetTypes.h"
#include "../world/level/LevelListener.h"
#include <vector>

class Minecraft;
class Level;
class IRakNetInstance;
class Packet;
class Player;

class ServerSideNetworkHandler : public NetEventCallback, public LevelListener
{
public:
	ServerSideNetworkHandler(Minecraft* minecraft, IRakNetInstance* raknetInstance);
	virtual ~ServerSideNetworkHandler();

	virtual void levelGenerated(Level* level) override;

	virtual void tileChanged(int x, int y, int z) override;
	virtual void tileBrightnessChanged(int x, int y, int z) override { /* do nothing */ }
	virtual Packet* getAddPacketFromEntity(Entity* entity);
	virtual void entityAdded(Entity* e) override;
	virtual void entityRemoved(Entity* e) override;
	virtual void levelEvent(Player* source, int type, int x, int y, int z, int data) override;
	virtual void tileEvent(int x, int y, int z, int b0, int b1) override;

	virtual void onNewClient(const RakNet::RakNetGUID& clientGuid) override;
	virtual void onDisconnect(const RakNet::RakNetGUID& guid) override;

	void onReady_ClientGeneration(const RakNet::RakNetGUID& source);
	void onReady_RequestedChunks(const RakNet::RakNetGUID& source);

	virtual void handle(const RakNet::RakNetGUID& source, LoginPacket* packet) override;
	virtual void handle(const RakNet::RakNetGUID& source, ReadyPacket* packet) override;
	virtual void handle(const RakNet::RakNetGUID& source, MovePlayerPacket* packet) override;
	//virtual void handle(const RakNet::RakNetGUID& source, PlaceBlockPacket* packet);
	virtual void handle(const RakNet::RakNetGUID& source, RemoveBlockPacket* packet) override;
	//virtual void handle(const RakNet::RakNetGUID& source, ExplodePacket* packet);
	virtual void handle(const RakNet::RakNetGUID& source, RequestChunkPacket* packet) override;
	virtual void handle(const RakNet::RakNetGUID& source, PlayerEquipmentPacket* packet) override;
	virtual void handle(const RakNet::RakNetGUID& source, PlayerArmorEquipmentPacket* packet) override;
	virtual void handle(const RakNet::RakNetGUID& source, SetHealthPacket* packet) override;
	//virtual void handle(const RakNet::RakNetGUID& source, TeleportEntityPacket* packet);
	virtual void handle(const RakNet::RakNetGUID& source, RidePacket* packet) override;
	virtual void handle(const RakNet::RakNetGUID& source, InteractPacket* packet) override;
	virtual void handle(const RakNet::RakNetGUID& source, AnimatePacket* packet) override;
	virtual void handle(const RakNet::RakNetGUID& source, UseItemPacket* packet) override;
	virtual void handle(const RakNet::RakNetGUID& source, EntityEventPacket* packet) override;
	virtual void handle(const RakNet::RakNetGUID& source, PlayerActionPacket* packet) override;
	virtual void handle(const RakNet::RakNetGUID& source, RespawnPacket* packet) override;
	virtual void handle(const RakNet::RakNetGUID& source, SendInventoryPacket* packet) override;
	virtual void handle(const RakNet::RakNetGUID& source, DropItemPacket* packet) override;
	virtual void handle(const RakNet::RakNetGUID& source, ContainerSetSlotPacket* packet) override;
	virtual void handle(const RakNet::RakNetGUID& source, ContainerClosePacket* packet) override;
	virtual void handle(const RakNet::RakNetGUID& source, SignUpdatePacket* packet) override;
	virtual void handle(const RakNet::RakNetGUID& source, ChatPacket* packet) override;

	bool allowsIncomingConnections() { return _allowIncoming; }
	void allowIncomingConnections(bool doAllow);

	Player* popPendingPlayer(const RakNet::RakNetGUID& source);

	void displayGameMessage(const std::string& message);
private:
	/**
	 * @brief Send packet to all players
	 */
	void redistributePacket(Packet* packet, const RakNet::RakNetGUID& fromPlayer);
	Player* getPlayer(const RakNet::RakNetGUID& source);

	Minecraft*					minecraft;
	Level*						level;
	IRakNetInstance*			raknetInstance;
	RakNet::RakPeerInterface*	rakPeer;

	std::vector<Player*> _pendingPlayers;
	bool _allowIncoming;
};

#endif
