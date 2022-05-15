#ifndef __EDSTATE_SPAWNDEFS_H
#define __EDSTATE_SPAWNDEFS_H


// - provides a menu of all available SpawnDefs for picking one to put into the world
class EdState_SpawnDefs : public EdState
{
private:
	class MySpawnDefLibraryItem : public EdLibraryItem
	{
	private:
		const SpawnDef &					m_spawnDef;

	public:
		explicit							MySpawnDefLibraryItem(const SpawnDef &spawnDef);

		// implement EdLibraryItem
		virtual const char *				GetName() const;
		virtual int							GetWidth() const;
		virtual int							GetHeight() const;
		virtual void						Draw(int screenX, int screenY) const;

		const SpawnDef &					GetSpawnDef() const { return m_spawnDef; }
	};

	//---------------------------------------------------------------

	std::vector<MySpawnDefLibraryItem *>	m_spawnDefLibraryItems;
	EdLibraryManager						m_spawnDefLibraryManager;

	void									updateGameRenderWorld(const EdContext &ctx) const;

public:
											EdState_SpawnDefs();
											~EdState_SpawnDefs();

	// implement EdState
	virtual void							Enter(const EdContext &ctx);
	virtual void							Update(const EdContext &ctx);
	virtual void							Draw(const EdContext &ctx);
	virtual void							Exit(const EdContext &ctx);
};


#endif	// __EDSTATE_SPAWNDEFS_H
