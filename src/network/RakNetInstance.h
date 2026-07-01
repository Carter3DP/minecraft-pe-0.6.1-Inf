#ifndef _MINECRAFT_NETWORK_RAKNETINSTANCE_H_
#define _MINECRAFT_NETWORK_RAKNETINSTANCE_H_

#include <vector>
#include <string>
#include "../raknet/RakNetTypes.h"
#include "../raknet/RakString.h"

namespace RakNet
{
	class RakPeerInterface;
}

class Packet;
class NetEventCallback;

typedef struct PingedCompatibleServer
{
	RakNet::RakString name;
	RakNet::SystemAddress address;
	RakNet::TimeMS pingTime;
	RakNet::TimeMS prevpingTime;
	RakNet::TimeMS lastSeenTime;
	std::string icon;
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

	RakNet::RakPeerInterface*	rakPeer;
	RakNet::RakNetGUID			serverGuid;

	ServerList		availableServers;
	bool			isPingingForServers;
	int				pingPort;
	RakNet::TimeMS	lastPingTime;

	bool _isServer;
	bool _isLoggedIn;
};

#endif /*_MINECRAFT_NETWORK_RAKNETINSTANCE_H_*/
