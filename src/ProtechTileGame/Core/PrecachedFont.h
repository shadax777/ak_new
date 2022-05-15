#ifndef __PRECACHEDFONT_H
#define __PRECACHEDFONT_H


class PrecachedFont
{
private:
	const char *			m_fontFileName;	// string literal must not go out of scope!
	Font					m_font;
	PrecachedFont *			m_next;
	static PrecachedFont *	s_list;

	// not supported
							PrecachedFont(const PrecachedFont &);
	PrecachedFont &			operator=(const PrecachedFont &);

public:
	explicit				PrecachedFont(const char *fontFileName);
	const Font &			Fnt() const { pt_assert_msg(m_font.Tex() != NULL, "font not yet loaded"); return m_font; }
	static void				LoadAllFonts();
};


#endif	// __PRECACHEDFONT_H
