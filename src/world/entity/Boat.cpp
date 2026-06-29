#include "Boat.h"

#include <cmath>

#include "player/Player.h"
#include "SharedFlags.h"
#include "../entity/EntityEvent.h"
#include "../level/Level.h"
#include "../level/material/Material.h"
#include "../level/tile/Tile.h"
#include "../../util/Random.h"
#include "../../util/PerfTimer.h"

#include "../../nbt/CompoundTag.h"

#include "../../network/RakNetInstance.h"
#include "../../network/packet/MoveEntityPacket.h"
#include "../../network/packet/SetEntityMotionPacket.h"


EntityBoat::EntityBoat(Level* lvl):
    super(lvl) {
    entityRendererId = ER_BOAT_RENDERER;
    boatCurrentDamage = 0;
    boatTimeSinceHit = 0;
    boatRockDirection = 1;
    blocksBuilding = true;
    setSize(1.5F, 0.6F);
    heightOffset = bbHeight / 2.0F;
    super::setPos(x, y + (double)heightOffset, z);
    super::xd = 0;
    super::yd = 0;
    super::zd = 0;
    super::xo = x;
    super::yo = y;
    super::zo = z;
    sentX = x;
    sentY = y;
    sentZ = z;
    sentXd = xd;
    sentYd = yd;
    sentZd = zd;
    sentRotX = xRot;
    sentRotY = yRot;
}

EntityBoat::~EntityBoat(){
}

bool EntityBoat::canTriggerWalking() {
    return false;
}

AABB EntityBoat::getCollisionBox(Entity* e) {
    return e->bb;
}

AABB EntityBoat::getBoundingBox() {
    return bb;
}

bool EntityBoat::isPushable() {
    return true;
}

double EntityBoat::getMountedYOffset() {
    return (double)bbHeight * 0 - 0.30000001192092896;
}

bool EntityBoat::hurt(Entity* e, int damage) {
    if (!level->isClientSide && isAlive()) {
        boatRockDirection = -boatRockDirection;
        boatTimeSinceHit = 10;
        boatCurrentDamage += damage * 10;
        markHurt();
        level->broadcastEntityEvent(this, EntityEvent::HURT);
        if (boatCurrentDamage > 40) {
            if (riddenByEntity != NULL) {
                riddenByEntity->mountEntity(NULL);
            }
            int drops;

            //wood drops
            for(drops = 0; drops < 3; ++drops) {
                spawnAtLocation(Tile::wood->id, 1);
            }

            //stick drops. Eventually will remove, I hated this
            for(drops = 0; drops < 2; ++drops) {
                spawnAtLocation(Item::stick->id, 1);
            }
            remove();
        }
        return true;
    } else {
        return true;
    }
}

void EntityBoat::handleEntityEvent(char eventId) {
    if (eventId == EntityEvent::HURT) {
        animateHurt();
    } else {
        super::handleEntityEvent(eventId);
    }
}

void EntityBoat::animateHurt() {
    boatRockDirection = -boatRockDirection;
    boatTimeSinceHit = 10;
    boatCurrentDamage += boatCurrentDamage * 10;
}

bool EntityBoat::canBeCollidedWith() {
    return isAlive();
}

bool EntityBoat::isPickable() {
    return isAlive();
}

void EntityBoat::lerpTo(double x, double y, double z, float yRot, float xRot, int steps) {
    boatX = x;
    boatY = y;
    boatZ = z;
    boatYaw = (double)yRot;
    boatPitch = (double)xRot;
    boatPosRotationIncrements = steps + 4;
    xd = vx;
    yd = vy;
    zd = vz;
}

void EntityBoat::push(double xa, double ya, double za) {
    vx = xd = xa;
    vy = yd = ya;
    vz = zd = za;
}

void EntityBoat::tick() {
    super::tick();
    if (boatTimeSinceHit > 0) {
        --boatTimeSinceHit;
    }
    if (boatCurrentDamage > 0) {
        --boatCurrentDamage;
    }
    xo = x;
    yo = y;
    zo = z;
    int8_t bboxes = 5;
    double waterCoverage = 0.0;
    for(int i = 0; i < bboxes; ++i) {
        double sliceMinY = bb.y0 + (bb.y1 - bb.y0) * (double)(i + 0) / (double)bboxes - 0.125;
        double sliceMaxY = bb.y0 + (bb.y1 - bb.y0) * (double)(i + 1) / (double)bboxes - 0.125;
        AABB boundbox = AABB(bb.x0, sliceMinY, bb.z0, bb.x1, sliceMaxY, bb.z1);
        if (level->containsMaterial(boundbox, Material::water)) {
            waterCoverage += 1.0 / (double)bboxes;
        }
    }
    double yv;
    double zv;
    double rot;
    double xv;
    if (level->isClientSide){
        if (boatPosRotationIncrements > 0) {
            xv = x + (boatX - x) / (double)boatPosRotationIncrements;
            yv = y + (boatY - y) / (double)boatPosRotationIncrements;
            zv = z + (boatZ - z) / (double)boatPosRotationIncrements;
            for(rot = boatYaw - (double)yRot; rot < -180.0; rot += 360.0) {
            }
            while(rot >= 180.0) {
                rot -= 360.0;
            }
            yRot = (float)((double)yRot + rot / (double)boatPosRotationIncrements);
            xRot = (float)((double)xRot + (boatPitch - (double)xRot) / (double)boatPosRotationIncrements);
            --boatPosRotationIncrements;
            setPos(xv, yv, zv);
            setRot(yRot, xRot);
        } else {
            xv = x + xd;
            yv = y + yd;
            zv = z + zd;
            setPos(xv, yv, zv);
            if (onGround) {
                xd *= 0.5;
                yd *= 0.5;
                zd *= 0.5;
            }
            xd *= 0.9900000095367432;
            yd *= 0.949999988079071;
            zd *= 0.9900000095367432;
        }
    } else {
        if (waterCoverage < 1.0) {
            xv = waterCoverage * 2.0 - 1.0;
            yd += 0.03999999910593033 * xv;
        } else {
            if (yd < 0.0) {
                yd /= 2.0;
            }
            yd += 0.007000000216066837;
        }
        if (riddenByEntity != NULL) {
            xd += riddenByEntity->xd * 0.2;
            zd += riddenByEntity->zd * 0.2;
        }
        xv = 0.4;
        if (xd < -xv) {
            xd = -xv;
        }
        if (xd > xv) {
            xd = xv;
        }
        if (zd < -xv) {
            zd = -xv;
        }
        if (zd > xv) {
            zd = xv;
        }
        if (onGround) {
            xd *= 0.5;
            yd *= 0.5;
            zd *= 0.5;
        }
        move(xd, yd, zd);
        yv = sqrt(xd * xd + zd * zd);
        if (yv > 0.15) {
            zv = cos((double)yRot * 3.141592653589793 / 180.0);
            rot = sin((double)yRot * 3.141592653589793 / 180.0);
            for(int i = 0; (double)i < 1.0 + yv * 60.0; ++i) {
                double randomAlongBoat = (double)(sharedRandom.nextFloat() * 2.0 - 1.0);
                double randomSide = (double)(sharedRandom.nextInt(2) * 2 - 1) * 0.7;
                double splashX;
                double splashZ;
                if (sharedRandom.nextBoolean()) {
                    splashX = x - zv * randomAlongBoat * 0.8 + rot * randomSide;
                    splashZ = z - rot * randomAlongBoat * 0.8 - zv * randomSide;
                    //todo: add splash particle
                    //level->addParticle("splash", splashX, y - 0.125, splashZ, xd, yd, zd);
                } else {
                    splashX = x + zv + rot * randomAlongBoat * 0.7;
                    splashZ = z + rot - zv * randomAlongBoat * 0.7;
                    //todo: add splash particle
                    //level->addParticle("splash", splashX, y - 0.125, splashZ, xd, yd, zd);
                }
            }
        }
        if (horizontalCollision && yv > 0.15) {
            remove();
            int dropIndex;
            for(dropIndex = 0; dropIndex < 3; ++dropIndex) {
                spawnAtLocation(Tile::wood->id, 1);
            }
            for(dropIndex = 0; dropIndex < 2; ++dropIndex) {
                spawnAtLocation(Item::stick->id, 1);
            }
        } 
        else {
            xd *= 0.9900000095367432;
            yd *= 0.949999988079071;
            zd *= 0.9900000095367432;
        }
        xRot = 0.0F;
        zv = (double)yRot;
        rot = xo - x;
        double deltaZ = zo - z;
        if (rot * rot + deltaZ * deltaZ > 0.001) {
            zv = (double)((float)(atan2(deltaZ, rot) * 180.0 / 3.141592653589793));
        }
        double yawDelta;
        for(yawDelta = zv - (double)yRot; yawDelta >= 180.0; yawDelta -= 360.0) {
        }
        while(yawDelta < -180.0) {
            yawDelta += 360.0;
        }
        if (yawDelta > 20.0) {
            yawDelta = 20.0;
        }
        if (yawDelta < -20.0) {
            yawDelta = -20.0;
        }
        yRot = (float)((double)yRot + yawDelta);
        setRot(yRot, xRot);
        EntityList& entities = level->getEntities(this, bb.expand(0.20000000298023224, 0.0, 0.20000000298023224));
        int entityIndex;
        if (!entities.empty() && entities.size() > 0) {
            for(entityIndex = 0; entityIndex < entities.size(); ++entityIndex) {
                Entity* nearbyEntity = entities[entityIndex];
                if (nearbyEntity != riddenByEntity && nearbyEntity->isPushable() && nearbyEntity->isEntityType(EntityTypes::boatEntity)) {
                    nearbyEntity->push(this);
                }
            }
        }
        //snow layer breaking behavior
        /*for(entityIndex = 0; entityIndex < 4; ++entityIndex) {
            int snowX = floor(x + ((double)(entityIndex % 2) - 0.5) * 0.8);
            int snowY = floor(y);
            int snowZ = floor(z + ((double)(entityIndex / 2) - 0.5) * 0.8);
            if (level->containsMaterial(boundbox, ) == Tile::snow->id) {
                level->setBlockWithNotify(snowX, snowY, snowZ, 0);
            }
        }*/
        if (riddenByEntity != NULL && riddenByEntity->removed) {
            riddenByEntity = NULL;
        }

        if (std::abs(x - sentX) > .1f || std::abs(y - sentY) > .05f || std::abs(z - sentZ) > .1f || std::abs(sentRotX - xRot) > 1 || std::abs(sentRotY - yRot) > 1) {
            MoveEntityPacket_PosRot packet(this);
            level->raknetInstance->send(packet);
            sentX = x;
            sentY = y;
            sentZ = z;
            sentRotX = xRot;
            sentRotY = yRot;
        }

        double ddx = std::abs(xd - sentXd);
        double ddy = std::abs(yd - sentYd);
        double ddz = std::abs(zd - sentZd);
        const double max = 0.02;
        const double diff = ddx * ddx + ddy * ddy + ddz * ddz;
        if (diff > max * max || (diff > 0 && xd == 0 && yd == 0 && zd == 0)) {
            sentXd = xd;
            sentYd = yd;
            sentZd = zd;
            SetEntityMotionPacket packet(this);
            level->raknetInstance->send(packet);
        }
    }
}
void EntityBoat::updateRiderPosition() {
    if (riddenByEntity != NULL) {
        double riderOffsetX = cos((double)yRot * 3.141592653589793 / 180.0) * 0.4;
        double riderOffsetZ = sin((double)yRot * 3.141592653589793 / 180.0) * 0.4;
        riddenByEntity->setPos(x + riderOffsetX, y + getMountedYOffset() + riddenByEntity->heightOffset, z + riderOffsetZ);
    }
}

float EntityBoat::getShadowHeightOffs() {
    return 0.0;
}

bool EntityBoat::interact(Player* player) {
    if (riddenByEntity != NULL && riddenByEntity->isPlayer() && riddenByEntity != player) {
        return true;
    } else {
        player->mountEntity(this);
        return true;
    }
}

void EntityBoat::addAdditonalSaveData( CompoundTag* entityTag )
{
	entityTag->putShort("Health", (short) boatCurrentDamage);
	entityTag->putShort("HurtTime", (short) boatTimeSinceHit);

	//if (isPlayer()) LOGI("Saving %d, %d, %d, %d\n", health, hurtTime, deathTime, attackTime);
}

void EntityBoat::readAdditionalSaveData( CompoundTag* tag )
{
	boatCurrentDamage = tag->getShort("Health");
	boatTimeSinceHit = tag->getShort("HurtTime");

	//if (isPlayer()) LOGI("Reading %d, %d, %d, %d\n", health, hurtTime, deathTime, attackTime);
}
