#include "../GameLocal.h"
#pragma hdrstop


SpawnSpotManager::~SpawnSpotManager()
{
	while(!m_spawnSpots.IsEmpty())
	{
		delete m_spawnSpots.Next(NULL);
	}
}

void SpawnSpotManager::LoadSpawnSpotsFromFile(const char *fileName, const SpawnDefManager &spawnDefManager)
{
	// remove previously existing SpawnSpots
	while(!m_spawnSpots.IsEmpty())
	{
		delete m_spawnSpots.Next(NULL);
	}

	PTString fileContent;
	FS_GetFileContentAsText(fileName, fileContent);
	Tokenizer tokenizer(fileContent.c_str());

	//Timer timer(true);

	PTString token;
	while(tokenizer.Next(token, false))
	{
		// expect "spawnDef"
		if(token != "spawnDef")
			throw PTException(VABuf("line #%i: expected 'spawnDef', got '%s'", tokenizer.CurrentLineNum(), token.c_str()));

		PTString spawnDefName;
		int posX, posY;
		int lookDir;

		// name of the spawnDef
		spawnDefName = tokenizer.Next();

		// posX
		tokenizer.NextExpect("posX");
		token = tokenizer.Next();
		posX = atoi(token.c_str());

		// posY
		tokenizer.NextExpect("posY");
		token = tokenizer.Next();
		posY = atoi(token.c_str());

		// lookDir
		tokenizer.NextExpect("lookDir");
		token = tokenizer.Next();
		lookDir = atoi(token.c_str());
		if(lookDir != 1 && lookDir != -1)
			throw PTException(VABuf("line #%i: bad lookDir: %i (expected 1 or -1)", tokenizer.CurrentLineNum(), lookDir));

		if(const SpawnDef *spawnDef = spawnDefManager.FindSpawnDef(spawnDefName.c_str()))
		{
			m_spawnSpots.AppendNode(*new SpawnSpot(spawnDef, posX, posY, lookDir));
		}
		else
		{
			LOG_Warning("unknown spawnDef: '%s'\n", spawnDefName.c_str());
		}
	}

	//LOG_Printf(COLOR_STR_CYAN "SpawnSpotManager::LoadSpawnSpotsFromFile: %f milliseconds\n", timer.Milliseconds());
}

void SpawnSpotManager::SaveSpawnSpotsToFile(const char *fileName)
{
	OutputFile f(fileName, true);
	for(const SpawnSpot *ss = m_spawnSpots.Next(NULL); ss != NULL; ss = m_spawnSpots.Next(ss))
	{
		f.PutStr(VABuf("spawnDef \"%s\"\n", ss->spawnDef->name.c_str()));
		f.PutStr(VABuf("posX %i\n", ss->posX));
		f.PutStr(VABuf("posY %i\n", ss->posY));
		f.PutStr(VABuf("lookDir %i\n", ss->lookDir));
		f.PutStr("\n");
	}
}

void SpawnSpotManager::AddSpawnSpot(const SpawnDef *spawnDef, int posX, int posY)
{
	m_spawnSpots.AppendNode(*new SpawnSpot(spawnDef, posX, posY, 1));
}

void SpawnSpotManager::DeleteSpawnSpot(const SpawnSpot *toDelete)
{
	pt_assert(m_spawnSpots.ContainsNode(*toDelete));
	delete toDelete;
}

SpawnSpot *SpawnSpotManager::GetFirstTouchedSpawnSpot(int touchPosX, int touchPosY)
{
	Vec2 touchPos(touchPosX, touchPosY);
	for(SpawnSpot *ss = m_spawnSpots.Next(NULL); ss != NULL; ss = m_spawnSpots.Next(ss))
	{
		if(ss->spawnDef->editorBounds.Translate(Vec2(ss->posX, ss->posY)).ContainsPoint(touchPos, true))
			return ss;
	}
	return NULL;
}

void SpawnSpotManager::AddDebugSpritesToGameRenderWorld(GameRenderWorld2D &rw) const
{
	for(const SpawnSpot *ss = m_spawnSpots.Next(NULL); ss != NULL; ss = m_spawnSpots.Next(ss))
	{
		TextureDrawAttribs tda;
		tda.EnablePortion(ss->spawnDef->editorTexturePortion);
		tda.EnableCenter();
		if(ss->lookDir == -1)
			tda.EnableMirror();
		rw.AddDebugSprite(ss->posX, ss->posY, ss->spawnDef->editorTexture, tda);
	}
}

void SpawnSpotManager::SpawnEntities(World &world, const Rect *optionalCurrentClipping, const Rect *optionalOldClipping)
{
	for(SpawnSpot *ss = m_spawnSpots.Next(NULL); ss != NULL; ss = m_spawnSpots.Next(ss))
	{
		// already spawned an entity which is still present?
		if(ss->spawnedEntity != NULL)
			continue;

		if(optionalCurrentClipping == NULL && optionalOldClipping == NULL)
		{
			ss->spawnedEntity = world.SpawnEntityBySpawnSpot(*ss, kSpawnType_OnLevelStart);
		}
		else
		{
			bool isVisibleNow = (optionalCurrentClipping == NULL) ? true : optionalCurrentClipping->ContainsPoint(Vec2(ss->posX, ss->posY), true);
			bool wasVisible = (optionalOldClipping == NULL) ?       false : optionalOldClipping->ContainsPoint(Vec2(ss->posX, ss->posY), true);

			if(isVisibleNow && !wasVisible)
			{
				ss->spawnedEntity = world.SpawnEntityBySpawnSpot(*ss, kSpawnType_BecameVisible);
			}
			else if(!isVisibleNow && wasVisible)
			{
				ss->spawnedEntity = world.SpawnEntityBySpawnSpot(*ss, kSpawnType_BecameInvisible);
			}
			else if(isVisibleNow && wasVisible)
			{
				ss->spawnedEntity = world.SpawnEntityBySpawnSpot(*ss, kSpawnType_WhileVisible);
			}
			// don't care for "is not and was not visible"
		}
	}
}
