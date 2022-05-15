#ifndef __EDSTATE_MANIPULATETILESINWORLD_H
#define __EDSTATE_MANIPULATETILESINWORLD_H


class EdState_ManipulateTilesInWorld : public EdState
{
private:
	int				m_selectionFirstGridX;			// to start selecting a range of Tiles
	int				m_selectionFirstGridY;			// ditto
	bool			m_selectingTileRange;

	void			updateGameRenderWorld(const EdContext &ctx) const;

	static void		getCurrentlyTouchedTileIndexes(const EdContext &ctx, int &outGridX, int &outGridY);

public:
					EdState_ManipulateTilesInWorld();

	// implement EdState
	virtual void	Enter(const EdContext &ctx);
	virtual void	Update(const EdContext &ctx);
	virtual void	Draw(const EdContext &ctx);
	virtual void	Exit(const EdContext &ctx);
};


#endif	// __EDSTATE_MANIPULATETILESINWORLD_H
