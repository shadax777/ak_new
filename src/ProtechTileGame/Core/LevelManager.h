#ifndef __LEVELMANAGER_H
#define __LEVELMANAGER_H


class LevelLoadCallbackBase;	// below
typedef LinkedListSelfNode<LevelLoadCallbackBase>	LevelLoadCallbackSelfNode;


/////////////////////////////////////////////////////////////////////
//
// LevelLoadCallbackBase
//
/////////////////////////////////////////////////////////////////////
class LevelLoadCallbackBase
{
	friend class LevelManager;

private:
	LevelLoadCallbackSelfNode	m_node;
	virtual void				onNewLevelLoaded(LevelData *newLevelData) = 0;

protected:
	LevelLoadCallbackBase() : m_node(*this) {}
};


/////////////////////////////////////////////////////////////////////
//
// LevelLoadCallback<> - uses a member-function as callback
//
/////////////////////////////////////////////////////////////////////
template <class T>
class LevelLoadCallback : public LevelLoadCallbackBase
{
private:
	T *				m_target;
	void			(T::*m_func)(LevelData *newLevelData);

	virtual void	onNewLevelLoaded(LevelData *newLevelData) { (m_target->*m_func)(newLevelData); }

public:
					LevelLoadCallback(T *target, void (T::*func)(LevelData *newLevelData)) : m_target(target), m_func(func) {}
};


/////////////////////////////////////////////////////////////////////
//
// LevelLoadCallbackStatic - uses a freestanding function as callback
//
/////////////////////////////////////////////////////////////////////
class LevelLoadCallbackStatic : public LevelLoadCallbackBase
{
private:
	void			(*m_func)(LevelData *newLevelData);

	virtual void	onNewLevelLoaded(LevelData *newLevelData) { (*m_func)(newLevelData); }

public:
	explicit		LevelLoadCallbackStatic(void (*func)(LevelData *newLevelData)) : m_func(func) {}
};


/////////////////////////////////////////////////////////////////////
//
// LevelManager
//
// - loads and saves level data and keeps track of the last loaded level
// - a single level is organized as directory containing all major parts of a level in separate files
//
/////////////////////////////////////////////////////////////////////
class LevelManager
{
private:
	const SpawnDefManager &					m_spawnDefManager;
	LinkedList<LevelLoadCallbackSelfNode>	m_loadCallbacks;
	LevelData *								m_levelData;
	PTString								m_lastLoadedDirectory;	// is != "" if a level is currently loaded; used for saving the last loaded level

	static PTString							getLevelNameFromDirectoryPath(const char *directoryPath);

public:
	explicit								LevelManager(const SpawnDefManager &spawnDefManager);
											~LevelManager();
	void									LoadLevel(const char *directory);
	void									SaveLoadedLevel() const;
	void									RegisterLevelLoadCallback(LevelLoadCallbackBase &callback);
};


#endif	// __LEVELMANAGER_H
