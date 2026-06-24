#ifndef NET_MINECRAFT_NETWORK_PACKET__SetEntityMotionPacket_H__
#define NET_MINECRAFT_NETWORK_PACKET__SetEntityMotionPacket_H__

//package net.minecraft.network.packet;

#include "../Packet.h"
#include "../../world/entity/Entity.h"

class SetEntityMotionPacket: public Packet
{
public:
	SetEntityMotionPacket() {}

	SetEntityMotionPacket(Entity* e) {
		init(e->entityId, e->xd, e->yd, e->zd);
	}

	SetEntityMotionPacket(int id, double xd, double yd, double zd) {
		init(id, xd, yd, zd);
	}

	void write(RakNet::BitStream* bitStream)
	{
		const double M = 3.9;
		short xa = (short)(8000.0f * Mth::clamp(xd, -M, M));
		short ya = (short)(8000.0f * Mth::clamp(yd, -M, M));
		short za = (short)(8000.0f * Mth::clamp(zd, -M, M));

		bitStream->Write((RakNet::MessageID)(ID_USER_PACKET_ENUM + PACKET_SETENTITYMOTION));
		bitStream->Write(id);
		bitStream->Write(xa);
		bitStream->Write(ya);
		bitStream->Write(za);
	}

	void read(RakNet::BitStream* bitStream)
	{
		short xa, ya, za;
		bitStream->Read(id);
		bitStream->Read(xa);
		bitStream->Read(ya);
		bitStream->Read(za);

		xd = xa / 8000.0f;
		yd = ya / 8000.0f;
		zd = za / 8000.0f;
	}

	void handle(const RakNet::RakNetGUID& source, NetEventCallback* callback)
	{
		callback->handle(source, (SetEntityMotionPacket*)this);
	}

private:
	void init(int entityId, double xd, double yd, double zd) {
		this->id = entityId;
		this->xd = xd;
		this->yd = yd;
		this->zd = zd;
	}

public:
	int id;
	double xd, yd, zd;

};

#endif /*NET_MINECRAFT_NETWORK_PACKET__SetEntityMotionPacket_H__*/
