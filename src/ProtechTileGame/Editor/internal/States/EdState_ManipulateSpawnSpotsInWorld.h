#ifndef __EDSTATE_MANIPULATESPAWNSPOTSINWORLD_H
#define __EDSTATE_MANIPULATESPAWNSPOTSINWORLD_H


class EdState_ManipulateSpawnSpotsInWorld : public EdState
{
private:
	typedef bool					(EdState_ManipulateSpawnSpotsInWorld::*MyUserActionFn_t)(const EdContext &ctx);

	std::vector<MyUserActionFn_t>	m_userActions;
	std::vector<SpawnSpot *>		m_selection;
	int								m_oldMousePosInWorld[2];
	bool							m_draggingSelection;

	void							updateGameRenderWorld(const EdContext &ctx) const;

	// - user actions
	// - they get all executed in turn by Update()
	// - the bool return value is no longer used; it just signaled whether or not the intended action was actually performed
	bool							tryStartDraggingSelection(const EdContext &ctx);
	bool							tryDragSelection(const EdContext &ctx);
	bool							trySelectSingleSpawnSpot(const EdContext &ctx);
	bool							tryAddSpawnSpotToSelection(const EdContext &ctx);
	bool							tryRemoveSpawnSpotFromSelection(const EdContext &ctx);
	bool							tryRemoveAllSpawnSpotsFromSelection(const EdContext &ctx);
	bool							tryDeleteAllSelectedSpawnSpots(const EdContext &ctx);
	bool							tryDeleteTouchedSpawnSpot(const EdContext &ctx);
	bool							tryDragSelectedSpawnSpots(const EdContext &ctx);
	bool							tryCopySpawnDefFromWorldToClipboard(const EdContext &ctx);
	bool							tryCreateNewSpawnSpotInWorld(const EdContext &ctx);
	bool							tryClearClipboard(const EdContext &ctx);
	bool							tryToggleLookDirOfSelectedSpawnSpots(const EdContext &ctx);

public:
									EdState_ManipulateSpawnSpotsInWorld();

	// implement EdState
	virtual void					Enter(const EdContext &ctx);
	virtual void					Update(const EdContext &ctx);
	virtual void					Draw(const EdContext &ctx);
	virtual void					Exit(const EdContext &ctx);
};


#endif	// __EDSTATE_MANIPULATESPAWNSPOTSINWORLD_H
