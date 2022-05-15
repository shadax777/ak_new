#ifndef __PRECACHEDTEXTUREANIM_H
#define __PRECACHEDTEXTUREANIM_H


class PrecachedTextureAnim
{
private:
	const char *					m_animFileName;	// string literal must not go out of scope!
	const TextureAnim *				m_anim;
	PrecachedTextureAnim *			m_next;
	static PrecachedTextureAnim *	s_list;

	// not supported
									PrecachedTextureAnim(const PrecachedTextureAnim &);
	PrecachedTextureAnim &			operator=(const PrecachedTextureAnim &);

public:
	explicit						PrecachedTextureAnim(const char *animFileName);
	const TextureAnim &				Anim() const { pt_assert_msg(m_anim != NULL, "underlying animation is not yet loaded"); return *m_anim; }
	static void						CacheAllTextureAnims(TextureAnimCache &cacheForLookup);
};


#endif	// __PRECACHEDTEXTUREANIM_H
