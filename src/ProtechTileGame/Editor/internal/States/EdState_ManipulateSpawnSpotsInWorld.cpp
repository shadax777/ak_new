#include "../../GameLocal.h"
#pragma hdrstop
#include "../EditorLocal.h"


EdState_ManipulateSpawnSpotsInWorld::EdState_ManipulateSpawnSpotsInWorld()
{
	m_oldMousePosInWorld[0] = m_oldMousePosInWorld[1] = 0;
	m_draggingSelection = false;

	m_userActions.push_back(&EdState_ManipulateSpawnSpotsInWorld::tryStartDraggingSelection);
	m_userActions.push_back(&EdState_ManipulateSpawnSpotsInWorld::tryDragSelection);
	m_userActions.push_back(&EdState_ManipulateSpawnSpotsInWorld::trySelectSingleSpawnSpot);
	m_userActions.push_back(&EdState_ManipulateSpawnSpotsInWorld::tryAddSpawnSpotToSelection);
	m_userActions.push_back(&EdState_ManipulateSpawnSpotsInWorld::tryRemoveAllSpawnSpotsFromSelection);
	m_userActions.push_back(&EdState_ManipulateSpawnSpotsInWorld::tryRemoveSpawnSpotFromSelection);
	m_userActions.push_back(&EdState_ManipulateSpawnSpotsInWorld::tryDeleteAllSelectedSpawnSpots);
	m_userActions.push_back(&EdState_ManipulateSpawnSpotsInWorld::tryDeleteTouchedSpawnSpot);
	m_userActions.push_back(&EdState_ManipulateSpawnSpotsInWorld::tryDragSelectedSpawnSpots);
	m_userActions.push_back(&EdState_ManipulateSpawnSpotsInWorld::tryCopySpawnDefFromWorldToClipboard);
	m_userActions.push_back(&EdState_ManipulateSpawnSpotsInWorld::tryCreateNewSpawnSpotInWorld);
	m_userActions.push_back(&EdState_ManipulateSpawnSpotsInWorld::tryClearClipboard);
	m_userActions.push_back(&EdState_ManipulateSpawnSpotsInWorld::tryToggleLookDirOfSelectedSpawnSpots);
}

void EdState_ManipulateSpawnSpotsInWorld::Enter(const EdContext &ctx)
{
	m_selection.clear();	// prevent dangling pointers when loading a new level; FIXME: this is stupid when coming back from EdState_SpawnDefs
	MEM_CopyArray(m_oldMousePosInWorld, ctx.mousePosInWorld);
	m_draggingSelection = false;
}

void EdState_ManipulateSpawnSpotsInWorld::Update(const EdContext &ctx)
{
	EdStateHelper::MoveCameraByInput(ctx);
	if(!ctx.currentInput->currentMouseButtons[MOUSEBUTTON_LEFT])
		m_draggingSelection = false;
	for(size_t i = 0; i < m_userActions.size(); i++)
	{
		MyUserActionFn_t fn = m_userActions[i];
		(this->*fn)(ctx);
	}
	MEM_CopyArray(m_oldMousePosInWorld, ctx.mousePosInWorld);

	updateGameRenderWorld(ctx);
}

void EdState_ManipulateSpawnSpotsInWorld::updateGameRenderWorld(const EdContext &ctx) const
{
	EdStateHelper::AddRoomBackgroundsToGameRenderWorld(ctx);
	EdStateHelper::AddTileLayerToEditToGameRenderWorld(ctx);
	EdStateHelper::AddTileLayerGridLinesToGameRenderWorld(ctx);
	EdStateHelper::AddRoomRectangleToGameRenderWorld(ctx);
	EdStateHelper::AddSpawnSpotsToGameRenderWorld(ctx);

	// draw red rectangles around the selected SpawnSpots
	for(size_t i = 0; i < m_selection.size(); i++)
	{
		EdStateHelper::AddSpawnSpotRectangleToGameRenderWorld(ctx, *m_selection[i]);
	}

	// draw a red rectangle around the touched SpawnSpot and also the name + position of the SpawnDef at the mouse cursor
	if(SpawnSpot *touchedSpawnSpot = ctx.levelToEdit->spawnSpotManager.GetFirstTouchedSpawnSpot(ctx.mousePosInWorld[0], ctx.mousePosInWorld[1]))
	{
		EdStateHelper::AddSpawnSpotRectangleToGameRenderWorld(ctx, *touchedSpawnSpot);
		VABuf text("%s (%i, %i)", touchedSpawnSpot->spawnDef->name.c_str(), touchedSpawnSpot->posX, touchedSpawnSpot->posY);
		ctx.gameRenderWorld->AddDebugText(ctx.mousePosInWorld[0] + 10, ctx.mousePosInWorld[1] + 20, false, false, text, *ctx.font);
	}

	// draw SpawnDef in clipboard at the mouse pos
	if(ctx.stateBB->spawnDef != NULL)
	{
		int x = ctx.mousePosInWorld[0] - ctx.stateBB->spawnDef->editorTexturePortion.w / 2;
		int y = ctx.mousePosInWorld[1] - ctx.stateBB->spawnDef->editorTexturePortion.h / 2;
		TextureDrawAttribs tda;
		tda.EnablePortion(ctx.stateBB->spawnDef->editorTexturePortion);
		ctx.gameRenderWorld->AddDebugSprite(x, y, ctx.stateBB->spawnDef->editorTexture, tda);
	}
}

void EdState_ManipulateSpawnSpotsInWorld::Draw(const EdContext &ctx)
{
	ctx.gameRenderWorld->RenderScene(*ctx.gameRenderView);

	// caption + statistics
	EdStateHelper::DrawCaption(ctx, "Spawn-Spot Editor");
	EdStateHelper::DrawText(ctx, VABuf("total spawn spots:    %i", ctx.levelToEdit->spawnSpotManager.NumSpawnSpots()), 1);
	EdStateHelper::DrawText(ctx, VABuf("selected spawn spots: %i", (int)m_selection.size()), 2);

	EdStateHelper::DrawMouseCursor(ctx);

	// can copy to clipboard? (must touch a SpawnSpot and hold down SHIFT)
	if(ctx.levelToEdit->spawnSpotManager.GetFirstTouchedSpawnSpot(ctx.mousePosInWorld[0], ctx.mousePosInWorld[1]) != NULL && ctx.currentInput->currentKeyMatrix[PT_KEY_SHIFT])
	{
		EdStateHelper::DrawMouseCursorCopySymbol(ctx);
	}
}

void EdState_ManipulateSpawnSpotsInWorld::Exit(const EdContext &ctx)
{
}

bool EdState_ManipulateSpawnSpotsInWorld::tryStartDraggingSelection(const EdContext &ctx)
{
	if(!ctx.currentInput->MouseButtonToggledDown(MOUSEBUTTON_LEFT))
		return false;

	if(EdStateHelper::GetFirstTouchedSpawnSpot(ctx) == NULL)
		return false;

	m_draggingSelection = true;
	return true;
}

bool EdState_ManipulateSpawnSpotsInWorld::tryDragSelection(const EdContext &ctx)
{
	if(!m_draggingSelection)
		return false;

	if(!ctx.currentInput->currentMouseButtons[MOUSEBUTTON_LEFT])
	{
		// stop dragging
		m_draggingSelection = false;
		return false;
	}

	return true;
}

bool EdState_ManipulateSpawnSpotsInWorld::trySelectSingleSpawnSpot(const EdContext &ctx)
{
	if(ctx.stateBB->spawnDef != NULL)
		return false;

	if(!ctx.currentInput->MouseButtonToggledDown(MOUSEBUTTON_LEFT))
		return false;

	if(ctx.currentInput->currentKeyMatrix[PT_KEY_CONTROL])
		return false;

	if(ctx.currentInput->currentKeyMatrix[PT_KEY_SHIFT])
		return false;

	SpawnSpot *touchedSpawnSpot = EdStateHelper::GetFirstTouchedSpawnSpot(ctx);
	if(touchedSpawnSpot == NULL)
		return false;

	if(pt_exists(m_selection, touchedSpawnSpot))
		return false;

	m_selection.clear();
	m_selection.push_back(touchedSpawnSpot);
	return true;
}

bool EdState_ManipulateSpawnSpotsInWorld::tryAddSpawnSpotToSelection(const EdContext &ctx)
{
	if(ctx.stateBB->spawnDef != NULL)
		return false;

	if(!ctx.currentInput->MouseButtonToggledDown(MOUSEBUTTON_LEFT))
		return false;

	if(!ctx.currentInput->currentKeyMatrix[PT_KEY_CONTROL])
		return false;

	SpawnSpot *touchedSpawnSpot = EdStateHelper::GetFirstTouchedSpawnSpot(ctx);
	if(touchedSpawnSpot == NULL)
		return false;

	if(pt_exists(m_selection, touchedSpawnSpot))
		return false;

	m_selection.push_back(touchedSpawnSpot);
	return true;
}

bool EdState_ManipulateSpawnSpotsInWorld::tryRemoveSpawnSpotFromSelection(const EdContext &ctx)
{
	if(!m_selection.empty())
		return false;

	if(!ctx.currentInput->MouseButtonToggledDown(MOUSEBUTTON_LEFT))
		return false;

	if(!ctx.currentInput->currentKeyMatrix[PT_KEY_CONTROL])
		return false;

	SpawnSpot *touchedSpawnSpot = EdStateHelper::GetFirstTouchedSpawnSpot(ctx);
	if(touchedSpawnSpot == NULL)
		return false;

	if(!pt_exists(m_selection, touchedSpawnSpot))
		return false;

	pt_erase_all(m_selection, touchedSpawnSpot);
	return true;
}

bool EdState_ManipulateSpawnSpotsInWorld::tryRemoveAllSpawnSpotsFromSelection(const EdContext &ctx)
{
	if(!ctx.currentInput->MouseButtonToggledDown(MOUSEBUTTON_LEFT))
		return false;

	if(EdStateHelper::GetFirstTouchedSpawnSpot(ctx) != NULL)
		return false;

	m_selection.clear();
	return true;
}

bool EdState_ManipulateSpawnSpotsInWorld::tryDeleteAllSelectedSpawnSpots(const EdContext &ctx)
{
	if(m_selection.empty())
		return false;

	if(!ctx.currentInput->KeyToggledDown(PT_KEY_DEL))
		return false;

	for(size_t i = 0; i < m_selection.size(); i++)
	{
		ctx.levelToEdit->spawnSpotManager.DeleteSpawnSpot(m_selection[i]);
	}
	m_selection.clear();
	return true;
}

bool EdState_ManipulateSpawnSpotsInWorld::tryDeleteTouchedSpawnSpot(const EdContext &ctx)
{
	if(!m_selection.empty())
		return false;

	if(!ctx.currentInput->currentKeyMatrix[PT_KEY_DEL])
		return false;

	SpawnSpot *touchedSpawnSpot = EdStateHelper::GetFirstTouchedSpawnSpot(ctx);
	if(touchedSpawnSpot == NULL)
		return false;

	ctx.levelToEdit->spawnSpotManager.DeleteSpawnSpot(touchedSpawnSpot);
	pt_erase_all(m_selection, touchedSpawnSpot);	// just in case
	return true;
}

bool EdState_ManipulateSpawnSpotsInWorld::tryDragSelectedSpawnSpots(const EdContext &ctx)
{
	if(m_selection.empty())
		return false;

	if(!ctx.currentInput->currentMouseButtons[MOUSEBUTTON_LEFT])
		return false;

	//LOG_Printf("drag dx=%f dy=%f\n", ctx.currentInput->mouseMoveDX, ctx.currentInput->mouseMoveDY);
	int dxInWorld = ctx.mousePosInWorld[0] - m_oldMousePosInWorld[0];
	int dyInWorld = ctx.mousePosInWorld[1] - m_oldMousePosInWorld[1];
	//LOG_Printf("drag: dx=%i dy=%i\n", dxInWorld, dyInWorld);
	for(size_t i = 0; i < m_selection.size(); i++)
	{
		m_selection[i]->posX += dxInWorld;
		m_selection[i]->posY += dyInWorld;
	}
	return true;
}

bool EdState_ManipulateSpawnSpotsInWorld::tryCopySpawnDefFromWorldToClipboard(const EdContext &ctx)
{
	SpawnSpot *touchedSpawnSpot = EdStateHelper::GetFirstTouchedSpawnSpot(ctx);
	if(touchedSpawnSpot == NULL)
		return false;

	if(!ctx.currentInput->currentKeyMatrix[PT_KEY_SHIFT])
		return false;

	if(!ctx.currentInput->MouseButtonToggledDown(MOUSEBUTTON_LEFT))
		return false;

	ctx.stateBB->spawnDef = ctx.stateBB->lastSpawnDef = touchedSpawnSpot->spawnDef;
	return true;
}

bool EdState_ManipulateSpawnSpotsInWorld::tryCreateNewSpawnSpotInWorld(const EdContext &ctx)
{
	if(ctx.stateBB->spawnDef == NULL)
		return false;

	if(!m_selection.empty())
		return false;

	if(!ctx.currentInput->MouseButtonToggledDown(MOUSEBUTTON_LEFT))
		return false;

	if(ctx.currentInput->currentKeyMatrix[PT_KEY_SHIFT])
		return false;

	ctx.levelToEdit->spawnSpotManager.AddSpawnSpot(ctx.stateBB->spawnDef, ctx.mousePosInWorld[0], ctx.mousePosInWorld[1]);
	return true;
}

bool EdState_ManipulateSpawnSpotsInWorld::tryClearClipboard(const EdContext &ctx)
{
	if(!ctx.currentInput->MouseButtonToggledDown(MOUSEBUTTON_RIGHT))
		return false;

	ctx.stateBB->spawnDef = NULL;
	return true;
}

bool EdState_ManipulateSpawnSpotsInWorld::tryToggleLookDirOfSelectedSpawnSpots(const EdContext &ctx)
{
	if(!ctx.currentInput->KeyToggledDown((PTKeyNum)'d'))
		return false;

	for(size_t i = 0; i < m_selection.size(); i++)
	{
		m_selection[i]->lookDir *= -1;
	}
	return true;
}
