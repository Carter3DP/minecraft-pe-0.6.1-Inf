#include "BoatItem.h"

#include "../entity/Boat.h"
#include "../entity/player/Player.h"
#include "../level/Level.h"
#include "../level/tile/Tile.h"
#include "../phys/HitResult.h"
#include "../phys/Vec3.h"
#include "../../util/Mth.h"
#include "ItemInstance.h"

ItemInstance* BoatItem::use(ItemInstance* itemInstance, Level* level, Player* player)
{
    const float a = 1.0F;
    const float xRot = player->xRotO + (player->xRot - player->xRotO) * a;
    const float yRot = player->yRotO + (player->yRot - player->yRotO) * a;
    const double x = player->xo + (player->x - player->xo) * a;
    const double y = player->yo + (player->y - player->yo) * a + 1.62 - player->heightOffset;
    const double z = player->zo + (player->z - player->zo) * a;

    const Vec3 from(x, y, z);
    const float yawCos = Mth::cos(-yRot * Mth::DEGRAD - Mth::PI);
    const float yawSin = Mth::sin(-yRot * Mth::DEGRAD - Mth::PI);
    const float pitchCos = -Mth::cos(-xRot * Mth::DEGRAD);
    const float pitchSin = Mth::sin(-xRot * Mth::DEGRAD);
    const double reach = 5.0;
    const Vec3 to = from.add((double)(yawSin * pitchCos) * reach,
                             (double)pitchSin * reach,
                             (double)(yawCos * pitchCos) * reach);

    HitResult hit = level->clip(from, to, true);
    if (!hit.isHit())
        return itemInstance;

    if (hit.type == TILE) {
        int xTile = hit.x;
        int yTile = hit.y;
        int zTile = hit.z;

        if (!level->isClientSide) {
            if (Tile::topSnow && level->getTile(xTile, yTile, zTile) == Tile::topSnow->id)
                --yTile;

            EntityBoat* boat = new EntityBoat(level);
            boat->setPos((double)xTile + 0.5, (double)yTile + 1.0, (double)zTile + 0.5);
            level->addEntity(boat);
        }

        --itemInstance->count;
    }

    return itemInstance;
}
