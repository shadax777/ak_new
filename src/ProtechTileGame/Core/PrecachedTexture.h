#ifndef __PRECACHEDTEXTURE_H
#define __PRECACHEDTEXTURE_H


class PrecachedTexture
{
private:
	const char *				m_textureFileName;	// string literal must not go out of scope!
	const Texture *				m_texture;
	PrecachedTexture *			m_next;
	static PrecachedTexture *	s_list;

	// not supported
								PrecachedTexture(const PrecachedTexture &);
	PrecachedTexture &			operator=(const PrecachedTexture &);

public:
	explicit					PrecachedTexture(const char *textureFileName);
	const Texture &				Tex() const { pt_assert(m_texture != NULL); return *m_texture; }
	static void					CacheAllTextures(TextureCache &texCacheForLookup);
};


#endif	// __PRECACHEDTEXTURE_H
