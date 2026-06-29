#if !defined(DEMO_MODE) && !defined(APPLE_DEMO_PROMOTION)

#ifndef NET_MINECRAFT_WORLD_LEVEL_STORAGE__ExternalFileLevelStorage_H__
#define NET_MINECRAFT_WORLD_LEVEL_STORAGE__ExternalFileLevelStorage_H__

//package net.minecraft.world.level.storage;

#include <vector>
#include <list>
#include <map>

//#include "com/mojang/nbt/CompoundTag.h"
#include "LevelStorage.h"
#include "../chunk/storage/ChunkStorage.h"

class Player;
class Dimension;
class RegionFile;


typedef struct UnsavedLevelChunk
{
	int				pos;
	RakNet::TimeMS	addedToList;
	LevelChunk*		chunk;
} UnsavedLevelChunk;

typedef std::list<UnsavedLevelChunk> UnsavedChunkList;
typedef std::map<std::string, RegionFile*> RegionFileMap;

/*public*/
class ExternalFileLevelStorage:
		public LevelStorage,
		public ChunkStorage
		//public PlayerIO
{
public:
    ExternalFileLevelStorage(const std::string& levelId, const std::string& fullPath);
	virtual ~ExternalFileLevelStorage();

    LevelData* prepareLevel(Level* level) override;

	//throws LevelConflictException
    void checkSession() {}

    ChunkStorage* createChunkStorage(Dimension* dimension) override { return this; }

    void saveLevelData(LevelData& levelData, std::vector<Player*>* players) override;
    // PlayerIO getPlayerIO() { return this; }
	// CompoundTag loadPlayerDataTag(std::string playerName) { return NULL; }

    void closeAll() override {}

	static bool readLevelData(const std::string& directory, LevelData& dest);
	static bool readPlayerData(const std::string& filename, LevelData& dest);
	static bool writeLevelData(const std::string& datFileName, LevelData& dest, const std::vector<Player*>* players);
	static void saveLevelData(const std::string& directory, LevelData& levelData, std::vector<Player*>* players);

    int savePendingUnsavedChunks(int maxCount);

	//
	// ChunkStorage methods
	//
	virtual LevelChunk* load(Level* level, int x, int z) override;
	void save(Level* level, LevelChunk* levelChunk) override;
	// @note, loadEntities and saveEntities dont use second parameter
	void loadEntities(Level* level, LevelChunk* levelChunk) override;
	void saveEntities(Level* level, LevelChunk* levelChunk) override;
	void saveGame(Level* level) override;
    void saveAll(Level* level, std::vector<LevelChunk*>& levelChunks) override;

	virtual void tick() override;
	virtual void flush() override {}
private:
	RegionFile* getRegionFile(int chunkX, int chunkZ, bool create);
	RegionFile* getLegacyRegionFile();
	static int floorDiv32(int value);
	static int localChunkCoord(int value);

	std::string levelId;
	std::string levelPath;
	LevelData* loadedLevelData;
	RegionFile* regionFile;
	RegionFile* entitiesFile;
	RegionFileMap regionFiles;

	Level* level;
	int tickCount;
	int loadedStorageVersion;
	UnsavedChunkList unsavedChunkList;
	int lastSavedEntitiesTick;
};

#endif /*NET_MINECRAFT_WORLD_LEVEL_STORAGE__ExternalFileLevelStorage_H__*/

#endif /*DEMO_MODE*/
