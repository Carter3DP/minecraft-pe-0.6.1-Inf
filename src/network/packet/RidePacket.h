#ifndef NET_MINECRAFT_NETWORK_PACKET__RidePacket_H__
#define NET_MINECRAFT_NETWORK_PACKET__RidePacket_H__

#include "../Packet.h"
#include "../../world/entity/Entity.h"

class RidePacket : public Packet
{
public:
	RidePacket()
	:	riderId(0),
		vehicleId(0)
	{}

	RidePacket(const Entity* rider, const Entity* vehicle)
	:	riderId(rider ? rider->entityId : 0),
		vehicleId(vehicle ? vehicle->entityId : 0)
	{}

	void write(RakNet::BitStream* bitStream) override
	{
		bitStream->Write((RakNet::MessageID)(ID_USER_PACKET_ENUM + PACKET_RIDE));
		bitStream->Write(riderId);
		bitStream->Write(vehicleId);
	}

	void read(RakNet::BitStream* bitStream) override
	{
		bitStream->Read(riderId);
		bitStream->Read(vehicleId);
	}

	void handle(const RakNet::RakNetGUID& source, NetEventCallback* callback) override
	{
		callback->handle(source, (RidePacket*)this);
	}

	int riderId;
	int vehicleId;
};

#endif /*NET_MINECRAFT_NETWORK_PACKET__RidePacket_H__*/
