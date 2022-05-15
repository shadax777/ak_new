#ifndef __EDSTATE_TILESET_H
#define __EDSTATE_TILESET_H


// - shows the TileSet of the currently being edited TileLayer
// - allows for selecting a range of Tiles and copies them to the EdTileClipboard of the current EdContext
class EdState_TileSet : public EdState
{
private:
	int				m_downScroll;					// amount of having scrolled down
	int				m_rightScroll;					// amount of having scrolled right
	int				m_selectionFirstGridX;			// to start selecting a range of Tiles
	int				m_selectionFirstGridY;			// ditto
	bool			m_selectingTileRange;

	void			getCurrentlyTouchedTileIndexes(const EdContext &ctx, int &outGridX, int &outGridY) const;

public:
					EdState_TileSet();

	// implement EdState
	virtual void	Enter(const EdContext &ctx);
	virtual void	Update(const EdContext &ctx);
	virtual void	Draw(const EdContext &ctx);
	virtual void	Exit(const EdContext &ctx);
};


#endif	// __EDSTATE_TILESET_H
