#include "../GameLocal.h"
#pragma hdrstop


PrecachedFont *PrecachedFont::s_list;

PrecachedFont::PrecachedFont(const char *fontFileName)
: m_fontFileName(fontFileName),
m_next(s_list)
{
	s_list = this;
}

void PrecachedFont::LoadAllFonts()
{
	for(PrecachedFont *cur = s_list; cur != NULL; cur = cur->m_next)
	{
		cur->m_font.Load(cur->m_fontFileName);
	}
}
