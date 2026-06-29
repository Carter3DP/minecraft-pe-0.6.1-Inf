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
    float hurtTime = (float)boat->boatTimeSinceHit - a;
    float damageTime = (float)boat->boatCurrentDamage - a;
    if (damageTime < 0.0F) {
        damageTime = 0.0F;
    }
    if (hurtTime > 0.0F) {
        glRotatef(sin(hurtTime) * hurtTime * damageTime / 10.0F * (float)boat->boatRockDirection, 1.0F, 0.0F, 0.0F);
    }
    bindTexture("/terrain.png");
    float modelScale = 0.75F;
    glScalef(modelScale, modelScale, modelScale);
    glScalef(1.0F / modelScale, 1.0F / modelScale, 1.0F / modelScale);
    bindTexture("/item/boat.png");
    glScalef(-1.0F, -1.0F, 1.0F);
    modelBoat->render(boat, 0.0F, 0.0F, -0.1F, 0.0F, 0.0F, 0.0625F);
    glPopMatrix();
}

void BoatRenderer::render(Entity* boat, double x, double y, double z, float rot, float a) {
    renderboat((EntityBoat*)boat, x, y, z, rot, a);
}
