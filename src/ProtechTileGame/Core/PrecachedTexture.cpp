#include "../GameLocal.h"
#pragma hdrstop


PrecachedTexture *PrecachedTexture::s_list;

PrecachedTexture::PrecachedTexture(const char *textureFileName)
: m_textureFileName(textureFileName),
m_texture(NULL),
m_next(s_list)
{
	s_list = this;
}

void PrecachedTexture::CacheAllTextures(TextureCache &texCacheForLookup)
{
	for(PrecachedTexture *cur = s_list; cur != NULL; cur = cur->m_next)
	{
		cur->m_texture = &texCacheForLookup[cur->m_textureFileName];
	}
}
