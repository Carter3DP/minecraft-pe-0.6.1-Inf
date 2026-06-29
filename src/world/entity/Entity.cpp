#include "Entity.h"
#include "EntityPos.h"
#include "../level/Level.h"
#include "../level/tile/LiquidTile.h"
#include "item/ItemEntity.h"
#include "../item/ItemInstance.h"
#include "../../nbt/CompoundTag.h"
#include "../../util/PerfTimer.h"
#include "../../network/RakNetInstance.h"
#include "../../network/packet/RidePacket.h"

int
	Entity::entityCounter = 0;
Random
	Entity::sharedRandom(getEpochTimeS());

Entity::Entity( Level* level )
:	level(level),
	viewScale(1.0f),
	blocksBuilding(false),
	onGround(false),
	wasInWater(false),
	collision(false),
	hurtMarked(false),
	slide(true),
	isStuckInWeb(false),
	removed(false),
	reallyRemoveIfPlayer(false),
	canRemove(true), //@todo: remove
	noPhysics(false),
	firstTick(true),
	riddenByEntity(NULL),
	ridingEntity(NULL),

	bbWidth(0.6f),
	bbHeight(1.8f),
	heightOffset(0 / 16.0f),
	bb(0,0,0,0,0,0),

	ySlideOffset(0),
	fallDistance(0),
	footSize(0),
	invulnerableTime(0),
	pushthrough(0),
	airCapacity(TOTAL_AIR_SUPPLY),
	airSupply(TOTAL_AIR_SUPPLY),

	xOld(0),yOld(0),zOld(0),
	horizontalCollision(false), verticalCollision(false),

	x(0), y(0), z(0),
	xo(0),yo(0),zo(0),xd(0),yd(0),zd(0),
	xRot(0), yRot(0),
	xRotO(0), yRotO(0),

	xChunk(0), yChunk(0), zChunk(0),
	inChunk(false),

	fireImmune(false),
	onFire(0),
	flameTime(1),
	walkDist(0), walkDistO(0),
	tickCount(0),
	entityRendererId(ER_DEFAULT_RENDERER),
	nextStep(1),
	makeStepSound(true),
	invisible(false)
{
	_init();

	entityId = ++entityCounter;

	//ref = Ref<Entity>::create(this);

	setPos(0, 0, 0);
}

Entity::~Entity() {
	//if (ref->isUnique())
	//	delete ref;
}

SynchedEntityData* Entity::getEntityData() {
	return NULL;
}
const SynchedEntityData* Entity::getEntityData() const {
	return NULL;
}

bool Entity::isInWall() {
    int xt = Mth::floor(x);
    int yt = Mth::floor(y + getHeadHeight());
    int zt = Mth::floor(z);
    return level->isSolidBlockingTile(xt, yt, zt);
}

void Entity::resetPos(bool clearMore) {
    if (level == NULL) return;
    while (y > 0) {
        setPos(x, y, z);
        if (level->getCubes(this, bb).size() == 0) break;
        y += 1;
    }

    xd = yd = zd = 0;
    xRot = 0;
}

bool Entity::isInWater() {
    return level->checkAndHandleWater(bb.grow(0, -0.4f, 0), Material::water, this);
}

bool Entity::isInLava() {
	return level->containsMaterial(bb.grow(-0.1f, -0.4f, -0.1f), Material::lava);
}

bool Entity::isFree(double xa, double ya, double za, double grow) {
    AABB box = bb.grow(grow, grow, grow).cloneMove(xa, ya, za);
    const std::vector<AABB>& aABBs = level->getCubes(this, box);
    if (aABBs.size() > 0) return false;
    if (level->containsAnyLiquid(box)) return false;
    return true;
}

bool Entity::isFree(double xa, double ya, double za) {
    AABB box = bb.cloneMove(xa, ya, za);
	const std::vector<AABB>& aABBs = level->getCubes(this, box);
    if (aABBs.size() > 0) return false;
    if (level->containsAnyLiquid(box)) return false;
    return true;
}

//static void __attribute__((noinline)) setPositionFromBbox(Entity* e) { // @RPI
//    const AABB& bb = e->bb;
//    e->x = (e->bb.x0 + e->bb.x1) / 2.0f;
//    e->y = bb.y0 + e->heightOffset - e->ySlideOffset;
//    e->z = (bb.z0 + bb.z1) / 2.0f;
//}

/*public*/
void Entity::move(double xa, double ya, double za) {
	//if (std::abs(xa) + std::abs(ya) + std::abs(za) < 0.00001f) //@RPI
	//	return;

	if (noPhysics) {
        bb.move(xa, ya, za);
        x = (bb.x0 + bb.x1) / 2.0f;
        y = bb.y0 + heightOffset - ySlideOffset;
        z = (bb.z0 + bb.z1) / 2.0f;
        return;
    }

	TIMER_PUSH("move");

    double xo = x;
    double zo = z;

	if (isStuckInWeb) {
		isStuckInWeb = false;

		xa *= .25f;
		ya *= .05f;
		za *= .25f;
		xd = .0f;
		yd = .0f;
		zd = .0f;
	}

    double xaOrg = xa;
    double yaOrg = ya;
    double zaOrg = za;

    AABB bbOrg = bb;

    bool sneaking = onGround && isSneaking();

    if (sneaking) {
        double d = 0.05;
        while (xa != 0 && level->getCubes(this, bb.cloneMove(xa, -1.0, 0)).empty()) {
            if (xa < d && xa >= -d) xa = 0;
            else if (xa > 0) xa -= d;
            else xa += d;
            xaOrg = xa;
        }
        while (za != 0 && level->getCubes(this, bb.cloneMove(0, -1.0, za)).empty()) {
            if (za < d && za >= -d) za = 0;
            else if (za > 0) za -= d;
            else za += d;
            zaOrg = za;
        }

		while (xa != 0 && za != 0 && level->getCubes(this, bb.cloneMove(xa, -1.0, za)).empty()) {
			if (xa < d && xa >= -d) xa = 0;
			else if (xa > 0) xa -= d;
			else xa += d;
			if (za < d && za >= -d) za = 0;
			else if (za > 0) za -= d;
			else za += d;
			xaOrg = xa;
			zaOrg = za;
		}

    }

    std::vector<AABB>& aABBs = level->getCubes(this, bb.expand(xa, ya, za));

	// LAND FIRST, then x and z
    for (unsigned int i = 0; i < aABBs.size(); i++)
        ya = aABBs[i].clipYCollide(bb, ya);
    bb.move(0, ya, 0);

	if (!slide && yaOrg != ya) {
		xa = ya = za = 0;
	}

    bool og = onGround || (yaOrg != ya && yaOrg < 0);

    for (unsigned int i = 0; i < aABBs.size(); i++)
        xa = aABBs[i].clipXCollide(bb, xa);

    bb.move(xa, 0, 0);

    if (!slide && xaOrg != xa) {
        xa = ya = za = 0;
    }

    for (unsigned int i = 0; i < aABBs.size(); i++)
        za = aABBs[i].clipZCollide(bb, za);
    bb.move(0, 0, za);

    if (!slide && zaOrg != za) {
        xa = ya = za = 0;
    }

    if (footSize > 0 && og && (ySlideOffset < 0.05f) && ((xaOrg != xa) || (zaOrg != za))) {

		double xaN = xa;
        double yaN = ya;
        double zaN = za;

        xa = xaOrg;
        ya = footSize;
        za = zaOrg;

        AABB normal = bb;
        bb.set(bbOrg);
        aABBs = level->getCubes(this, bb.expand(xa, ya, za));

        // LAND FIRST, then x and z
        for (unsigned int i = 0; i < aABBs.size(); i++)
            ya = aABBs[i].clipYCollide(bb, ya);
        bb.move(0, ya, 0);

        if (!slide && yaOrg != ya) {
            xa = ya = za = 0;
        }

		for (unsigned int i = 0; i < aABBs.size(); i++)
            xa = aABBs[i].clipXCollide(bb, xa);
        bb.move(xa, 0, 0);

        if (!slide && xaOrg != xa) {
            xa = ya = za = 0;
        }

        for (unsigned int i = 0; i < aABBs.size(); i++)
            za = aABBs[i].clipZCollide(bb, za);
        bb.move(0, 0, za);

        if (!slide && zaOrg != za) {
            xa = ya = za = 0;
        }

        if (xaN * xaN + zaN * zaN >= xa * xa + za * za) {
            xa = xaN;
            ya = yaN;
            za = zaN;
            bb.set(normal);
        } else {
            ySlideOffset += 0.5f;
        }
    }

	TIMER_POP_PUSH("rest");

	x = (bb.x0 + bb.x1) / 2.0f;
    y = bb.y0 + heightOffset - ySlideOffset;
    z = (bb.z0 + bb.z1) / 2.0f;

    horizontalCollision = (xaOrg != xa) || (zaOrg != za);
    verticalCollision = (yaOrg != ya);
    onGround = yaOrg != ya && yaOrg < 0;
    collision = horizontalCollision || verticalCollision;
    checkFallDamage(ya, onGround);

    if (xaOrg != xa) xd = 0;
    if (yaOrg != ya) yd = 0;
    if (zaOrg != za) zd = 0;

    double xm = x - xo;
    double zm = z - zo;

    if (makeStepSound && !sneaking) {
        walkDist += Mth::sqrt(xm * xm + zm * zm) * 0.6f;
        int xt = Mth::floor(x);
        int yt = Mth::floor(y - 0.2f - this->heightOffset);
        int zt = Mth::floor(z);

        int t = level->getTile(xt, yt, zt);
        if (t == 0) {
            int under = level->getTile(xt, yt-1, zt);
            if (Tile::fence->id == under || Tile::fenceGate->id == under) {
                t = under;
            }
        }

        if (walkDist > nextStep && t > 0) {
            nextStep = ((int) walkDist) + 1;
            playStepSound(xt, yt, zt, t);
            //Tile::tiles[t]->stepOn(level, xt, yt, zt, this); //@todo: step
        }
    }

    int x0 = Mth::floor(bb.x0);
    int y0 = Mth::floor(bb.y0);
    int z0 = Mth::floor(bb.z0);
    int x1 = Mth::floor(bb.x1);
    int y1 = Mth::floor(bb.y1);
    int z1 = Mth::floor(bb.z1);

    if (level->hasChunksAt(x0, y0, z0, x1, y1, z1)) {
        for (int x = x0; x <= x1; x++)
            for (int y = y0; y <= y1; y++)
                for (int z = z0; z <= z1; z++) {
                    int t = level->getTile(x, y, z);
                    if (t > 0) {
                        Tile::tiles[t]->entityInside(level, x, y, z, this);
                    }
                }
    }

    ySlideOffset *= 0.4f;

    bool water = this->isInWater();
    if (level->containsFireTile(bb)) {
        burn(1);
        if (!water) {
            onFire++;
            if (onFire == 0) onFire = 20 * 15;
        }
    } else {
        if (onFire <= 0) {
            onFire = -flameTime;
        }
    }

    if (water && onFire > 0) {
        //level.playSound(this-> "random.fizz", 0.7f, 1.6f + (random.nextFloat() - random.nextFloat()) * 0.4f);
        onFire = -flameTime;
    }

	TIMER_POP();
}

void Entity::makeStuckInWeb() {
	isStuckInWeb = true;
	fallDistance = 0;
}

/*public virtual*/
bool Entity::isUnderLiquid(const Material* material) {
    double yp = y + getHeadHeight();
    int xt = Mth::floor(x);
    int yt = Mth::floor(yp);
    int zt = Mth::floor(z);
    int t = level->getTile(xt, yt, zt);
    if (t != 0 && Tile::tiles[t]->material == material) {
        float hh = LiquidTile::getHeight(level->getData(xt, yt, zt)) - 1 / 9.0f;
        double h = yt + 1 - hh;
        return yp < h;
    }
    return false;
}

/*protected virtual*/
void Entity::setPos(EntityPos* pos)
{
    if (pos->move) setPos(pos->x, pos->y, pos->z);
    else setPos(x, y, z);

    if (pos->rot) setRot(pos->yRot, pos->xRot);
    else setRot(yRot, xRot);
}

void Entity::setPos( double x, double y, double z )
{
	this->x = x;
	this->y = y;
	this->z = z;
	double w = bbWidth / 2;
	double h = bbHeight;
	bb.set(x - w, y - heightOffset + ySlideOffset, z - w, x + w, y - heightOffset + ySlideOffset + h, z + w);
}

/*virtual*/
float Entity::getBrightness(float a) {
    int xTile = Mth::floor(x);

    float hh = (bb.y1 - bb.y0) * 0.66f;
    int yTile = Mth::floor(y - this->heightOffset + hh);
    int zTile = Mth::floor(z);
    if (level->hasChunksAt(Mth::floor(bb.x0), Mth::floor(bb.y0), Mth::floor(bb.z0), Mth::floor(bb.x1), Mth::floor(bb.y1), Mth::floor(bb.z1))) {
        return level->getBrightness(xTile, yTile, zTile);
    }
    return 0;
}

bool Entity::operator==( Entity& rhs )
{
	return entityId == rhs.entityId;
}

int Entity::hashCode()
{
	return entityId;
}

void Entity::remove()
{
	removed = true;
}

void Entity::setSize( float w, float h )
{
	bbWidth = w;
	bbHeight = h;
}

void Entity::setRot( float yRot, float xRot )
{
	this->yRot = yRotO = yRot;
	this->xRot = xRotO = xRot;
}

void Entity::turn( float xo, float yo )
{
	float xRotOld = xRot;
	float yRotOld = yRot;

	yRot += xo * 0.15f;
	xRot -= yo * 0.15f;
	if (xRot < -90) xRot = -90;
	if (xRot > 90) xRot = 90;

	xRotO += xRot - xRotOld;
	yRotO += yRot - yRotOld;
}

void Entity::interpolateTurn( float xo, float yo )
{
	yRot += xo * 0.15f;
	xRot -= yo * 0.15f;
	if (xRot < -90) xRot = -90;
	if (xRot > 90) xRot = 90;
}

void Entity::tick()
{
	baseTick();
}

void Entity::setOnFire(int numberOfSeconds)
{
    int newValue = numberOfSeconds * 20;
    if (onFire < newValue)
    {
        onFire = newValue;
    }
}

void Entity::baseTick()
{
	TIMER_PUSH("entityBaseTick");

	tickCount++;
	walkDistO = walkDist;
	xo = x;
	yo = y;
	zo = z;
	xRotO = xRot;
	yRotO = yRot;

	if (isInWater()) {
		if (!wasInWater && !firstTick) {
		    float speed = sqrt(xd * xd * 0.2f + yd * yd + zd * zd * 0.2f) * 0.2f;
		    if (speed > 1) speed = 1;
		    level->playSound(this, "random.splash", speed, 1 + (sharedRandom.nextFloat() - sharedRandom.nextFloat()) * 0.4f);
		    double yt = std::floor(bb.y0);
		    for (int i = 0; i < 1 + bbWidth * 20; i++) {
		        float xo = (sharedRandom.nextFloat() * 2 - 1) * bbWidth;
		        float zo = (sharedRandom.nextFloat() * 2 - 1) * bbWidth;
		        level->addParticle(PARTICLETYPE(bubble), x + xo, yt + 1, z + zo, xd, yd - sharedRandom.nextFloat() * 0.2f, zd);
		    }
		    //for (int i = 0; i < 1 + bbWidth * 20; i++) {
		    //    float xo = (sharedRandom.nextFloat() * 2 - 1) * bbWidth;
		    //    float zo = (sharedRandom.nextFloat() * 2 - 1) * bbWidth;
		    //    level->addParticle(PARTICLETYPE(splash), x + xo, yt + 1, z + zo, xd, yd, zd);
		    //}
		}
		fallDistance = 0;
		wasInWater = true;
		onFire = 0;
	} else {
		wasInWater = false;
	}

	if (level->isClientSide) {
	    onFire = 0;
	} else {
	    if (onFire > 0) {
	        if (fireImmune) {
	            onFire -= 4;
	            if (onFire < 0) onFire = 0;
	        } else {
	            if (onFire % 20 == 0) {
	                hurt(NULL, 1);
	            }
	            onFire--;
	        }
	    }
	}

	if (isInLava()) {
	    lavaHurt();
	}

	if (y < -64) {
		outOfWorld();
	}

	//if (!level->isOnline) {
	//    setSharedFlag(FLAG_ONFIRE, onFire > 0);
	//}

	firstTick = false;
	TIMER_POP();
}

void Entity::outOfWorld()
{
	remove();
}

void Entity::checkFallDamage( float ya, bool onGround )
{
	if (onGround) {
		if (fallDistance > 0) {
			if(isMob()) {
				int xt = Mth::floor(x);
				int yt = Mth::floor(y - 0.2f - heightOffset);
				int zt = Mth::floor(z);
				int t = level->getTile(xt, yt, zt);
				if (t == 0 && level->getTile(xt, yt - 1, zt) == Tile::fence->id) {
					t = level->getTile(xt, yt - 1, zt);
				}

				if (t > 0) {
					Tile::tiles[t]->fallOn(level, xt, yt, zt, this, fallDistance);
				}
			}
			causeFallDamage(fallDistance);
			fallDistance = 0;
		}
	} else {
		if (ya < 0) fallDistance -= ya;
	}
}

void Entity::causeFallDamage( float fallDamage2 )
{
}

float Entity::getHeadHeight()
{
	return 0;
}

void Entity::moveRelative( double xa, double za, float speed )
{
	double dist = sqrt(xa * xa + za * za);
	if (dist < 0.01f) return;

	if (dist < 1) dist = 1;
	dist = speed / dist;
	xa *= dist;
	za *= dist;

	float sin_ = (float) sin(yRot * Mth::PI / 180);
	float cos_ = (float) cos(yRot * Mth::PI / 180);

	xd += xa * cos_ - za * sin_;
	zd += za * cos_ + xa * sin_;
}

void Entity::setLevel( Level* level )
{
	this->level = level;
}

void Entity::moveTo( double x, double y, double z, float yRot, float xRot )
{
	this->xOld = this->xo = this->x = x;
	this->yOld = this->yo = this->y = y + heightOffset;
	this->zOld = this->zo = this->z = z;
	this->yRot = this->yRotO = yRot;
	this->xRot = this->xRotO = xRot;
	this->setPos(this->x, this->y, this->z);
}

double Entity::distanceTo( Entity* e )
{
	double xd = x - e->x;
	double yd = y - e->y;
	double zd = z - e->z;
	return sqrt(xd * xd + yd * yd + zd * zd);
}

double Entity::distanceTo( double x2, double y2, double z2 )
{
	double xd = (x - x2);
	double yd = (y - y2);
	double zd = (z - z2);
	return sqrt(xd * xd + yd * yd + zd * zd);
}

double Entity::distanceToSqr( double x2, double y2, double z2 )
{
	double xd = (x - x2);
	double yd = (y - y2);
	double zd = (z - z2);
	return xd * xd + yd * yd + zd * zd;
}

double Entity::distanceToSqr( Entity* e )
{
	double xd = x - e->x;
	double yd = y - e->y;
	double zd = z - e->z;
	return xd * xd + yd * yd + zd * zd;
}

void Entity::playerTouch( Player* player )
{
}

void Entity::push( Entity* e )
{
	if (e->riddenByEntity != this && e->ridingEntity != this) {
		double xa = e->x - x;
		double za = e->z - z;

		double dd = Mth::absMax(xa, za);

		if (dd >= 0.01f) {
			dd = sqrt(dd);
			xa /= dd;
			za /= dd;

			double pow = 1 / dd;
			if (pow > 1) pow = 1;
			xa *= pow;
			za *= pow;

			xa *= 0.05f;
			za *= 0.05f;

			xa *= 1 - pushthrough;
			za *= 1 - pushthrough;

			this->push(-xa, 0, -za);
			e->push(xa, 0, za);
		}
	}
}

void Entity::push( double xa, double ya, double za )
{
	xd += xa;
	yd += ya;
	zd += za;
}

void Entity::markHurt()
{
	this->hurtMarked = true;
}

bool Entity::hurt( Entity* source, int damage )
{
	markHurt();
	return false;
}

void Entity::reset() {
	this->_init();
}
void Entity::_init() {
	xo = xOld = x;
	yo = yOld = y;
	zo = zOld = z;
	xRotO	= xRot;
	yRotO	= yRot;
	onFire  = 0;
	removed = false;
	fallDistance = 0;
}

bool Entity::intersects( double x0, double y0, double z0, double x1, double y1, double z1 )
{
	return bb.intersects(x0, y0, z0, x1, y1, z1);
}

bool Entity::isPickable()
{
	return false;
}

bool Entity::isPushable()
{
	return false;
}

bool Entity::isShootable()
{
	return false;
}

void Entity::awardKillScore( Entity* victim, int score )
{
}

bool Entity::shouldRender( Vec3& c )
{
	if (invisible) return false;
	double xd = x - c.x;
	double yd = y - c.y;
	double zd = z - c.z;
	double distance = xd * xd + yd * yd + zd * zd;
	return shouldRenderAtSqrDistance(distance);
}

bool Entity::shouldRenderAtSqrDistance( double distance )
{
	double size = bb.getSize();
	size *= 64.0f * viewScale;
	return distance < size * size;
}

bool Entity::isCreativeModeAllowed()
{
	return false;
}

float Entity::getShadowHeightOffs()
{
	return bbHeight / 2;
}

bool Entity::isAlive()
{
	return !removed;
}

bool Entity::interact( Player* player )
{
	return false;
}

void Entity::lerpTo( double x, double y, double z, float yRot, float xRot, int steps )
{
	setPos(x, y, z);
	setRot(yRot, xRot);
}

float Entity::getPickRadius()
{
	return 0.1f;
}

void Entity::lerpMotion( double xd, double yd, double zd )
{
	this->xd = xd;
	this->yd = yd;
	this->zd = zd;
}

void Entity::animateHurt()
{
}

void Entity::setEquippedSlot( int slot, int item, int auxValue )
{
}

bool Entity::isSneaking()
{
	return false;
}

bool Entity::isPlayer()
{
	return false;
}


void Entity::lavaHurt() {
    if (fireImmune) {
    } else {
        hurt(NULL, 4);
        onFire = 30 * SharedConstants::TicksPerSecond;
    }
}

//   AABB getCollideBox() {
//       return NULL;
//   }

void Entity::burn(int dmg) {
    if (!fireImmune) {
        hurt(NULL, dmg);
    }
}

//   std::string getTexture() {
//       return NULL;
//   }

bool Entity::save(CompoundTag* entityTag) {
	int id = getEntityTypeId();

    if (removed || id == 0) {
        return false;
    }
    entityTag->putInt("id", id);
    saveWithoutId(entityTag);
    return true;
}

void Entity::saveWithoutId(CompoundTag* entityTag) {
	entityTag->put("Pos", ListTagDoubleAdder (x) (y) (z).tag);
    entityTag->put("Motion", ListTagDoubleAdder (xd) (yd) (zd).tag);
    entityTag->put("Rotation", ListTagFloatAdder (yRot) (xRot).tag);

    entityTag->putFloat("FallDistance", fallDistance);
    entityTag->putShort("Fire", (short) onFire);
    entityTag->putShort("Air", (short) airSupply);
    entityTag->putBoolean("OnGround", onGround);

    addAdditonalSaveData(entityTag);
}

bool Entity::load( CompoundTag* tag )
{
    ListTag* pos = tag->getList("Pos");
    ListTag* motion = tag->getList("Motion");
    ListTag* rotation = tag->getList("Rotation");
    setPos(0, 0, 0);

    xd = motion->getDouble(0);
    yd = motion->getDouble(1);
    zd = motion->getDouble(2);

    if (Mth::abs(xd) > 10.0) {
        xd = 0;
    }
    if (Mth::abs(yd) > 10.0) {
        yd = 0;
    }
    if (Mth::abs(zd) > 10.0) {
        zd = 0;
    }

    double xx = pos->getDouble(0);
    double yy = pos->getDouble(1);
    double zz = pos->getDouble(2);

    xo = xOld = x = xx;
    yo = yOld = y = yy;
    zo = zOld = z = zz;

    yRotO = yRot = fmod( rotation->getFloat(0), 360.0f);
    xRotO = xRot = fmod( rotation->getFloat(1), 360.0f);

    fallDistance= tag->getFloat("FallDistance");
    onFire		= tag->getShort("Fire");
    airSupply	= tag->getShort("Air");
    onGround	= tag->getBoolean("OnGround");

    setPos(x, y, z);

    readAdditionalSaveData(tag);
	return (tag->errorState == 0);
}

//   /*protected*/ const String getEncodeId() {
//       return EntityIO.getEncodeId(this->;
//   }

ItemEntity* Entity::spawnAtLocation(int resource, int count) {
	return spawnAtLocation(resource, count, 0);
}

ItemEntity* Entity::spawnAtLocation(int resource, int count, float yOffs) {
	return spawnAtLocation(new ItemInstance(resource, count, 0), yOffs);
}

ItemEntity* Entity::spawnAtLocation(ItemInstance* itemInstance, float yOffs) {
	ItemEntity* ie = new ItemEntity(level, x, y + yOffs, z, *itemInstance);

	{ //@todo:itementity
		delete itemInstance;
		itemInstance = NULL;
	}

	ie->throwTime = 10;
	level->addEntity(ie);
	return ie;
}

bool Entity::isOnFire() {
	return onFire > 0;// || getSharedFlag(FLAG_ONFIRE);
}

bool Entity::interactPreventDefault() {
	return false;
}

//   AABB getCollideAgainstBox(Entity entity) {
//       return NULL;
//   }

//   Vec3 getLookAngle() {
//       return NULL;
//   }

//   void prepareCustomTextures() {
//   }

//   ItemInstance[] getEquipmentSlots() {
//       return NULL;
//   }

bool Entity::isItemEntity() {
	return false;
}

bool Entity::isHangingEntity() {
	return false;
}

int Entity::getAuxData() {
	return 0;
}

void Entity::playStepSound( int xt, int yt, int zt, int t ) {
    const Tile::SoundType* soundType = Tile::tiles[t]->soundType;
    if (level->getTile(xt, yt + 1, zt) == Tile::topSnow->id) {
        soundType = Tile::topSnow->soundType;
        level->playSound(this, soundType->getStepSound(), soundType->getVolume() * 0.25f, soundType->getPitch()); // was * 0.15f
    } else if (!Tile::tiles[t]->material->isLiquid()) {
        level->playSound(this, soundType->getStepSound(), soundType->getVolume() * 0.25f, soundType->getPitch());
    }
}

double Entity::getMountedYOffset() {
        return (double)bbHeight * 0.75;
}

 void Entity::mountEntity(Entity* ent) {
		Entity* oldVehicle = ridingEntity;
        entityRiderPitchDelta = 0;
        entityRiderYawDelta = 0;
        if (ent == NULL) {
            if (ridingEntity != NULL) {
                moveTo(ridingEntity->x, ridingEntity->bb.y0 + (double)ridingEntity->bbHeight, ridingEntity->z, yRot, xRot);
                ridingEntity->riddenByEntity = NULL;
            }

            ridingEntity = NULL;
        } else if (ridingEntity == ent) {
            ridingEntity->riddenByEntity = NULL;
            ridingEntity = NULL;
            moveTo(ent->x, ent->bb.y0 + (double)ent->bbHeight, ent->z, yRot, xRot);
        } else {
            if (ridingEntity != NULL) {
                ridingEntity->riddenByEntity = NULL;
            }

            if (ent->riddenByEntity != NULL) {
                ent->riddenByEntity->ridingEntity = NULL;
            }

            ridingEntity = ent;
            ent->riddenByEntity = this;
        }
		if (!level->isClientSide && oldVehicle != ridingEntity) {
			RidePacket packet(this, ridingEntity);
			level->raknetInstance->send(packet);
		}
    }

	void Entity::updateRidden() {
        if (ridingEntity->removed) {
            ridingEntity = NULL;
        } else {
            xd = 0.0;
            yd = 0.0;
            zd = 0.0;
            tick();
            if (ridingEntity != NULL) {
                ridingEntity->updateRiderPosition();
                entityRiderYawDelta += (double)(ridingEntity->yRot - ridingEntity->yRotO);

                for(entityRiderPitchDelta += (double)(ridingEntity->yRot - ridingEntity->yRotO); entityRiderYawDelta >= 180.0; entityRiderYawDelta -= 360.0) {
                }

                while(entityRiderYawDelta < -180.0) {
                    entityRiderYawDelta += 360.0;
                }

                while(entityRiderPitchDelta >= 180.0) {
                    entityRiderPitchDelta -= 360.0;
                }

                while(entityRiderPitchDelta < -180.0) {
                    entityRiderPitchDelta += 360.0;
                }

                double var1 = entityRiderYawDelta * 0.5;
                double var3 = entityRiderPitchDelta * 0.5;
                float var5 = 10.0;
                if (var1 > (double)var5) {
                    var1 = (double)var5;
                }

                if (var1 < (double)(-var5)) {
                    var1 = (double)(-var5);
                }

                if (var3 > (double)var5) {
                    var3 = (double)var5;
                }

                if (var3 < (double)(-var5)) {
                    var3 = (double)(-var5);
                }

                entityRiderYawDelta -= var1;
                entityRiderPitchDelta -= var3;
                yRot = (float)((double)yRot + var1);
                xRot = (float)((double)xRot + var3);
            }
        }
    }

void Entity::updateRiderPosition() {
		riddenByEntity->setPos(x, y + getMountedYOffset() + riddenByEntity->heightOffset, z);
    }

bool Entity::isRiding() {
        return ridingEntity != NULL;
    }
