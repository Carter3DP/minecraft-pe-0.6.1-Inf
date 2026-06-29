#ifndef NET_MINECRAFT_WORLD_LEVEL_LEVELGEN__RandomLevelSource_H__
#define NET_MINECRAFT_WORLD_LEVEL_LEVELGEN__RandomLevelSource_H__

//package net.minecraft.world.level.levelgen;

class Biome;
class Level;
class LevelChunk;

#if 1 || USE_MAP
	#include <map>
	typedef std::map<int, LevelChunk*> ChunkMap;
#else
	#if defined(__APPLE__)
		#include <ext/hash_map>
		namespace std {
			using namespace __gnu_cxx;
		}
	#else
		#include <hash_map>
	#endif
	typedef std::hash_map<int, LevelChunk*> ChunkMap;
#endif


#include "../chunk/ChunkSource.h"
#include "LargeCaveFeature.h"
#include "CanyonFeature.h"
#include "synth/PerlinNoise.h"
#include "../../../SharedConstants.h"

class RandomLevelSource: public ChunkSource
{
    static const double SNOW_CUTOFF;
    static const double SNOW_SCALE;

public:
    static const int CHUNK_HEIGHT = 8;
    static const int CHUNK_WIDTH = 4;

	RandomLevelSource(Level* level, long seed, int version, bool spawnMobs);
	~RandomLevelSource();

	bool hasChunk(int x, int y) override;
    LevelChunk* create(int x, int z) override;
	LevelChunk* getChunk(int xOffs, int zOffs) override;

	void prepareHeights(int xOffs, int zOffs, unsigned char* blocks, /*Biome*/void* biomes, double* temperatures);
    void buildSurfaces(int xOffs, int zOffs, unsigned char* blocks, Biome** biomes);
	void postProcess(ChunkSource* parent, int xt, int zt) override;

    bool tick() override;

    Biome::MobList getMobsAt(const MobCategory& mobCategory, int x, int y, int z) override;

	bool shouldSave() override;
    std::string gatherStats() override;

	//bool save(bool force, ProgressListener progressListener) {
private:
	double* getHeights(double* buffer, int x, int y, int z, int xSize, int ySize, int zSize);
	void calcWaterDepths(ChunkSource* parent, int xt, int zt);

public:
	//Biome** biomes;
	LargeCaveFeature caveFeature;
	CanyonFeature canyonFeature;
	int waterDepths[16+16][16+16];
private:
	ChunkMap chunkMap;

	Random random;
	PerlinNoise lperlinNoise1;
    PerlinNoise lperlinNoise2;
    PerlinNoise perlinNoise1;
    PerlinNoise perlinNoise2;
    PerlinNoise perlinNoise3;
	PerlinNoise scaleNoise;
    PerlinNoise depthNoise;
    PerlinNoise forestNoise;

	Level* level;
	bool spawnMobs;

	double* buffer;
    double sandBuffer[16 * 16];
    double gravelBuffer[16 * 16];
    double depthBuffer[16 * 16];
	double* pnr;
	double* ar;
	double* br;
	double* sr;
	double* dr;
	double* fi;
	double* fis;
    ///*private*/ double[] temperatures;
	double* temperatures; // normally unused like above, but restored this maybe might come handy - shredder
};

class PerformanceTestChunkSource : public ChunkSource
{
	Level* level;
public:
	PerformanceTestChunkSource(Level* level)
	:	ChunkSource(),
		level(level)
	{

	}

	virtual bool hasChunk(int x, int y) override { return true; };
	virtual LevelChunk* getChunk(int x, int z) override { return create(x, z); };

    virtual LevelChunk* create(int x, int z) override;
	virtual void postProcess(ChunkSource* parent, int x, int z) override {};

	virtual bool tick() override { return false; };

	virtual bool shouldSave() override { return false; };

    /**
     * Returns some stats that are rendered when the user holds F3.
     */
	virtual std::string gatherStats() override { return "PerformanceTestChunkSource"; };
};


#endif /*NET_MINECRAFT_WORLD_LEVEL_LEVELGEN__RandomLevelSource_H__*/
