#ifndef NET_MINECRAFT_NETWORK_PACKET__SetTimePacket_H__
#define NET_MINECRAFT_NETWORK_PACKET__SetTimePacket_H__

//package net.minecraft.network.packet;

#include "../Packet.h"
#include <cstdint>

class SetTimePacket: public Packet {
public:
    SetTimePacket() {
    }

    SetTimePacket(long time)
	:	time((int32_t)time)
	{}

	void write(RakNet::BitStream* bitStream) override
	{
		bitStream->Write((RakNet::MessageID)(ID_USER_PACKET_ENUM + PACKET_SETTIME));
		bitStream->Write(time);
	}

	void read(RakNet::BitStream* bitStream) override
	{
		bitStream->Read(time);
	}

	void handle(const RakNet::RakNetGUID& source, NetEventCallback* callback) override
	{
		callback->handle(source, (SetTimePacket*)this);
	}
	
	int32_t time;
};

#endif /*NET_MINECRAFT_NETWORK_PACKET__SetTimePacket_H__*/
