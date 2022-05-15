#include "../GameLocal.h"
#pragma hdrstop


LevelManager::LevelManager(const SpawnDefManager &spawnDefManager)
: m_spawnDefManager(spawnDefManager),
m_levelData(NULL)
{
	// nothing
}

LevelManager::~LevelManager()
{
	pt_delete_and_nullify(m_levelData);
}

PTString LevelManager::getLevelNameFromDirectoryPath(const char *directoryPath)
{
	// directoryPath = "c:/game/basedir/levels/ak_level01" (no matter if a trailing slash is present or not)
	// => levelName  = "ak_level01"

	const char *levelNameEnd = directoryPath + strlen(directoryPath);
	while(levelNameEnd > directoryPath && (*levelNameEnd == '\0' || *levelNameEnd == '/' || *levelNameEnd == '\\'))
		levelNameEnd--;

	const char *levelNameBegin = levelNameEnd;
	while(levelNameBegin > directoryPath && levelNameBegin[-1] != '/' && levelNameBegin[-1] != '\\')
		levelNameBegin--;

	return PTString(levelNameBegin, levelNameEnd + 1);
}

void LevelManager::LoadLevel(const char *directory)
{
	// a single level is organized as directory containing all parts of a level in separate files

	LevelData *newLevel = new LevelData;
	try
	{
		Sys_SkipOneFrame();

		// directory    = "levels/ak_level01"
		// => levelName = "ak_level01"
		PTString levelName = getLevelNameFromDirectoryPath(directory);
		newLevel->tileLayer.LoadFromFile(VABuf("%s/%s.tilelayer", directory, levelName.c_str()));
		newLevel->roomMatrix.LoadFromFile(VABuf("%s/%s.rooms", directory, levelName.c_str()));
		newLevel->spawnSpotManager.LoadSpawnSpotsFromFile(VABuf("%s/%s.spawnspots", directory, levelName.c_str()), m_spawnDefManager);
		m_lastLoadedDirectory = directory;
		delete m_levelData;
		m_levelData = newLevel;
		LOG_Printf("level %s loaded.\n", levelName.c_str());
	}
	catch(...)
	{
		delete newLevel;
		throw;
	}
	for(LevelLoadCallbackSelfNode *cur = m_loadCallbacks.Next(NULL); cur != NULL; cur = m_loadCallbacks.Next(cur))
	{
		cur->self.onNewLevelLoaded(m_levelData);
	}
}

void LevelManager::SaveLoadedLevel() const
{
	PTString levelName = getLevelNameFromDirectoryPath(m_lastLoadedDirectory.c_str());
	m_levelData->tileLayer.SaveToFile(VABuf("%s/%s.tilelayer", m_lastLoadedDirectory.c_str(), levelName.c_str()));
	m_levelData->roomMatrix.SaveToFile(VABuf("%s/%s.rooms", m_lastLoadedDirectory.c_str(), levelName.c_str()));
	m_levelData->spawnSpotManager.SaveSpawnSpotsToFile(VABuf("%s/%s.spawnspots", m_lastLoadedDirectory.c_str(), levelName.c_str()));
	LOG_Printf("level %s saved in directory %s.\n", levelName.c_str(), m_lastLoadedDirectory.c_str());
}

void LevelManager::RegisterLevelLoadCallback(LevelLoadCallbackBase &callback)
{
	pt_assert(!callback.m_node.IsLinked());
	m_loadCallbacks.AppendNode(callback.m_node);
}
