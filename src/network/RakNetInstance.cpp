#include "RakNetInstance.h"
#include "Packet.h"
#include "NetEventCallback.h"
#include "../raknet/RakPeerInterface.h"
#include "../raknet/BitStream.h"
#include "../raknet/MessageIdentifiers.h"
#include "../raknet/GetTime.h"
#include "../AppConstants.h"

#include "../platform/log.h"

#include <cstdio>
#include <cerrno>
#include <cstring>

#if defined(_WIN32)
#include <direct.h>
#else
#include <sys/stat.h>
#include <sys/types.h>
#endif

#if defined(__APPLE__)
#include <algorithm>
#include <cstring>
#include <ifaddrs.h>
#include <net/if.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string>
#include <vector>
#endif

#define APP_IDENTIFIER "MCCPP;" APP_VERSION_STRING ";"
#define APP_IDENTIFIER_MINECON "MCCPP;MINECON;"
#define SERVER_META_MARKER "\nMCCPPMETA;"
#define SERVER_ICON_REQUEST "MCCPPICONREQ"
#define SERVER_ICON_CHUNK "MCCPPICONCHUNK"

static const unsigned int SERVER_ICON_CHUNK_SIZE = 850;
static const unsigned int SERVER_ICON_MAX_BYTES = 256 * 1024;

static bool createDirectoryIfMissing(const std::string& path)
{
	if (path.empty())
		return true;
#if defined(_WIN32)
	return _mkdir(path.c_str()) == 0 || errno == EEXIST;
#else
	return mkdir(path.c_str(), 0755) == 0 || errno == EEXIST;
#endif
}

static bool createDirectoriesForFile(const std::string& path)
{
	size_t fpos = path.find_last_of("/\\");
	if (fpos == std::string::npos)
		return true;

	std::string dir = path.substr(0, fpos);
	std::string current;
	for (size_t i = 0; i <= dir.size(); ++i)
	{
		if (i == dir.size() || dir[i] == '/' || dir[i] == '\\')
		{
			if (!current.empty() && !createDirectoryIfMissing(current))
				return false;
		}
		if (i < dir.size())
			current.push_back(dir[i]);
	}
	return true;
}

static bool fileExists(const std::string& path)
{
	FILE* fp = fopen(path.c_str(), "rb");
	if (!fp)
		return false;
	fclose(fp);
	return true;
}

static std::string base64Encode(const std::vector<unsigned char>& data)
{
	static const char* alphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
	std::string out;
	int val = 0;
	int valb = -6;
	for (unsigned char c : data)
	{
		val = (val << 8) + c;
		valb += 8;
		while (valb >= 0)
		{
			out.push_back(alphabet[(val >> valb) & 0x3f]);
			valb -= 6;
		}
	}
	if (valb > -6)
		out.push_back(alphabet[((val << 8) >> (valb + 8)) & 0x3f]);
	while (out.size() % 4)
		out.push_back('=');
	return out;
}

static std::vector<unsigned char> base64Decode(const std::string& text)
{
	static const char* alphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
	int table[256];
	for (int i = 0; i < 256; ++i) table[i] = -1;
	for (int i = 0; i < 64; ++i) table[(unsigned char)alphabet[i]] = i;

	std::vector<unsigned char> out;
	int val = 0;
	int valb = -8;
	for (unsigned char c : text)
	{
		if (table[c] == -1)
			break;
		val = (val << 6) + table[c];
		valb += 6;
		if (valb >= 0)
		{
			out.push_back((unsigned char)((val >> valb) & 0xff));
			valb -= 8;
		}
	}
	return out;
}

static std::string escapeServerField(const std::string& value)
{
	std::string out;
	for (char c : value)
	{
		if (c == '\\' || c == '|' || c == '\n' || c == '\r')
			out.push_back('\\');
		if (c == '\n') out.push_back('n');
		else if (c == '\r') out.push_back('r');
		else out.push_back(c);
	}
	return out;
}

static std::string unescapeServerField(const std::string& value)
{
	std::string out;
	for (size_t i = 0; i < value.size(); ++i)
	{
		if (value[i] == '\\' && i + 1 < value.size())
		{
			char n = value[++i];
			if (n == 'n') out.push_back('\n');
			else if (n == 'r') out.push_back('\r');
			else out.push_back(n);
		}
		else
			out.push_back(value[i]);
	}
	return out;
}

#if defined(__APPLE__)
static std::vector<std::string> getLocalBroadcastAddresses()
{
	std::vector<std::string> addresses;
	struct ifaddrs* interfaces = NULL;

	if (getifaddrs(&interfaces) != 0)
		return addresses;

	for (struct ifaddrs* current = interfaces; current != NULL; current = current->ifa_next)
	{
		if (current->ifa_addr == NULL || current->ifa_broadaddr == NULL)
			continue;
		if (current->ifa_addr->sa_family != AF_INET)
			continue;
		if ((current->ifa_flags & IFF_UP) == 0 || (current->ifa_flags & IFF_LOOPBACK) != 0 || (current->ifa_flags & IFF_BROADCAST) == 0)
			continue;

		const struct sockaddr_in* broadcastAddress = (const struct sockaddr_in*)current->ifa_broadaddr;
		const char* address = inet_ntoa(broadcastAddress->sin_addr);
		if (address == NULL || address[0] == '\0')
			continue;
		if (strcmp(address, "0.0.0.0") == 0 || strcmp(address, "255.255.255.255") == 0)
			continue;

		if (std::find(addresses.begin(), addresses.end(), address) == addresses.end())
			addresses.push_back(address);
	}

	freeifaddrs(interfaces);
	return addresses;
}
#endif

RakNetInstance::RakNetInstance()
:	rakPeer(NULL),
	isPingingForServers(false),
	pingPort(0),
	lastPingTime(0),
	_isServer(false),
	_isLoggedIn(false),
	_hasServerIcon(false)
{
	rakPeer = RakNet::RakPeerInterface::GetInstance();
	rakPeer->SetTimeoutTime(20000, RakNet::UNASSIGNED_SYSTEM_ADDRESS);
    rakPeer->SetOccasionalPing(true);
}

RakNetInstance::~RakNetInstance()
{
	if (rakPeer)
	{
		rakPeer->Shutdown(100, 0);
		RakNet::RakPeerInterface::DestroyInstance(rakPeer);
		rakPeer = NULL;
	}
}

bool RakNetInstance::host(const std::string& localName, int port, int maxConnections /* = 4 */)
{
	if (rakPeer->IsActive())
	{
		rakPeer->Shutdown(500);
	}

	RakNet::SocketDescriptor socket(port, 0);
	socket.socketFamily = AF_INET;

	rakPeer->SetMaximumIncomingConnections(maxConnections);
	RakNet::StartupResult result = rakPeer->Startup(maxConnections, &socket, 1);

	_isServer = true;
	isPingingForServers = false;
	loadServerIcon();

	return (result == RakNet::RAKNET_STARTED);
}

void RakNetInstance::announceServer(const std::string& localName)
{
	if (_isServer && rakPeer->IsActive())
	{
		RakNet::RakString connectionData;

#if defined(MINECON)
		connectionData += APP_IDENTIFIER_MINECON;
#else
		connectionData += APP_IDENTIFIER;
#endif
		connectionData += localName.c_str();
		connectionData += SERVER_META_MARKER;
#if defined(STANDALONE_SERVER)
		connectionData += "type=standalone;";
#else
		connectionData += "type=client;";
#endif
		connectionData += _hasServerIcon ? "icon=1;" : "icon=0;";
		LOGI("[ServerIcon] Announcing server '%s' with icon=%s, rawBytes=%u, encodedBytes=%u, offlineResponseBytes=%u\n",
			localName.c_str(), _hasServerIcon ? "yes" : "no", (unsigned int)_serverIconData.size(), (unsigned int)_serverIconBase64.size(), (unsigned int)connectionData.GetLength());
		if (connectionData.GetLength() > 1200)
		{
			LOGW("[ServerIcon] Warning: offline response is %u bytes. Discovery metadata is unexpectedly large.\n",
				(unsigned int)connectionData.GetLength());
		}

		RakNet::BitStream bitStream;
		bitStream.Write(connectionData);
		rakPeer->SetOfflinePingResponse((const char*)bitStream.GetData(), bitStream.GetNumberOfBytesUsed());
	}
}

bool RakNetInstance::connect(const char* host, int port)
{
	_isLoggedIn = false;
	RakNet::StartupResult result;

	RakNet::SocketDescriptor socket(0, 0);
	socket.socketFamily = AF_INET;

	if (rakPeer->IsActive())
	{
		rakPeer->Shutdown(500);
	}

	result = rakPeer->Startup(4, &socket, 1);

	_isServer = false;
	isPingingForServers = false;

	if (result == RakNet::RAKNET_STARTED)
	{
		RakNet::ConnectionAttemptResult connectResult = rakPeer->Connect(host, port, NULL, 0, NULL, 0, 12, 500, 0);

		return (connectResult == RakNet::CONNECTION_ATTEMPT_STARTED);
	}

	return false;
}

void RakNetInstance::disconnect()
{
	if (rakPeer->IsActive())
	{
		rakPeer->Shutdown(500);
	}
	_isLoggedIn = false;
	_isServer = false;
	isPingingForServers = false;
}

void RakNetInstance::pingForHosts(int basePort)
{
	if (!rakPeer->IsActive())
	{
		RakNet::SocketDescriptor socket(0, 0);
		rakPeer->Startup(4, &socket, 1);
	}

	isPingingForServers = true;
	pingPort = basePort;
	lastPingTime = RakNet::GetTimeMS();

#if defined(__APPLE__)
	std::vector<std::string> broadcastAddresses = getLocalBroadcastAddresses();
#endif
	for (int i = 0; i < 4; ++i)
	{
		rakPeer->Ping("255.255.255.255", basePort + i, true);
#if defined(__APPLE__)
		for (unsigned int j = 0; j < broadcastAddresses.size(); ++j)
			rakPeer->Ping(broadcastAddresses[j].c_str(), basePort + i, true);
#endif
	}
}

void RakNetInstance::stopPingForHosts()
{
	if (isPingingForServers)
	{
		rakPeer->Shutdown(0);
		isPingingForServers = false;
	}
}

const ServerList& RakNetInstance::getServerList()
{
	return availableServers;
}

void RakNetInstance::clearServerList()
{
	availableServers.clear();
	/*
	for (int i = 0; i < 20; ++i) {
		PingedCompatibleServer ps;
		ps.isSpecial = false;
		ps.name = "Fake-Real";
		ps.address.FromString("192.168.1.236|19132");
		availableServers.push_back(ps);
	}
	*/
}

void RakNetInstance::setServerStoragePath(const std::string& path)
{
	_serverStoragePath = path;
}

std::string RakNetInstance::getServerStorageBasePath() const
{
	std::string base = _serverStoragePath.empty() ? "." : _serverStoragePath;
	return base + "/games/com.mojang";
}

std::string RakNetInstance::getServerIconPath(const RakNet::SystemAddress& address) const
{
	std::string ip = address.ToString(false);
	for (size_t i = 0; i < ip.size(); ++i)
		if (ip[i] == ':' || ip[i] == '\\' || ip[i] == '/')
			ip[i] = '_';

	return getServerStorageBasePath() + "/servers/" + ip + "/icon.png";
}

bool RakNetInstance::writeServerIcon(const RakNet::SystemAddress& address, const std::vector<unsigned char>& iconData, std::string& outPath) const
{
	if (iconData.empty())
		return false;

	outPath = getServerIconPath(address);
	if (!createDirectoriesForFile(outPath))
		return false;

	FILE* fp = fopen(outPath.c_str(), "wb");
	if (!fp)
		return false;
	fwrite(iconData.data(), 1, iconData.size(), fp);
	fclose(fp);
	return true;
}

void RakNetInstance::loadServerIcon()
{
	_hasServerIcon = false;
	_serverIconBase64.clear();
	_serverIconData.clear();

	LOGI("[ServerIcon] Looking for server icon at icon.png relative to the current working directory\n");
	FILE* fp = fopen("icon.png", "rb");
	if (!fp)
	{
		LOGW("[ServerIcon] No icon.png loaded: fopen failed with errno=%d (%s)\n", errno, strerror(errno));
		return;
	}

	if (fseek(fp, 0, SEEK_END) != 0)
	{
		LOGW("[ServerIcon] No icon.png loaded: failed to seek to end, errno=%d (%s)\n", errno, strerror(errno));
		fclose(fp);
		return;
	}
	long size = ftell(fp);
	if (size < 0)
	{
		LOGW("[ServerIcon] No icon.png loaded: ftell failed, errno=%d (%s)\n", errno, strerror(errno));
		fclose(fp);
		return;
	}
	if (fseek(fp, 0, SEEK_SET) != 0)
	{
		LOGW("[ServerIcon] No icon.png loaded: failed to seek to start, errno=%d (%s)\n", errno, strerror(errno));
		fclose(fp);
		return;
	}
	if (size <= 0)
	{
		LOGW("[ServerIcon] No icon.png loaded: file is empty (%ld bytes)\n", size);
		fclose(fp);
		return;
	}

	std::vector<unsigned char> data((size_t)size);
	size_t read = fread(data.data(), 1, data.size(), fp);
	if (read != data.size())
	{
		LOGW("[ServerIcon] No icon.png loaded: read %u of %u bytes, ferror=%d, errno=%d (%s)\n",
			(unsigned int)read, (unsigned int)data.size(), ferror(fp), errno, strerror(errno));
		fclose(fp);
		return;
	}
	fclose(fp);

	_serverIconData = data;
	_serverIconBase64 = base64Encode(data);
	_hasServerIcon = !_serverIconData.empty();
	if (_hasServerIcon)
		LOGI("[ServerIcon] Loaded icon.png successfully: rawBytes=%u, encodedBytes=%u\n",
			(unsigned int)data.size(), (unsigned int)_serverIconBase64.size());
	else
		LOGW("[ServerIcon] No icon.png loaded: file data was empty after reading\n");
}

void RakNetInstance::requestServerIcon(const RakNet::SystemAddress& address)
{
	std::string key = address.ToString(true);
	if (_pendingServerIcons.find(key) != _pendingServerIcons.end())
		return;

	PendingServerIcon pending;
	pending.totalBytes = 0;
	pending.totalChunks = 0;
	pending.receivedChunks = 0;
	_pendingServerIcons[key] = pending;

	RakNet::BitStream bitStream;
	bitStream.Write(RakNet::RakString(SERVER_ICON_REQUEST));
	bool sent = rakPeer->AdvertiseSystem(address.ToString(false), address.GetPort(), (const char*)bitStream.GetData(), bitStream.GetNumberOfBytesUsed());
	LOGI("[ServerIcon] Client requested icon from %s: sent=%s\n",
		address.ToString(true), sent ? "yes" : "no");
}

void RakNetInstance::sendServerIconChunks(const RakNet::SystemAddress& address)
{
	if (!_hasServerIcon || _serverIconData.empty())
	{
		LOGW("[ServerIcon] Server received icon request from %s but no icon is loaded\n",
			address.ToString(true));
		return;
	}

	unsigned int totalBytes = (unsigned int)_serverIconData.size();
	unsigned int totalChunks = (totalBytes + SERVER_ICON_CHUNK_SIZE - 1) / SERVER_ICON_CHUNK_SIZE;
	unsigned int sentChunks = 0;

	for (unsigned int chunkIndex = 0; chunkIndex < totalChunks; ++chunkIndex)
	{
		unsigned int offset = chunkIndex * SERVER_ICON_CHUNK_SIZE;
		unsigned int chunkSize = totalBytes - offset;
		if (chunkSize > SERVER_ICON_CHUNK_SIZE)
			chunkSize = SERVER_ICON_CHUNK_SIZE;

		RakNet::BitStream bitStream;
		bitStream.Write(RakNet::RakString(SERVER_ICON_CHUNK));
		bitStream.Write(totalBytes);
		bitStream.Write(totalChunks);
		bitStream.Write(chunkIndex);
		bitStream.Write(chunkSize);
		bitStream.WriteAlignedBytes(&_serverIconData[offset], chunkSize);

		if (rakPeer->AdvertiseSystem(address.ToString(false), address.GetPort(), (const char*)bitStream.GetData(), bitStream.GetNumberOfBytesUsed()))
			++sentChunks;
	}

	LOGI("[ServerIcon] Server sent icon chunks to %s: rawBytes=%u, chunks=%u/%u, chunkSize=%u\n",
		address.ToString(true), totalBytes, sentChunks, totalChunks, SERVER_ICON_CHUNK_SIZE);
}

void RakNetInstance::handleAdvertiseSystem(const RakNet::Packet* packet, RakNet::BitStream& bitStream)
{
	RakNet::RakString message;
	if (!bitStream.Read(message))
	{
		LOGW("[ServerIcon] Received malformed advertise packet from %s: missing message id\n",
			packet->systemAddress.ToString(true));
		return;
	}

	if (message.StrCmp(SERVER_ICON_REQUEST) == 0)
	{
		if (_isServer)
		{
			LOGI("[ServerIcon] Server received icon request from %s\n",
				packet->systemAddress.ToString(true));
			sendServerIconChunks(packet->systemAddress);
		}
		return;
	}

	if (message.StrCmp(SERVER_ICON_CHUNK) == 0)
	{
		if (!_isServer)
			handleServerIconChunk(packet->systemAddress, bitStream);
		return;
	}
}

void RakNetInstance::handleServerIconChunk(const RakNet::SystemAddress& address, RakNet::BitStream& bitStream)
{
	unsigned int totalBytes = 0;
	unsigned int totalChunks = 0;
	unsigned int chunkIndex = 0;
	unsigned int chunkSize = 0;

	if (!bitStream.Read(totalBytes) || !bitStream.Read(totalChunks) || !bitStream.Read(chunkIndex) || !bitStream.Read(chunkSize))
	{
		LOGW("[ServerIcon] Client received malformed icon chunk header from %s\n",
			address.ToString(true));
		return;
	}

	if (totalBytes == 0 || totalBytes > SERVER_ICON_MAX_BYTES || totalChunks == 0 || chunkIndex >= totalChunks ||
		chunkSize == 0 || chunkSize > SERVER_ICON_CHUNK_SIZE || chunkIndex * SERVER_ICON_CHUNK_SIZE + chunkSize > totalBytes)
	{
		LOGW("[ServerIcon] Client rejected icon chunk from %s: rawBytes=%u, chunks=%u, index=%u, chunkBytes=%u\n",
			address.ToString(true), totalBytes, totalChunks, chunkIndex, chunkSize);
		return;
	}

	std::vector<unsigned char> chunk(chunkSize);
	if (!bitStream.ReadAlignedBytes(chunk.data(), chunkSize))
	{
		LOGW("[ServerIcon] Client failed to read icon chunk payload from %s: index=%u, chunkBytes=%u\n",
			address.ToString(true), chunkIndex, chunkSize);
		return;
	}

	std::string key = address.ToString(true);
	PendingServerIcon& pending = _pendingServerIcons[key];
	if (pending.totalBytes == 0)
	{
		pending.totalBytes = totalBytes;
		pending.totalChunks = totalChunks;
		pending.receivedChunks = 0;
		pending.data.resize(totalBytes);
		pending.received.assign(totalChunks, false);
		LOGI("[ServerIcon] Client started receiving icon from %s: rawBytes=%u, chunks=%u\n",
			address.ToString(true), totalBytes, totalChunks);
	}

	if (pending.totalBytes != totalBytes || pending.totalChunks != totalChunks)
	{
		LOGW("[ServerIcon] Client rejected mismatched icon chunk from %s: expected rawBytes=%u chunks=%u, got rawBytes=%u chunks=%u\n",
			address.ToString(true), pending.totalBytes, pending.totalChunks, totalBytes, totalChunks);
		return;
	}

	if (!pending.received[chunkIndex])
	{
		unsigned int offset = chunkIndex * SERVER_ICON_CHUNK_SIZE;
		memcpy(&pending.data[offset], chunk.data(), chunkSize);
		pending.received[chunkIndex] = true;
		++pending.receivedChunks;
	}

	if (pending.receivedChunks != pending.totalChunks)
		return;

	std::string iconPath;
	if (!writeServerIcon(address, pending.data, iconPath))
	{
		LOGW("[ServerIcon] Client received complete icon from %s but failed to save it (rawBytes=%u)\n",
			address.ToString(true), pending.totalBytes);
		_pendingServerIcons.erase(key);
		return;
	}

	for (unsigned int i = 0; i < availableServers.size(); ++i)
	{
		if (availableServers[i].address == address)
		{
			availableServers[i].icon = iconPath;
			availableServers[i].hasIcon = true;
			availableServers[i].isClientHosted = false;
			availableServers[i].isSaved = true;
			break;
		}
	}

	LOGI("[ServerIcon] Client saved chunked icon from %s to %s (rawBytes=%u, chunks=%u)\n",
		address.ToString(true), iconPath.c_str(), pending.totalBytes, pending.totalChunks);
	_pendingServerIcons.erase(key);
}

ServerList RakNetInstance::loadSavedServers()
{
	ServerList servers;
	std::string path = getServerStorageBasePath() + "/servers/servers.txt";
	FILE* fp = fopen(path.c_str(), "r");
	if (!fp)
		return servers;

	char line[2048];
	while (fgets(line, sizeof(line), fp))
	{
		std::string value(line);
		while (!value.empty() && (value[value.size() - 1] == '\n' || value[value.size() - 1] == '\r'))
			value.resize(value.size() - 1);
		if (value.empty())
			continue;

		std::vector<std::string> fields;
		size_t start = 0;
		bool escaped = false;
		for (size_t i = 0; i <= value.size(); ++i)
		{
			if (i < value.size() && value[i] == '\\' && !escaped)
			{
				escaped = true;
				continue;
			}
			if (i == value.size() || (value[i] == '|' && !escaped))
			{
				fields.push_back(unescapeServerField(value.substr(start, i - start)));
				start = i + 1;
			}
			escaped = false;
		}

		if (fields.size() < 2)
			continue;

		PingedCompatibleServer server;
		server.address.FromString(fields[0].c_str());
		server.name = fields[1].c_str();
		server.icon = fields.size() > 2 ? fields[2] : "";
		server.hasIcon = fields.size() > 3 ? fields[3] == "1" : !server.icon.empty();
		server.isClientHosted = fields.size() > 4 ? fields[4] == "client" : false;
		server.isSaved = true;
		server.isSpecial = false;
		server.pingTime = 0;
		server.prevpingTime = 0;
		server.lastSeenTime = RakNet::GetTimeMS();
		servers.push_back(server);
	}

	fclose(fp);
	return servers;
}

void RakNetInstance::saveServerList(const ServerList& servers)
{
	std::string path = getServerStorageBasePath() + "/servers/servers.txt";
	if (!createDirectoriesForFile(path))
		return;

	FILE* fp = fopen(path.c_str(), "w");
	if (!fp)
		return;

	for (unsigned int i = 0; i < servers.size(); ++i)
	{
		const PingedCompatibleServer& server = servers[i];
		if (!server.isSaved && server.name.GetLength() == 0)
			continue;
		fprintf(fp, "%s|%s|%s|%s|%s\n",
			escapeServerField(server.address.ToString(true)).c_str(),
			escapeServerField(server.name.C_String()).c_str(),
			escapeServerField(server.icon).c_str(),
			server.hasIcon ? "1" : "0",
			server.isClientHosted ? "client" : "standalone");
	}

	fclose(fp);
}

RakNet::RakPeerInterface* RakNetInstance::getPeer()
{
	return rakPeer;
}

bool RakNetInstance::isProbablyBroken() {
    return rakPeer->errorState < -100;
}
void RakNetInstance::resetIsBroken() {
	rakPeer->errorState = 0;
}

bool RakNetInstance::isMyLocalGuid(const RakNet::RakNetGUID& guid)
{
	return rakPeer->IsActive() && rakPeer->GetMyGUID() == guid;
}

void RakNetInstance::runEvents(NetEventCallback* callback)
{
	RakNet::Packet* currentEvent;

	while ((currentEvent = rakPeer->Receive()) != NULL)
	{
		int packetId = currentEvent->data[0];
		int length = currentEvent->length;

		RakNet::BitStream activeBitStream(currentEvent->data + 1, length - 1, false);

		if (callback) {
			if (packetId < ID_USER_PACKET_ENUM)
			{
				//LOGI("Received event: %s\n", getPacketName(packetId));
				switch (packetId)
				{
				case ID_NEW_INCOMING_CONNECTION:
					callback->onNewClient(currentEvent->guid);
					break;
				case ID_CONNECTION_REQUEST_ACCEPTED:
					serverGuid = currentEvent->guid;
					callback->onConnect(currentEvent->guid);
					break;
				case ID_CONNECTION_ATTEMPT_FAILED:
					callback->onUnableToConnect();
					break;
				case ID_DISCONNECTION_NOTIFICATION:
				case ID_CONNECTION_LOST:
					callback->onDisconnect(currentEvent->guid);
					break;
				case ID_UNCONNECTED_PONG:
					{
						RakNet::TimeMS time;
						RakNet::RakString data;
						activeBitStream.Read(time);
						activeBitStream.Read(data);
						RakNet::TimeMS ping = RakNet::GetTimeMS() - time;

						int index = handleUnconnectedPong(data, currentEvent, APP_IDENTIFIER, false, ping);
						if (index < 0) {
							// Check if it's an official Mojang MineCon server
							index = handleUnconnectedPong(data, currentEvent, APP_IDENTIFIER_MINECON, true, ping);
							if (index >= 0) availableServers[index].isSpecial = true;
						}
					}
					break;
				case ID_ADVERTISE_SYSTEM:
					handleAdvertiseSystem(currentEvent, activeBitStream);
					break;
				}
			}
			else
			{
				int userPacketId = packetId - ID_USER_PACKET_ENUM;
				bool isStatusPacket = userPacketId <= PACKET_READY;

				if (isStatusPacket || _isServer || _isLoggedIn) {

					if (Packet* packet = MinecraftPackets::createPacket(packetId)) {
						packet->read(&activeBitStream);
						packet->handle(currentEvent->guid, callback);
						delete packet;
					}
				}
			}
		}

		rakPeer->DeallocatePacket(currentEvent);
		//delete activeBitStream;
	}

	if (isPingingForServers)
	{
		if (RakNet::GetTimeMS() - lastPingTime > 1000)
		{
			// remove servers that hasn't responded for a while
			ServerList::iterator it = availableServers.begin();
			for (; it != availableServers.end(); )
			{
				if (RakNet::GetTimeMS() - it->lastSeenTime > 3000)
				{
					it = availableServers.erase(it);
				}
				else
				{
					++it;
				}
			}

			pingForHosts(pingPort);
		}
	}

}

void RakNetInstance::send(Packet& packet) {
	RakNet::BitStream bitStream;
	packet.write(&bitStream);
	if (_isServer)
	{
		// broadcast to all connected clients
		rakPeer->Send(&bitStream, packet.priority, packet.reliability, 0, RakNet::UNASSIGNED_SYSTEM_ADDRESS, true);
	}
	else
	{
		// send to server
		rakPeer->Send(&bitStream, packet.priority, packet.reliability, 0, serverGuid, false);
	}
}

void RakNetInstance::send(const RakNet::RakNetGUID& guid, Packet& packet) {
	RakNet::BitStream bitStream;
	packet.write(&bitStream);
	rakPeer->Send(&bitStream, packet.priority, packet.reliability, 0, guid, false);
}


void RakNetInstance::send(Packet* packet)
{
	send(*packet);
	delete packet;
}

void RakNetInstance::send(const RakNet::RakNetGUID& guid, Packet* packet)
{
	send(guid, *packet);
	delete packet;
}

#ifdef _DEBUG
const char* RakNetInstance::getPacketName(int packetId)
{

	switch (packetId)
	{
	case ID_CONNECTED_PING:
		return "ID_CONNECTED_PING";
		
	case ID_UNCONNECTED_PING:
		return "ID_UNCONNECTED_PING";
		
	case ID_UNCONNECTED_PING_OPEN_CONNECTIONS:
		return "ID_UNCONNECTED_PING_OPEN_CONNECTIONS";
		
	case ID_CONNECTED_PONG:
		return "ID_CONNECTED_PONG";
		
	case ID_DETECT_LOST_CONNECTIONS:
		return "ID_DETECT_LOST_CONNECTIONS";
		
	case ID_OPEN_CONNECTION_REQUEST_1:
		return "ID_OPEN_CONNECTION_REQUEST_1";
		
	case ID_OPEN_CONNECTION_REPLY_1:
		return "ID_OPEN_CONNECTION_REPLY_1";
		
	case ID_OPEN_CONNECTION_REQUEST_2:
		return "ID_OPEN_CONNECTION_REQUEST_2";
		
	case ID_OPEN_CONNECTION_REPLY_2:
		return "ID_OPEN_CONNECTION_REPLY_2";
		
	case ID_CONNECTION_REQUEST:
		return "ID_CONNECTION_REQUEST";
		
	case ID_REMOTE_SYSTEM_REQUIRES_PUBLIC_KEY:
		return "ID_REMOTE_SYSTEM_REQUIRES_PUBLIC_KEY";
		
	case ID_OUR_SYSTEM_REQUIRES_SECURITY:
		return "ID_OUR_SYSTEM_REQUIRES_SECURITY";
		
	case ID_PUBLIC_KEY_MISMATCH:
		return "ID_PUBLIC_KEY_MISMATCH";
		
	case ID_OUT_OF_BAND_INTERNAL:
		return "ID_OUT_OF_BAND_INTERNAL";
		
	case ID_SND_RECEIPT_ACKED:
		return "ID_SND_RECEIPT_ACKED";
		
	case ID_SND_RECEIPT_LOSS:
		return "ID_SND_RECEIPT_LOSS";
		
	case ID_CONNECTION_REQUEST_ACCEPTED:
		return "ID_CONNECTION_REQUEST_ACCEPTED";
		
	case ID_CONNECTION_ATTEMPT_FAILED:
		return "ID_CONNECTION_ATTEMPT_FAILED";
		
	case ID_ALREADY_CONNECTED:
		return "ID_ALREADY_CONNECTED";
		
	case ID_NEW_INCOMING_CONNECTION:
		return "ID_NEW_INCOMING_CONNECTION";
		
	case ID_NO_FREE_INCOMING_CONNECTIONS:
		return "ID_NO_FREE_INCOMING_CONNECTIONS";
		
	case ID_DISCONNECTION_NOTIFICATION:
		return "ID_DISCONNECTION_NOTIFICATION";
		
	case ID_CONNECTION_LOST:
		return "ID_CONNECTION_LOST";
		
	case ID_CONNECTION_BANNED:
		return "ID_CONNECTION_BANNED";
		
	case ID_INVALID_PASSWORD:
		return "ID_INVALID_PASSWORD";
		
	case ID_INCOMPATIBLE_PROTOCOL_VERSION:
		return "ID_INCOMPATIBLE_PROTOCOL_VERSION";
		
	case ID_IP_RECENTLY_CONNECTED:
		return "ID_IP_RECENTLY_CONNECTED";
		
	case ID_TIMESTAMP:
		return "ID_TIMESTAMP";
		
	case ID_UNCONNECTED_PONG:
		return "ID_UNCONNECTED_PONG";
		
	case ID_ADVERTISE_SYSTEM:
		return "ID_ADVERTISE_SYSTEM";
		
	case ID_DOWNLOAD_PROGRESS:
		return "ID_DOWNLOAD_PROGRESS";
		
	case ID_REMOTE_DISCONNECTION_NOTIFICATION:
		return "ID_REMOTE_DISCONNECTION_NOTIFICATION";
		
	case ID_REMOTE_CONNECTION_LOST:
		return "ID_REMOTE_CONNECTION_LOST";
		
	case ID_REMOTE_NEW_INCOMING_CONNECTION:
		return "ID_REMOTE_NEW_INCOMING_CONNECTION";
		
	case ID_FILE_LIST_TRANSFER_HEADER:
		return "ID_FILE_LIST_TRANSFER_HEADER";
		
	case ID_FILE_LIST_TRANSFER_FILE:
		return "ID_FILE_LIST_TRANSFER_FILE";
		
	case ID_FILE_LIST_REFERENCE_PUSH_ACK:
		return "ID_FILE_LIST_REFERENCE_PUSH_ACK";
		
	case ID_DDT_DOWNLOAD_REQUEST:
		return "ID_DDT_DOWNLOAD_REQUEST";
		
	case ID_TRANSPORT_STRING:
		return "ID_TRANSPORT_STRING";
		
	case ID_REPLICA_MANAGER_CONSTRUCTION:
		return "ID_REPLICA_MANAGER_CONSTRUCTION";
		
	case ID_REPLICA_MANAGER_SCOPE_CHANGE:
		return "ID_REPLICA_MANAGER_SCOPE_CHANGE";
		
	case ID_REPLICA_MANAGER_SERIALIZE:
		return "ID_REPLICA_MANAGER_SERIALIZE";
		
	case ID_REPLICA_MANAGER_DOWNLOAD_STARTED:
		return "ID_REPLICA_MANAGER_DOWNLOAD_STARTED";
		
	case ID_REPLICA_MANAGER_DOWNLOAD_COMPLETE:
		return "ID_REPLICA_MANAGER_DOWNLOAD_COMPLETE";
		
	case ID_RAKVOICE_OPEN_CHANNEL_REQUEST:
		return "ID_RAKVOICE_OPEN_CHANNEL_REQUEST";
		
	case ID_RAKVOICE_OPEN_CHANNEL_REPLY:
		return "ID_RAKVOICE_OPEN_CHANNEL_REPLY";
		
	case ID_RAKVOICE_CLOSE_CHANNEL:
		return "ID_RAKVOICE_CLOSE_CHANNEL";
		
	case ID_RAKVOICE_DATA:
		return "ID_RAKVOICE_DATA";
		
	case ID_AUTOPATCHER_GET_CHANGELIST_SINCE_DATE:
		return "ID_AUTOPATCHER_GET_CHANGELIST_SINCE_DATE";
		
	case ID_AUTOPATCHER_CREATION_LIST:
		return "ID_AUTOPATCHER_CREATION_LIST";
		
	case ID_AUTOPATCHER_DELETION_LIST:
		return "ID_AUTOPATCHER_DELETION_LIST";
		
	case ID_AUTOPATCHER_GET_PATCH:
		return "ID_AUTOPATCHER_GET_PATCH";
		
	case ID_AUTOPATCHER_PATCH_LIST:
		return "ID_AUTOPATCHER_PATCH_LIST";
		
	case ID_AUTOPATCHER_REPOSITORY_FATAL_ERROR:
		return "ID_AUTOPATCHER_REPOSITORY_FATAL_ERROR";
		
	case ID_AUTOPATCHER_FINISHED_INTERNAL:
		return "ID_AUTOPATCHER_FINISHED_INTERNAL";
		
	case ID_AUTOPATCHER_FINISHED:
		return "ID_AUTOPATCHER_FINISHED";
		
	case ID_AUTOPATCHER_RESTART_APPLICATION:
		return "ID_AUTOPATCHER_RESTART_APPLICATION";
		
	case ID_NAT_PUNCHTHROUGH_REQUEST:
		return "ID_NAT_PUNCHTHROUGH_REQUEST";
		
	case ID_NAT_GROUP_PUNCHTHROUGH_REQUEST:
		return "ID_NAT_GROUP_PUNCHTHROUGH_REQUEST";
		
	case ID_NAT_GROUP_PUNCHTHROUGH_REPLY:
		return "ID_NAT_GROUP_PUNCHTHROUGH_REPLY";
		
	case ID_NAT_CONNECT_AT_TIME:
		return "ID_NAT_CONNECT_AT_TIME";
		
	case ID_NAT_GET_MOST_RECENT_PORT:
		return "ID_NAT_GET_MOST_RECENT_PORT";
		
	case ID_NAT_CLIENT_READY:
		return "ID_NAT_CLIENT_READY";
		
	case ID_NAT_GROUP_PUNCHTHROUGH_FAILURE_NOTIFICATION:
		return "ID_NAT_GROUP_PUNCHTHROUGH_FAILURE_NOTIFICATION";
		
	case ID_NAT_TARGET_NOT_CONNECTED:
		return "ID_NAT_TARGET_NOT_CONNECTED";
		
	case ID_NAT_TARGET_UNRESPONSIVE:
		return "ID_NAT_TARGET_UNRESPONSIVE";
		
	case ID_NAT_CONNECTION_TO_TARGET_LOST:
		return "ID_NAT_CONNECTION_TO_TARGET_LOST";
		
	case ID_NAT_ALREADY_IN_PROGRESS:
		return "ID_NAT_ALREADY_IN_PROGRESS";
		
	case ID_NAT_PUNCHTHROUGH_FAILED:
		return "ID_NAT_PUNCHTHROUGH_FAILED";
		
	case ID_NAT_PUNCHTHROUGH_SUCCEEDED:
		return "ID_NAT_PUNCHTHROUGH_SUCCEEDED";
		
	case ID_NAT_GROUP_PUNCH_FAILED:
		return "ID_NAT_GROUP_PUNCH_FAILED";
		
	case ID_NAT_GROUP_PUNCH_SUCCEEDED:
		return "ID_NAT_GROUP_PUNCH_SUCCEEDED";
		
	case ID_READY_EVENT_SET:
		return "ID_READY_EVENT_SET";
		
	case ID_READY_EVENT_UNSET:
		return "ID_READY_EVENT_UNSET";
		
	case ID_READY_EVENT_ALL_SET:
		return "ID_READY_EVENT_ALL_SET";
		
	case ID_READY_EVENT_QUERY:
		return "ID_READY_EVENT_QUERY";
		
	case ID_LOBBY_GENERAL:
		return "ID_LOBBY_GENERAL";
		
	case ID_RPC_REMOTE_ERROR:
		return "ID_RPC_REMOTE_ERROR";
		
	case ID_RPC_PLUGIN:
		return "ID_RPC_PLUGIN";
		
	case ID_FILE_LIST_REFERENCE_PUSH:
		return "ID_FILE_LIST_REFERENCE_PUSH";
		
	case ID_READY_EVENT_FORCE_ALL_SET:
		return "ID_READY_EVENT_FORCE_ALL_SET";
		
	case ID_ROOMS_EXECUTE_FUNC:
		return "ID_ROOMS_EXECUTE_FUNC";
		
	case ID_ROOMS_LOGON_STATUS:
		return "ID_ROOMS_LOGON_STATUS";
		
	case ID_ROOMS_HANDLE_CHANGE:
		return "ID_ROOMS_HANDLE_CHANGE";
		
	case ID_LOBBY2_SEND_MESSAGE:
		return "ID_LOBBY2_SEND_MESSAGE";
		
	case ID_LOBBY2_SERVER_ERROR:
		return "ID_LOBBY2_SERVER_ERROR";
		
	case ID_FCM2_NEW_HOST:
		return "ID_FCM2_NEW_HOST";
		
	case ID_FCM2_REQUEST_FCMGUID:
		return "ID_FCM2_REQUEST_FCMGUID";
		
	case ID_FCM2_RESPOND_CONNECTION_COUNT:
		return "ID_FCM2_RESPOND_CONNECTION_COUNT";
		
	case ID_FCM2_INFORM_FCMGUID:
		return "ID_FCM2_INFORM_FCMGUID";
		
	case ID_FCM2_UPDATE_MIN_TOTAL_CONNECTION_COUNT:
		return "ID_FCM2_UPDATE_MIN_TOTAL_CONNECTION_COUNT";
		
	case ID_UDP_PROXY_GENERAL:
		return "ID_UDP_PROXY_GENERAL";
		
	case ID_SQLite3_EXEC:
		return "ID_SQLite3_EXEC";
		
	case ID_SQLite3_UNKNOWN_DB:
		return "ID_SQLite3_UNKNOWN_DB";
		
	case ID_SQLLITE_LOGGER:
		return "ID_SQLLITE_LOGGER";
		
	case ID_NAT_TYPE_DETECTION_REQUEST:
		return "ID_NAT_TYPE_DETECTION_REQUEST";
		
	case ID_NAT_TYPE_DETECTION_RESULT:
		return "ID_NAT_TYPE_DETECTION_RESULT";
		
	case ID_ROUTER_2_INTERNAL:
		return "ID_ROUTER_2_INTERNAL";
		
	case ID_ROUTER_2_FORWARDING_NO_PATH:
		return "ID_ROUTER_2_FORWARDING_NO_PATH";
		
	case ID_ROUTER_2_FORWARDING_ESTABLISHED:
		return "ID_ROUTER_2_FORWARDING_ESTABLISHED";
		
	case ID_ROUTER_2_REROUTED:
		return "ID_ROUTER_2_REROUTED";
		
	case ID_TEAM_BALANCER_INTERNAL:
		return "ID_TEAM_BALANCER_INTERNAL";
		
	case ID_TEAM_BALANCER_REQUESTED_TEAM_CHANGE_PENDING:
		return "ID_TEAM_BALANCER_REQUESTED_TEAM_CHANGE_PENDING";
		
	case ID_TEAM_BALANCER_TEAMS_LOCKED:
		return "ID_TEAM_BALANCER_TEAMS_LOCKED";
		
	case ID_TEAM_BALANCER_TEAM_ASSIGNED:
		return "ID_TEAM_BALANCER_TEAM_ASSIGNED";
		
	case ID_LIGHTSPEED_INTEGRATION:
		return "ID_LIGHTSPEED_INTEGRATION";
		
	case ID_XBOX_LOBBY:
		return "ID_XBOX_LOBBY";
		
	case ID_TWO_WAY_AUTHENTICATION_INCOMING_CHALLENGE_SUCCESS:
		return "ID_TWO_WAY_AUTHENTICATION_INCOMING_CHALLENGE_SUCCESS";
		
	case ID_TWO_WAY_AUTHENTICATION_OUTGOING_CHALLENGE_SUCCESS:
		return "ID_TWO_WAY_AUTHENTICATION_OUTGOING_CHALLENGE_SUCCESS";
		
	case ID_TWO_WAY_AUTHENTICATION_INCOMING_CHALLENGE_FAILURE:
		return "ID_TWO_WAY_AUTHENTICATION_INCOMING_CHALLENGE_FAILURE";
		
	case ID_TWO_WAY_AUTHENTICATION_OUTGOING_CHALLENGE_FAILURE:
		return "ID_TWO_WAY_AUTHENTICATION_OUTGOING_CHALLENGE_FAILURE";
		
	case ID_TWO_WAY_AUTHENTICATION_OUTGOING_CHALLENGE_TIMEOUT:
		return "ID_TWO_WAY_AUTHENTICATION_OUTGOING_CHALLENGE_TIMEOUT";
		
	case ID_TWO_WAY_AUTHENTICATION_NEGOTIATION:
		return "ID_TWO_WAY_AUTHENTICATION_NEGOTIATION";
		
	case ID_CLOUD_POST_REQUEST:
		return "ID_CLOUD_POST_REQUEST";
		
	case ID_CLOUD_RELEASE_REQUEST:
		return "ID_CLOUD_RELEASE_REQUEST";
		
	case ID_CLOUD_GET_REQUEST:
		return "ID_CLOUD_GET_REQUEST";
		
	case ID_CLOUD_GET_RESPONSE:
		return "ID_CLOUD_GET_RESPONSE";
		
	case ID_CLOUD_UNSUBSCRIBE_REQUEST:
		return "ID_CLOUD_UNSUBSCRIBE_REQUEST";
		
	case ID_CLOUD_SERVER_TO_SERVER_COMMAND:
		return "ID_CLOUD_SERVER_TO_SERVER_COMMAND";
		
	case ID_CLOUD_SUBSCRIPTION_NOTIFICATION:
		return "ID_CLOUD_SUBSCRIPTION_NOTIFICATION";
		
	case ID_RESERVED_1:
		return "ID_RESERVED_1";
		
	case ID_RESERVED_2:
		return "ID_RESERVED_2";
		
	case ID_RESERVED_3:
		return "ID_RESERVED_3";
		
	case ID_RESERVED_4:
		return "ID_RESERVED_4";
		
	case ID_RESERVED_5:
		return "ID_RESERVED_5";
		
	case ID_RESERVED_6:
		return "ID_RESERVED_6";
		
	case ID_RESERVED_7:
		return "ID_RESERVED_7";
		
	case ID_RESERVED_8:
		return "ID_RESERVED_8";
		
	case ID_RESERVED_9:
		return "ID_RESERVED_9";
		
	default:
		break;
	}
	return "Unknown or user-defined";
}
#endif

int RakNetInstance::handleUnconnectedPong(const RakNet::RakString& data, const RakNet::Packet* p, const char* appid, bool insertAtBeginning, const RakNet::TimeMS ping)
{
	RakNet::RakString appIdentifier(appid);
	// This weird code is a result of RakString.Find being pretty useless
	bool emptyNameOrLonger = data.GetLength() >= appIdentifier.GetLength();

	if ( !emptyNameOrLonger || appIdentifier.StrCmp(data.SubStr(0, appIdentifier.GetLength())) != 0)
		return -1;

	RakNet::RakString serverName = data.SubStr(appIdentifier.GetLength(), data.GetLength() - appIdentifier.GetLength());
	bool hasMetadata = false;
	bool isClientHosted = false;
	bool hasIcon = false;
	std::string iconData;

	int metaIndex = serverName.Find(SERVER_META_MARKER);
	if (metaIndex >= 0)
	{
		RakNet::RakString meta = serverName.SubStr(metaIndex + strlen(SERVER_META_MARKER), serverName.GetLength() - metaIndex - strlen(SERVER_META_MARKER));
		serverName = serverName.SubStr(0, metaIndex);
		hasMetadata = true;

		std::string metaText = meta.C_String();
		size_t start = 0;
		while (start < metaText.size())
		{
			size_t end = metaText.find(';', start);
			std::string token = metaText.substr(start, end == std::string::npos ? std::string::npos : end - start);
			size_t equals = token.find('=');
			if (equals != std::string::npos)
			{
				std::string key = token.substr(0, equals);
				std::string value = token.substr(equals + 1);
				if (key == "type")
					isClientHosted = value == "client";
				else if (key == "icon")
					hasIcon = value == "1";
				else if (key == "icondata")
					iconData = value;
			}
			if (end == std::string::npos)
				break;
			start = end + 1;
		}
	}

	std::string iconPath;
	if (hasMetadata)
	{
		LOGI("[ServerIcon] Client parsed announce from %s: type=%s, hasIcon=%s, encodedBytes=%u\n",
			p->systemAddress.ToString(true),
			isClientHosted ? "client" : "standalone",
			hasIcon ? "yes" : "no",
			(unsigned int)iconData.size());
	}
	else
	{
		LOGI("[ServerIcon] Client parsed announce from %s with no icon metadata\n",
			p->systemAddress.ToString(true));
	}

	if (hasMetadata && hasIcon && !isClientHosted && !iconData.empty())
	{
		std::vector<unsigned char> decodedIcon = base64Decode(iconData);
		if (decodedIcon.empty())
		{
			LOGW("[ServerIcon] Client received icon metadata from %s but decoded payload was empty (encodedBytes=%u)\n",
				p->systemAddress.ToString(true), (unsigned int)iconData.size());
		}
		else if (writeServerIcon(p->systemAddress, decodedIcon, iconPath))
		{
			LOGI("[ServerIcon] Client saved icon from %s to %s (rawBytes=%u, encodedBytes=%u)\n",
				p->systemAddress.ToString(true), iconPath.c_str(), (unsigned int)decodedIcon.size(), (unsigned int)iconData.size());
		}
		else
		{
			LOGW("[ServerIcon] Client received icon from %s but failed to save it (rawBytes=%u, encodedBytes=%u)\n",
				p->systemAddress.ToString(true), (unsigned int)decodedIcon.size(), (unsigned int)iconData.size());
		}
	}
	else if (hasMetadata && hasIcon && !isClientHosted)
	{
		std::string existingIconPath = getServerIconPath(p->systemAddress);
		if (fileExists(existingIconPath))
		{
			iconPath = existingIconPath;
			LOGI("[ServerIcon] Client found saved icon for %s at %s; skipping chunk request\n",
				p->systemAddress.ToString(true), iconPath.c_str());
		}
		else
		{
			LOGI("[ServerIcon] Client received icon metadata from %s without inline data; requesting chunked icon\n",
				p->systemAddress.ToString(true));
			requestServerIcon(p->systemAddress);
		}
	}
	else if (hasMetadata && hasIcon && isClientHosted)
	{
		LOGI("[ServerIcon] Client received icon metadata from %s but server is client-hosted; using default icon\n",
			p->systemAddress.ToString(true));
	}

	bool found = false;
	for (unsigned int i = 0; i < availableServers.size(); i++) {
		if (availableServers[i].address == p->systemAddress) {
			//availableServers[i].prevpingTime = availableServers[i].pingTime;
			availableServers[i].pingTime = ping;
			availableServers[i].lastSeenTime = RakNet::GetTimeMS();

			bool emptyName = serverName.GetLength() == 0;
			if (emptyName)
				availableServers[i].name = "";
			else {
				availableServers[i].name = serverName;
			}
			availableServers[i].hasIcon = hasIcon;
			availableServers[i].isClientHosted = isClientHosted;
			availableServers[i].isSaved = true;
			if (!iconPath.empty())
				availableServers[i].icon = iconPath;
			//LOGI("Swapping name: %s\n", availableServers[i].name.C_String());
			return i;
		}
	}
	PingedCompatibleServer server;
	server.address = p->systemAddress;
	server.pingTime = ping;
	server.prevpingTime = ping;
	server.lastSeenTime = RakNet::GetTimeMS();
	server.isSpecial = false;
	server.name = serverName;
	server.icon = iconPath;
	server.hasIcon = hasIcon;
	server.isClientHosted = isClientHosted;
	server.isSaved = true;

	if (insertAtBeginning) {
		availableServers.insert(availableServers.begin(), server);
		return 0;
	} else {
		availableServers.push_back(server);
		return availableServers.size() - 1;
	}
}

void RakNetInstance::setIsLoggedIn( bool status ) {
	_isLoggedIn = status;
}
