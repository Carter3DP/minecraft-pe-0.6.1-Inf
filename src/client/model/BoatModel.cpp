#include "BoatModel.h"
#include "../../world/entity/Entity.h"

BoatModel::BoatModel()
:   boatSide0(0, 8),
	boatSide1(0, 0),
	boatSide2(0, 0),
	boatSide3(0, 0),
	boatSide4(0, 0)
{
	int8_t xx = 24;
    int8_t yy = 6;
    int8_t zz = 20;
    int8_t yr = 4;

	boatSide0.addBox((float)(-xx / 2), (float)(-zz / 2 + 2), -3.0F, xx, zz - 4, 4, 0.0F);
    boatSide0.setPos(0.0F, (float)(0 + yr), 0.0F);
    boatSide1.addBox((float)(-xx / 2 + 2), (float)(-yy - 1), -1.0F, xx - 4, yy, 2, 0.0F);
    boatSide1.setPos((float)(-xx / 2 + 1), (float)(0 + yr), 0.0F);
    boatSide2.addBox((float)(-xx / 2 + 2), (float)(-yy - 1), -1.0F, xx - 4, yy, 2, 0.0F);
    boatSide2.setPos((float)(xx / 2 - 1), (float)(0 + yr), 0.0F);
    boatSide3.addBox((float)(-xx / 2 + 2), (float)(-yy - 1), -1.0F, xx - 4, yy, 2, 0.0F);
    boatSide3.setPos(0.0F, (float)(0 + yr), (float)(-zz / 2 + 1));
    boatSide4.addBox((float)(-xx / 2 + 2), (float)(-yy - 1), -1.0F, xx - 4, yy, 2, 0.0F);
    boatSide4.setPos(0.0F, (float)(0 + yr), (float)(zz / 2 - 1));
    boatSide0.xRot = 1.5707964F;
    boatSide1.yRot = 4.712389F;
    boatSide2.yRot = 1.5707964F;
    boatSide3.yRot = 3.1415927F;
}

void BoatModel::render( Entity* entity, float time, float r, float bob, float yRot, float xRot, float scale )
{
	boatSide0.render(scale);
    boatSide1.render(scale);
    boatSide2.render(scale);
    boatSide3.render(scale);
    boatSide4.render(scale);
}

void BoatModel::setupAnim( float time, float r, float bob, float yRot, float xRot, float scale )
{
}

