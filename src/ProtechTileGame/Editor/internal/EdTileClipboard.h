#ifndef __EDTILECLIPBOARD_H
#define __EDTILECLIPBOARD_H


// an EdTileClipboard is always bound to a specific TileLayer (and its underlying TileSet) to make sure that no tiles from other sources can be copied & pasted
class EdTileClipboard
{
private:
	TileLayer *						m_boundTileLayer;
	int								m_clipboardWidthInTiles;
	int								m_clipboardHeightInTiles;
	std::vector<const TileDef *>	m_clipboardData;	// 2-dimensional array

	void							resizeClipboard(int &inOutFirstGridX, int &inOutFirstGridY, int lastGridX, int lastGridY, int maxWidthInTiles, int maxHeightInTiles);

public:
									EdTileClipboard();
	void							BindToTileLayer(TileLayer &tl);
	void							CopyFromTileLayer(int firstGridX, int firstGridY, int lastGridX, int lastGridY);
	void							CopyFromTileSet(int firstGridX, int firstGridY, int lastGridX, int lastGridY);
	void							PasteToTileLayer(int startGridX, int startGridY) const;
	void							DrawCopiedTiles(int screenDrawOfsX, int screenDrawOfsY) const;
	void							MakeEmpty();
	bool							IsEmpty() const;
	void							TryScrollToNeightboredTileDef(int mouseWheelDelta);
};


#endif	// __EDTILECLIPBOARD_H
