#ifndef NET_MINECRAFT_NETWORK_PACKET__MovePlayerPacket_H__
#define NET_MINECRAFT_NETWORK_PACKET__MovePlayerPacket_H__

#include "../Packet.h"

class MovePlayerPacket : public Packet
{
public:

	int entityId;
	double x, y, z;
	float xRot, yRot;

	MovePlayerPacket()
	{
	}

	MovePlayerPacket(int entityId, double x, double y, double z, float xRot, float yRot)
	:	entityId(entityId),
		x(x),
		y(y),
		z(z),
		xRot(xRot),
		yRot(yRot)
	{}

	void write(RakNet::BitStream* bitStream) override
	{
		bitStream->Write((RakNet::MessageID)(ID_USER_PACKET_ENUM + PACKET_MOVEPLAYER));

		bitStream->Write(entityId);
		bitStream->Write(x);
		bitStream->Write(y);
		bitStream->Write(z);
		bitStream->Write(yRot);
		bitStream->Write(xRot);
	}

	void read(RakNet::BitStream* bitStream) override
	{
		bitStream->Read(entityId);
		bitStream->Read(x);
		bitStream->Read(y);
		bitStream->Read(z);
		bitStream->Read(yRot);
		bitStream->Read(xRot);
	}

	void handle(const RakNet::RakNetGUID& source, NetEventCallback* callback) override
	{
		callback->handle(source, (MovePlayerPacket*)this);
	}
};

#endif /*NET_MINECRAFT_NETWORK_PACKET__MovePlayerPacket_H__*/
