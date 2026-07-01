#ifndef _MINECRAFT_NETWORK_RAKNETINSTANCE_H_
#define _MINECRAFT_NETWORK_RAKNETINSTANCE_H_

#include <vector>
#include <string>
#include <map>
#include "../raknet/RakNetTypes.h"
#include "../raknet/RakString.h"

namespace RakNet
{
	class RakPeerInterface;
	class BitStream;
}

class Packet;
class NetEventCallback;

typedef struct PingedCompatibleServer
{
	PingedCompatibleServer()
	:	pingTime(0),
		prevpingTime(0),
		lastSeenTime(0),
		hasIcon(false),
		isClientHosted(false),
		isSaved(false),
		isSpecial(false)
	{
	}

	RakNet::RakString name;
	RakNet::SystemAddress address;
	RakNet::TimeMS pingTime;
	RakNet::TimeMS prevpingTime;
	RakNet::TimeMS lastSeenTime;
	std::string icon;
	bool hasIcon;
	bool isClientHosted;
	bool isSaved;
	bool isSpecial;
} PingedCompatibleServer;
typedef std::vector<PingedCompatibleServer> ServerList;

class IRakNetInstance
{
public:
    virtual ~IRakNetInstance() {}
    
	virtual bool host(const std::string& localName, int port, int maxConnections = 4) { return false; }
	virtual bool connect(const char* host, int port) { return false; }
	virtual void setIsLoggedIn(bool status) {}

	virtual void pingForHosts(int port) {}
	virtual void stopPingForHosts() {}
	virtual const ServerList& getServerList() { static ServerList l; return l; }
	virtual void clearServerList() {}
	virtual ServerList loadSavedServers() { return ServerList(); }
	virtual void saveServerList(const ServerList& servers) {}
	virtual void setServerStoragePath(const std::string& path) {}
    
	virtual void disconnect() {}
    
	virtual void announceServer(const std::string& localName) {}
    
	virtual RakNet::RakPeerInterface* getPeer() { return NULL; }
	virtual bool isMyLocalGuid(const RakNet::RakNetGUID& guid) { return true; }
    
	virtual void runEvents(NetEventCallback* callback) {}
    
	virtual void send(Packet& packet) {}
	virtual void send(const RakNet::RakNetGUID& guid, Packet& packet) {}
    
	// @attn: Those delete the packet
	virtual void send(Packet* packet) {}
	virtual void send(const RakNet::RakNetGUID& guid, Packet* packet) {}
    
	virtual bool isServer() { return true; }
    virtual bool isProbablyBroken() { return false; }
	virtual void resetIsBroken() {}
};

class RakNetInstance: public IRakNetInstance
{
public:

	RakNetInstance();
	virtual ~RakNetInstance();

	bool host(const std::string& localName, int port, int maxConnections = 4) override;
	bool connect(const char* host, int port) override;
	void setIsLoggedIn(bool status) override;

	void pingForHosts(int basePort) override;
	void stopPingForHosts() override;
	const ServerList& getServerList() override;
	void clearServerList() override;
	ServerList loadSavedServers() override;
	void saveServerList(const ServerList& servers) override;
	void setServerStoragePath(const std::string& path) override;

	void disconnect() override;

	void announceServer(const std::string& localName) override;

	RakNet::RakPeerInterface* getPeer() override;
	bool isMyLocalGuid(const RakNet::RakNetGUID& guid) override;

	void runEvents(NetEventCallback* callback) override;

	void send(Packet& packet) override;
	void send(const RakNet::RakNetGUID& guid, Packet& packet) override;

	// @attn: Those delete the packet
	void send(Packet* packet) override;
	void send(const RakNet::RakNetGUID& guid, Packet* packet) override;

	bool isServer() override { return _isServer; }
    bool isProbablyBroken() override;
	void resetIsBroken() override;

#ifdef _DEBUG
	const char* getPacketName(int packetId);
#else
	const char* getPacketName(int packetId) { return ""; }
#endif

private:
	int handleUnconnectedPong(const RakNet::RakString& data, const RakNet::Packet*, const char* appid, bool insertAtBeginning, const RakNet::TimeMS ping);
	void loadServerIcon();
	std::string getServerStorageBasePath() const;
	std::string getServerIconPath(const RakNet::SystemAddress& address) const;
	bool writeServerIcon(const RakNet::SystemAddress& address, const std::vector<unsigned char>& iconData, std::string& outPath) const;
	void requestServerIcon(const RakNet::SystemAddress& address);
	void sendServerIconChunks(const RakNet::SystemAddress& address);
	void handleAdvertiseSystem(const RakNet::Packet* packet, RakNet::BitStream& bitStream);
	void handleServerIconChunk(const RakNet::SystemAddress& address, RakNet::BitStream& bitStream);

	struct PendingServerIcon
	{
		unsigned int totalBytes;
		unsigned int totalChunks;
		unsigned int receivedChunks;
		std::vector<unsigned char> data;
		std::vector<bool> received;
	};

	RakNet::RakPeerInterface*	rakPeer;
	RakNet::RakNetGUID			serverGuid;

	ServerList		availableServers;
	bool			isPingingForServers;
	int				pingPort;
	RakNet::TimeMS	lastPingTime;

	bool _isServer;
	bool _isLoggedIn;
	bool _hasServerIcon;
	std::string _serverIconBase64;
	std::vector<unsigned char> _serverIconData;
	std::string _serverStoragePath;
	std::map<std::string, PendingServerIcon> _pendingServerIcons;
};

#endif /*_MINECRAFT_NETWORK_RAKNETINSTANCE_H_*/
