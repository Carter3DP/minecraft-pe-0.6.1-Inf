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
    if (e->level->isClientSide && e->isAlive()) {
        boatRockDirection = -boatRockDirection;
        boatTimeSinceHit = 10;
        boatCurrentDamage += damage * 10;
        markHurt();
        if (boatCurrentDamage > 40) {
            if (riddenByEntity != NULL) {
                riddenByEntity->mountEntity(this);
            }
            int drops;

            //wood drops
            for(drops = 0; drops < 3; ++drops) {
                e->spawnAtLocation(Tile::wood->id, 1);
            }

            //stick drops. Eventually will remove, I hated this
            for(drops = 0; drops < 2; ++drops) {
                e->spawnAtLocation(Item::stick->id, 1);
            }
            e->remove();
        }
        return true;
    } else {
        return true;
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
    double var2 = 0.0;
    for(int i = 0; i < bboxes; ++i) {
        double var5 = bb.y0 + (bb.y1 - bb.y0) * (double)(i + 0) / (double)bboxes - 0.125;
        double var7 = bb.y0 + (bb.y1 - bb.y0) * (double)(i + 1) / (double)bboxes - 0.125;
        AABB boundbox = AABB(bb.x0, var5, bb.z0, bb.x1, var7, bb.z1);
        if (level->containsMaterial(boundbox, Material::water)) {
            var2 += 1.0 / (double)bboxes;
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
        if (var2 < 1.0) {
            xv = var2 * 2.0 - 1.0;
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
                double var13 = (double)(sharedRandom.nextFloat() * 2.0 - 1.0);
                double var15 = (double)(sharedRandom.nextInt(2) * 2 - 1) * 0.7;
                double var17;
                double var19;
                if (sharedRandom.nextBoolean()) {
                    var17 = x - zv * var13 * 0.8 + rot * var15;
                    var19 = z - rot * var13 * 0.8 - zv * var15;
                    //todo: add splash particle
                    //level->addParticle("splash", var17, y - 0.125, var19, xd, yd, zd);
                } else {
                    var17 = x + zv + rot * var13 * 0.7;
                    var19 = z + rot - zv * var13 * 0.7;
                    //todo: add splash particle
                    //level->addParticle("splash", var17, y - 0.125, var19, xd, yd, zd);
                }
            }
        }
        else if (horizontalCollision && yv > 0.15) {
            if (level->isClientSide){
                remove();
                int var22;
                for(var22 = 0; var22 < 3; ++var22) {
                    spawnAtLocation(Tile::wood->id, 1);
                }
                for(var22 = 0; var22 < 2; ++var22) {
                    spawnAtLocation(Item::stick->id, 1);
                }
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
        double var23 = zo - z;
        if (rot * rot + var23 * var23 > 0.001) {
            zv = (double)((float)(atan2(var23, rot) * 180.0 / 3.141592653589793));
        }
        double var14;
        for(var14 = zv - (double)yRot; var14 >= 180.0; var14 -= 360.0) {
        }
        while(var14 < -180.0) {
            var14 += 360.0;
        }
        if (var14 > 20.0) {
            var14 = 20.0;
        }
        if (var14 < -20.0) {
            var14 = -20.0;
        }
        yRot = (float)((double)yRot + var14);
        setRot(yRot, xRot);
        EntityList& entities = level->getEntities(this, bb.expand(0.20000000298023224, 0.0, 0.20000000298023224));
        int var24;
        if (!entities.empty() && entities.size() > 0) {
            for(var24 = 0; var24 < entities.size(); ++var24) {
                Entity* var18 = entities[var24];
                if (var18 != riddenByEntity && var18->isPushable() && dynamic_cast<EntityBoat*>(var18) != nullptr) {
                    var18->push(this);
                }
            }
        }
        //snow layer breaking behavior
        /*for(var24 = 0; var24 < 4; ++var24) {
            int var25 = floor(x + ((double)(var24 % 2) - 0.5) * 0.8);
            int var26 = floor(y);
            int var20 = floor(z + ((double)(var24 / 2) - 0.5) * 0.8);
            if (level->containsMaterial(boundbox, ) == Tile::snow->id) {
                level->setBlockWithNotify(var25, var26, var20, 0);
            }
        }*/
        if (riddenByEntity != NULL && riddenByEntity->removed) {
            riddenByEntity = NULL;
        }
    }
}
void EntityBoat::updateRiderPosition() {
    if (riddenByEntity != NULL) {
        double var1 = cos((double)yRot * 3.141592653589793 / 180.0) * 0.4;
        double var3 = sin((double)yRot * 3.141592653589793 / 180.0) * 0.4;
        riddenByEntity->setPos(x + var1, y + getMountedYOffset() + riddenByEntity->heightOffset, z + var3);
    }
}

bool EntityBoat::save(CompoundTag* entityTag) {
}

bool EntityBoat::load(CompoundTag* entityTag) {
}

float EntityBoat::getShadowHeightOffs() {
    return 0.0;
}

bool EntityBoat::interact(Player* player) {
    if (riddenByEntity != NULL && dynamic_cast<Player*>(riddenByEntity) != nullptr && riddenByEntity != player) {
        return true;
    } else {
        if (level->isClientSide) {
            player->mountEntity(this);
        }
        return true;
    }
}

int EntityBoat::getEntityTypeId() {
    return EntityTypes::boatEntity;
}