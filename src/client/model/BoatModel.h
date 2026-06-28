#ifndef NET_MINECRAFT_CLIENT_MODEL__BoatModel_H__
#define NET_MINECRAFT_CLIENT_MODEL__BoatModel_H__

#include "Model.h"
#include "geom/ModelPart.h"

class Entity;

class BoatModel: public Model
{
    typedef Model super;
public:
    ModelPart boatSide0, boatSide1, boatSide2, boatSide3, boatSide4;

    BoatModel();

    /*@Override*/
    void render(Entity* entity, float time, float r, float bob, float yRot, float xRot, float scale);

    void setupAnim(float time, float r, float bob, float yRot, float xRot, float scale);
};

#endif /*NET_MINECRAFT_CLIENT_MODEL__BoatModel_H__*/
