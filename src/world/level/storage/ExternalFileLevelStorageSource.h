#if !defined(DEMO_MODE) && !defined(APPLE_DEMO_PROMOTION)

#ifndef NET_MINECRAFT_WORLD_LEVEL_STORAGE__ExternalFileLevelStorageSource_H__
#define NET_MINECRAFT_WORLD_LEVEL_STORAGE__ExternalFileLevelStorageSource_H__

//package net.minecraft.world.level.storage;

#include "LevelStorageSource.h"
#include "MemoryLevelStorage.h"

class ProgressListener;

class ExternalFileLevelStorageSource: public LevelStorageSource
{
public:
	ExternalFileLevelStorageSource(const std::string& externalPath, const std::string& temporaryFilesPath);

    std::string getName() override;
	void getLevelList(LevelSummaryList& dest) override;

    LevelStorage* selectLevel(const std::string& levelId, bool createPlayerDir) override;
    LevelData* getDataTagFor(const std::string& levelId) override;

    bool isNewLevelIdAcceptable(const std::string& levelId) override;

	void clearAll() override {}
    void deleteLevel(const std::string& levelId) override;
    void renameLevel(const std::string& levelId, const std::string& newLevelName) override;

    bool isConvertible(const std::string& levelId) override { return false; }
    bool requiresConversion(const std::string& levelId) override { return false; }
    bool convertLevel(const std::string& levelId, ProgressListener* progress) override { return false; }
private:
	void addLevelSummaryIfExists(LevelSummaryList& dest, const char* dirName);
	bool hasTempDirectory() { return _hasTempDirectory; }
    std::string getFullPath(const std::string& levelId);

	std::string basePath;
	std::string tmpBasePath;
	bool _hasTempDirectory;
};

#endif /*NET_MINECRAFT_WORLD_LEVEL_STORAGE__ExternalFileLevelStorageSource_H__*/

#endif /*DEMO_MODE*/
