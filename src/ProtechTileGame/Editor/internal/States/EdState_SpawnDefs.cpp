#include "../../GameLocal.h"
#pragma hdrstop
#include "../EditorLocal.h"

/////////////////////////////////////////////////////////////////////
//
// EdState_SpawnDefs::MySpawnDefLibraryItem
//
/////////////////////////////////////////////////////////////////////

EdState_SpawnDefs::MySpawnDefLibraryItem::MySpawnDefLibraryItem(const SpawnDef &spawnDef)
: m_spawnDef(spawnDef)
{
	// nothing
}

const char *EdState_SpawnDefs::MySpawnDefLibraryItem::GetName() const
{
	return m_spawnDef.name.c_str();
}

int EdState_SpawnDefs::MySpawnDefLibraryItem::GetWidth() const
{
	return (int)(-m_spawnDef.editorBounds.mins.x + m_spawnDef.editorBounds.maxs.x);
}

int EdState_SpawnDefs::MySpawnDefLibraryItem::GetHeight() const
{
	return (int)(-m_spawnDef.editorBounds.mins.y + m_spawnDef.editorBounds.maxs.y);
}

void EdState_SpawnDefs::MySpawnDefLibraryItem::Draw(int screenX, int screenY) const
{
	TextureDrawAttribs tda;
	int w = Math_Min(GetWidth(), m_spawnDef.editorTexturePortion.w);
	int h = Math_Min(GetHeight(), m_spawnDef.editorTexturePortion.h);
	tda.EnablePortion(m_spawnDef.editorTexturePortion.s, m_spawnDef.editorTexturePortion.t, w, h);
	g_renderSystem->DrawTexture2D(m_spawnDef.editorTexture->CardHandle(), screenX, screenY, tda);
}


/////////////////////////////////////////////////////////////////////
//
// EdState_SpawnDefs
//
/////////////////////////////////////////////////////////////////////


EdState_SpawnDefs::EdState_SpawnDefs()
{
}

EdState_SpawnDefs::~EdState_SpawnDefs()
{
	pt_for_each(m_spawnDefLibraryItems, pt_delete_and_nullify<MySpawnDefLibraryItem>);
}

void EdState_SpawnDefs::Enter(const EdContext &ctx)
{
	// create the SpawnDef library on first call
	if(m_spawnDefLibraryItems.empty())
	{
		for(int i = 0; i < ctx.gameContext->spawnDefManager->NumSpawnDefs(); i++)
		{
			const SpawnDef &spawnDef = ctx.gameContext->spawnDefManager->GetSpawnDef(i);
			MySpawnDefLibraryItem *libraryItem = new MySpawnDefLibraryItem(spawnDef);
			m_spawnDefLibraryManager.RegisterItem(libraryItem);
			m_spawnDefLibraryItems.push_back(libraryItem);
		}
	}
}

void EdState_SpawnDefs::Update(const EdContext &ctx)
{
	m_spawnDefLibraryManager.UpdateScroll(ctx);

	if(ctx.currentInput->MouseButtonToggledDown(MOUSEBUTTON_LEFT))
	{
		int touchedItemIndex = m_spawnDefLibraryManager.ItemIndexAtMouseCursor(ctx);
		if(touchedItemIndex != -1)
		{
			ctx.stateBB->spawnDef = ctx.stateBB->lastSpawnDef = &m_spawnDefLibraryItems[touchedItemIndex]->GetSpawnDef();
		}
	}

	if(ctx.currentInput->MouseButtonToggledDown(MOUSEBUTTON_RIGHT))
	{
		ctx.stateBB->spawnDef = (ctx.stateBB->spawnDef == NULL) ? ctx.stateBB->lastSpawnDef : NULL;
	}

	updateGameRenderWorld(ctx);
}

void EdState_SpawnDefs::updateGameRenderWorld(const EdContext &ctx) const
{
	// name of the touched SpawnDef
	int touchedItemIndex = m_spawnDefLibraryManager.ItemIndexAtMouseCursor(ctx);
	if(touchedItemIndex != -1)
	{
		//ctx.font->DrawString2D(m_spawnDefLibraryItems[touchedItemIndex]->GetSpawnDef().name.c_str(), ctx.mousePosOnScreen[0] + 10, ctx.mousePosOnScreen[1] + 20);
		ctx.gameRenderWorld->AddDebugText(ctx.mousePosOnScreen[0] + 10, ctx.mousePosOnScreen[1] + 20, false, false, m_spawnDefLibraryItems[touchedItemIndex]->GetSpawnDef().name.c_str(), *ctx.font);
	}

	// image of the selected SpawnDef
	if(ctx.stateBB->spawnDef != NULL)
	{
		int screenX = ctx.mousePosOnScreen[0] - (int)(ctx.stateBB->spawnDef->editorBounds.maxs.x - ctx.stateBB->spawnDef->editorBounds.mins.x) / 2;
		int screenY = ctx.mousePosOnScreen[1] - (int)(ctx.stateBB->spawnDef->editorBounds.maxs.y - ctx.stateBB->spawnDef->editorBounds.mins.y) / 2;
		TextureDrawAttribs tda;
		tda.EnablePortion(ctx.stateBB->spawnDef->editorTexturePortion);
		ctx.gameRenderWorld->AddDebugSprite(screenX, screenY, ctx.stateBB->spawnDef->editorTexture, tda);
	}
	//EdStateHelper::AddMouseCursorToGameRenderWorld(ctx);
}

void EdState_SpawnDefs::Draw(const EdContext &ctx)
{
	m_spawnDefLibraryManager.Draw(ctx);

	//// name of the touched SpawnDef
	//int touchedItemIndex = m_spawnDefLibraryManager.ItemIndexAtMouseCursor(ctx);
	//if(touchedItemIndex != -1)
	//{
	//	ctx.font->DrawString2D(m_spawnDefLibraryItems[touchedItemIndex]->GetSpawnDef().name.c_str(), ctx.mousePosOnScreen[0] + 10, ctx.mousePosOnScreen[1] + 20);
	//}

	// image of the selected SpawnDef
	if(ctx.stateBB->spawnDef != NULL)
	{
		int screenX = ctx.mousePosOnScreen[0] - ctx.stateBB->spawnDef->editorTexturePortion.w / 2;
		int screenY = ctx.mousePosOnScreen[1] - ctx.stateBB->spawnDef->editorTexturePortion.h / 2;
		TextureDrawAttribs tda;
		tda.EnablePortion(ctx.stateBB->spawnDef->editorTexturePortion);
		g_renderSystem->DrawTexture2D(ctx.stateBB->spawnDef->editorTexture->CardHandle(), screenX, screenY, tda);
	}
	EdStateHelper::DrawMouseCursor(ctx);
}

void EdState_SpawnDefs::Exit(const EdContext &ctx)
{
}
