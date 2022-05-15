#ifndef __EDLIBRARYMANAGER_H
#define __EDLIBRARYMANAGER_H

// - helper class that can deal with items on the screen in a way that they use the available on-screen space optimally by
//   positioning them from left to right, top to bottom
// - provides some basic logic for picking one of the registered items via the mouse and provides its index among all registered items
class EdLibraryManager
{
private:
	std::vector<const EdLibraryItem *>	m_items;
	int									m_maxItemWidth;
	int									m_maxItemHeight;
	int									m_selectedItemIndex;
	int									m_lastSelectedItemIndex;
	int									m_downScroll;	// amount of having scrolled down

public:
										EdLibraryManager();

	// - registers an item for being drawn and selected
	// - the registered item will only be held as pointer, so the caller must make sure to
	//   not let the item go out of scope
	void								RegisterItem(const EdLibraryItem *item);

	// - draws all registered items, a red rectangle around the touched item and the name of the touched item
	// - also draws an red rectangle around the the selected item
	void								Draw(const EdContext &ctx) const;

	// attempts to select an item or unselects the currently selected one
	//ItemSelectResult					OnMouseEvent(const MouseEvent &ev);

	// returns -1 if the mouse cursor does not point at an item
	int									ItemIndexAtMouseCursor(const EdContext &ctx) const;

	// attempts to select the item that is currently touched by the mouse
	void								TrySelectTouchedItem(const EdContext &ctx);

	// NULL if no item is currently selected
	const EdLibraryItem *				SelectedItem() const { return m_selectedItemIndex == -1 ? NULL : m_items[m_selectedItemIndex]; }

	// -1 if no item is currently selected
	int									SelectedItemIndex() const { return m_selectedItemIndex ;}

	// -1 if no item was ever selected
	int									LastSelectedItemIndex() const { return m_lastSelectedItemIndex; }

	// unselects the selected item (if one is selected)
	void								UnselectItem() { m_selectedItemIndex = -1; }

	void								SelectItemByIndex(int itemIndexToSelect);

	// scroll up/down
	void								UpdateScroll(const EdContext &ctx);
};


#endif	// __EDLIBRARYMANAGER_H
