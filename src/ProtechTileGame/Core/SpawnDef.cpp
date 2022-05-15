#include "../GameLocal.h"
#pragma hdrstop


// deprecated
/*
void SpawnDefManager::LoadFromFile(const char *spawnDefsFileName, const GameContext &gameContext)
{
	PTString fileContent;
	FS_GetFileContentAsText(spawnDefsFileName, fileContent);
	Tokenizer tokenizer(fileContent.c_str());

	// TEST: build a spawn table as C code while parsing all SpawnDefs
	OutputFile codeFile("spawnTable.c", true);
	try
	{
		PTString token;
		while(tokenizer.Next(token, false))
		{
			// token already holds the name of the SpawnDef
			// => check for duplicate name
			if(FindSpawnDef(token.c_str()) != NULL)
				throw PTException(VABuf("line #%i: there is already a SpawnDef named '%s'", tokenizer.CurrentLineNum(), token.c_str()));

			SpawnDef newSpawnDef;
			newSpawnDef.name = token;

			// TEST: code file
			PTString code_editorImage;
			std::vector<std::pair<PTString, PTString> > code_spawnArgs;

			tokenizer.NextExpect("{");
			while(1)
			{
				token = tokenizer.Next();
				if(token == "}")
				{
					break;
				}
				else if(token == "entityClassName")
				{
					token = tokenizer.Next();
					newSpawnDef.entityClassName = token;
				}
				else if(token == "spawnArgs")
				{
					tokenizer.NextExpect("{");
					while(1)
					{
						PTString key = tokenizer.Next();
						if(key == "}")
							break;

						PTString value = tokenizer.Next();
						if(value == "}")
							throw PTException(VABuf("line #%i: unexpected '}' in key/value pair", tokenizer.CurrentLineNum()));

						newSpawnDef.spawnArgs.SetString(key.c_str(), value.c_str());

						// TEST: code file
						code_spawnArgs.push_back(std::pair<PTString, PTString>(key, value));
					}
				}
				else if(token == "editorBoundsWidth")
				{
					token = tokenizer.Next();
					float width = (float)atof(token.c_str());
					newSpawnDef.editorBounds.mins.x = -width / 2.0f;
					newSpawnDef.editorBounds.maxs.x =  width / 2.0f;
				}
				else if(token == "editorBoundsHeight")
				{
					token = tokenizer.Next();
					float height = (float)atof(token.c_str());
					newSpawnDef.editorBounds.mins.y = -height / 2.0f;
					newSpawnDef.editorBounds.maxs.y =  height / 2.0f;
				}
				else if(token == "editorImage")
				{
					token = tokenizer.Next();

					// TEST: code file
					code_editorImage = token;

					// check for .anim file (use the first frame of it then)
					if(Str_MatchFileExtension(token.c_str(), "anim"))
					{
						const TextureAnim &anim = (*gameContext.textureAnimCache)[token.c_str()];
						newSpawnDef.editorTexture = anim.frames[0].texture;
						newSpawnDef.editorTexturePortion = anim.frames[0].texturePortion;
					}
					else
					{
						// filename of an image obviously provided: check for texture-portion
						PTString::size_type separatorPos = token.find(':');
						if(separatorPos == PTString::npos)
						{
							// no specific texture portion desired
							// => simply enable the texture-portion to use the whole image (this makes it easier for other parts
							newSpawnDef.editorTexture = &(*gameContext.textureCache)[token.c_str()];
							newSpawnDef.editorTexturePortion.s = 0;
							newSpawnDef.editorTexturePortion.t = 0;
							newSpawnDef.editorTexturePortion.w = newSpawnDef.editorTexture->ImageWidth();
							newSpawnDef.editorTexturePortion.h = newSpawnDef.editorTexture->ImageHeight();
						}
						else
						{
							// explicit texture-portion specified after the image filename
							int s, t, w, h;
							PTString tmp = token.substr(separatorPos + 1);
							if(sscanf(tmp.c_str(), "%i %i %i %i", &s, &t, &w, &h) < 4)
								throw PTException(VABuf("%s #%i: bad texture-portion: '%s'", spawnDefsFileName, newSpawnDef.name.c_str(), tmp.c_str()));

							PTString filename = token.substr(0, separatorPos);
							newSpawnDef.editorTexture = &(*gameContext.textureCache)[filename.c_str()];
							newSpawnDef.editorTexturePortion.s = s;
							newSpawnDef.editorTexturePortion.t = t;
							newSpawnDef.editorTexturePortion.w = w;
							newSpawnDef.editorTexturePortion.h = h;
						}
					}
				}
				else
				{
					throw PTException(VABuf("line #%i: unknown token: '%s'", tokenizer.CurrentLineNum(), token.c_str()));
				}
			}
			m_spawnDefs.push_back(newSpawnDef);

			// TEST: code file
			codeFile.PutStr("\t{\n");
			codeFile.PutStr(VABuf("\t\t\"%s\",\t\"%s\",\t{ %i, %i },\t\"%s\"", 
				newSpawnDef.name.c_str(),
				newSpawnDef.entityClassName.c_str(),
				(int)(newSpawnDef.editorBounds.maxs.x - newSpawnDef.editorBounds.mins.x),
				(int)(newSpawnDef.editorBounds.maxs.y - newSpawnDef.editorBounds.mins.y),
				code_editorImage.c_str()));
			if(!code_spawnArgs.empty())
			{
				codeFile.PutStr(",\t{");
				for(size_t i = 0; i < code_spawnArgs.size(); i++)
				{
					codeFile.PutStr(VABuf(" { \"%s\", \"%s\" }", code_spawnArgs[i].first.c_str(), code_spawnArgs[i].second.c_str()));
					if(i < code_spawnArgs.size() - 1)
						codeFile.PutStr(",");
				}
				codeFile.PutStr(" }");
			}
			codeFile.PutStr("\n");
			codeFile.PutStr("\t},\n");

		}
	}
	catch(PTException &ex)
	{
		throw PTException(VABuf("error loading file %s: %s", spawnDefsFileName, ex.what()));
	}
}
*/
void SpawnDefManager::LoadFromInternalTable(const GameContext &gameContext)
{
	try
	{
		for(const MySpawnDefTable *tabEntry = s_allSpawnDefs; tabEntry->spawnDefName != NULL; tabEntry++)
		{
			// check for duplicate name
			if(FindSpawnDef(tabEntry->spawnDefName) != NULL)
				throw PTException(VABuf("there is already a SpawnDef named '%s'", tabEntry->spawnDefName));

			SpawnDef newSpawnDef;

			newSpawnDef.name = tabEntry->spawnDefName;

			newSpawnDef.entityClassName = tabEntry->entityClassName;

			newSpawnDef.editorBounds.mins.x = -tabEntry->editorBounds[0] / 2.0f;
			newSpawnDef.editorBounds.maxs.x =  tabEntry->editorBounds[0] / 2.0f;
			newSpawnDef.editorBounds.mins.y = -tabEntry->editorBounds[1] / 2.0f;
			newSpawnDef.editorBounds.maxs.y =  tabEntry->editorBounds[1] / 2.0f;

			for(int i = 0; i < NELEMSi(tabEntry->spawnArgs[i]) && tabEntry->spawnArgs[i][0] != NULL; i++)
			{
				newSpawnDef.spawnArgs.SetString(tabEntry->spawnArgs[i][0], tabEntry->spawnArgs[i][1]);
			}

			// editor image: check for .anim file (use the first frame of it then)
			if(Str_MatchFileExtension(tabEntry->editorImage, "anim"))
			{
				const TextureAnim &anim = (*gameContext.textureAnimCache)[tabEntry->editorImage];
				newSpawnDef.editorTexture = anim.frames[0].texture;
				newSpawnDef.editorTexturePortion = anim.frames[0].texturePortion;
			}
			else
			{
				// filename of an image obviously provided: check for texture-portion
				PTString editorImage = tabEntry->editorImage;
				PTString::size_type separatorPos = editorImage.find(':');
				if(separatorPos == PTString::npos)
				{
					// no specific texture portion desired
					// => simply enable the texture-portion to use the whole image (this makes it easier for other parts
					newSpawnDef.editorTexture = &(*gameContext.textureCache)[editorImage.c_str()];
					newSpawnDef.editorTexturePortion.s = 0;
					newSpawnDef.editorTexturePortion.t = 0;
					newSpawnDef.editorTexturePortion.w = newSpawnDef.editorTexture->ImageWidth();
					newSpawnDef.editorTexturePortion.h = newSpawnDef.editorTexture->ImageHeight();
				}
				else
				{
					// explicit texture-portion specified after the image filename
					int s, t, w, h;
					PTString tmp = editorImage.substr(separatorPos + 1);
					if(sscanf(tmp.c_str(), "%i %i %i %i", &s, &t, &w, &h) < 4)
						throw PTException(VABuf("%s #%i: bad texture-portion: '%s'", tabEntry->spawnDefName, newSpawnDef.name.c_str(), tmp.c_str()));

					PTString filename = editorImage.substr(0, separatorPos);
					newSpawnDef.editorTexture = &(*gameContext.textureCache)[filename.c_str()];
					newSpawnDef.editorTexturePortion.s = s;
					newSpawnDef.editorTexturePortion.t = t;
					newSpawnDef.editorTexturePortion.w = w;
					newSpawnDef.editorTexturePortion.h = h;
				}
			}

			m_spawnDefs.push_back(newSpawnDef);
		}
	}
	catch(PTException &ex)
	{
		throw PTException(VABuf("error loading the internal spawn-def table: %s", ex.what()));
	}
}

const SpawnDef &SpawnDefManager::GetSpawnDef(int index) const
{
	pt_assert(index >= 0 && index < (int)m_spawnDefs.size());
	return m_spawnDefs[index];
}

const SpawnDef *SpawnDefManager::FindSpawnDef(const char *name) const
{
	for(size_t i = 0; i < m_spawnDefs.size(); i++)
	{
		if(strcmp(m_spawnDefs[i].name.c_str(), name) == 0)
			return &m_spawnDefs[i];
	}
	return NULL;
}
