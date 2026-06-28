#include "BoatRenderer.h"
#include "../../../util/Mth.h"
#include "../../../world/entity/Boat.h"

BoatRenderer::BoatRenderer(Model* model):
    modelBoat(model)
    {
    shadowRadius = 0.5F;
}

void BoatRenderer::renderboat(EntityBoat* boat, double x, double y, double z, float rot, float a) {
    glPushMatrix();
    glTranslatef((float)x, (float)y, (float)z);
    glRotatef(180.0F - rot, 0.0, 1.0, 0.0);
    float var10 = (float)boat->boatTimeSinceHit - a;
    float var11 = (float)boat->boatCurrentDamage - a;
    if (var11 < 0.0F) {
        var11 = 0.0F;
    }
    if (var10 > 0.0F) {
        glRotatef(sin(var10) * var10 * var11 / 10.0F * (float)boat->boatRockDirection, 1.0F, 0.0F, 0.0F);
    }
    bindTexture("/terrain.png");
    float var12 = 0.75F;
    glScalef(var12, var12, var12);
    glScalef(1.0F / var12, 1.0F / var12, 1.0F / var12);
    bindTexture("/item/boat.png");
    glScalef(-1.0F, -1.0F, 1.0F);
    modelBoat->renderHorrible(0.0F, 0.0F, -0.1F, 0.0F, 0.0F, 0.0625F);
    glPopMatrix();
}

void BoatRenderer::render(Entity* boat, double x, double y, double z, float rot, float a) {
    renderboat((EntityBoat*)boat, x, y, z, rot, a);
}
