#ifndef __ROOM_H
#define __ROOM_H

/*
specification of the RoomMatrix format (standalone files with .rooms extension)

//
// room index
// bgColor
// can scroll to upper room
// can scroll to below room
// can scroll to left room
// can scroll to right room
//

e. g. for 2 rooms:

	"0 0"
	"127 127 0"
	"false"
	"false"
	"false"
	"true"

	"1 0"
	"127 127 0"
	"true"
	"false"
	"false"
	"false"

	// ...
*/


// flags that specify in which direction the camera may scroll in a given room
enum
{
	kRoomScrollFlag_Up		= BIT(0),	// the room above the current one
	kRoomScrollFlag_Down	= BIT(1),	// the room below the current one
	kRoomScrollFlag_Left	= BIT(2),	// the room left to the current one
	kRoomScrollFlag_Right	= BIT(3)	// the room right to the current one
};


struct Room
{
	int		number;			// in the RoomMatrix
	Color	bgColor;
	byte_t	scrollFlags;

	Room() { static Room zeroed; *this = zeroed; }
};


// all rooms in a world
class RoomMatrix
{
private:
	std::vector<Room>	m_rooms;	// 2-dimensional fixed-size array (WORLD_WIDTH_IN_ROOMS * WORLD_HEIGHT_IN_ROOMS)

public:
						RoomMatrix();

	void				LoadFromFile(const char *roomsFileName);		// .rooms file
	void				SaveToFile(const char *roomsFileName) const;	// .rooms file

	const Room &		GetRoomByIndex(int roomIndexX, int roomIndexY) const;
	const Room *		TryGetRoomByIndex(int roomIndexX, int roomIndexY) const;
	const Room *		TryGetRoomAtWorldPos(const Vec2 &pos, int *outRoomIndexX = NULL, int *outRoomIndexY = NULL) const;

	//===============================================================
	//
	// editor-only related methods
	//
	//===============================================================

	void				ChangeRoomBgColor(int roomNumber, const Color &newBgColor);
	void				ToggleRoomScroll(int roomNumber, byte_t scrollFlag);
};


#endif	// __ROOM_H
