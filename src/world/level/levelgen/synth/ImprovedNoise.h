#ifndef NET_MINECRAFT_WORLD_LEVEL_LEVELGEN_SYNTH__ImprovedNoise_H__
#define NET_MINECRAFT_WORLD_LEVEL_LEVELGEN_SYNTH__ImprovedNoise_H__

//package net.minecraft.world.level.levelgen.synth;

#include "Synth.h"
class Random;

class ImprovedNoise: public Synth
{
public:
    ImprovedNoise();

    ImprovedNoise(Random* random);
	
	void init(Random* random);

    double noise(double _x, double _y, double _z);

    const double lerp(double t, double a, double b);

    const double grad2(int hash, double x, double z);
    const double grad(int hash, double x, double y, double z);

    double getValue(double x, double y) override;
    double getValue(double x, double y, double z);

    void add(double* buffer, double _x, double _y, double _z, int xSize, int ySize, int zSize, double xs, double ys, double zs, double pow);

    int hashCode();

	double scale;
	double xo, yo, zo;
private:
	int p[512];
};

#endif /*NET_MINECRAFT_WORLD_LEVEL_LEVELGEN_SYNTH__ImprovedNoise_H__*/
