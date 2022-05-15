#ifndef __EDLIBRARYITEM_H
#define __EDLIBRARYITEM_H


// a tile or entity template to pick from all available ones in order to paste it into the world
class EdLibraryItem
{
protected:
							~EdLibraryItem() {}	// no virtual destruction intended

public:
	virtual const char *	GetName() const = 0;
	virtual int				GetWidth() const = 0;
	virtual int				GetHeight() const = 0;
	virtual void			Draw(int screenX, int screenY) const = 0;
};


#endif	// __EDLIBRARYITEM_H
