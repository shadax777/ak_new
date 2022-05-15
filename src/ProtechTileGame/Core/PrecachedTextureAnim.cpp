#include "../GameLocal.h"
#pragma hdrstop


PrecachedTextureAnim *PrecachedTextureAnim::s_list;

PrecachedTextureAnim::PrecachedTextureAnim(const char *animFileName)
: m_animFileName(animFileName),
m_anim(NULL),
m_next(s_list)
{
	s_list = this;
}

void PrecachedTextureAnim::CacheAllTextureAnims(TextureAnimCache &cacheForLookup)
{
	for(PrecachedTextureAnim *cur = s_list; cur != NULL; cur = cur->m_next)
	{
		cur->m_anim = &cacheForLookup[cur->m_animFileName];
	}
}
