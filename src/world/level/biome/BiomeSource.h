#ifndef NET_MINECRAFT_WORLD_LEVEL_BIOME__BiomeSource_H__
#define NET_MINECRAFT_WORLD_LEVEL_BIOME__BiomeSource_H__

//package net.minecraft.world.level.biome;

#include "../../../util/Random.h"
#include "../levelgen/synth/PerlinNoise.h"

typedef PerlinNoise PerlinSimplexNoise;
class Level;
class Biome;
class ChunkPos;

class BiomeSource
{
protected:
	BiomeSource();
public:
    BiomeSource(Level* level);
	virtual ~BiomeSource();

    double* temperatures;
    double* downfalls; 
    double* noises;

	int lenTemperatures;
	int lenDownfalls;
	int lenNoises;
	int lenBiomes;

    virtual Biome* getBiome(const ChunkPos& chunk);
    virtual Biome* getBiome(int x, int z);

    virtual double getTemperature(int x, int z);

	// Note: The arrays returned here are temporary in the meaning that their
	//       contents might change in the future. If you need to SAVE the
	//       values, do a shallow copy to an array of your own.
	virtual double* getTemperatureBlock(double* temperatures,  int x, int z, int w, int h);
    virtual double* getDownfallBlock(double* downfalls, int x, int z, int w, int h);
	virtual Biome** getBiomeBlock(int x, int z, int w, int h);

private:
	virtual Biome** getBiomeBlock(Biome** biomes, int x, int z, int w, int h);

	Biome** biomes;
	PerlinSimplexNoise* temperatureMap;
	PerlinSimplexNoise* downfallMap;
	PerlinSimplexNoise* noiseMap;

	Random rndTemperature;
	Random rndDownfall;
	Random rndNoise;

	static const double zoom;

	static const double tempScale;
	static const double downfallScale;
	static const double noiseScale;
};

#endif /*NET_MINECRAFT_WORLD_LEVEL_BIOME__BiomeSource_H__*/
