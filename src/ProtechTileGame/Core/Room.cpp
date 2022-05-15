#include "../GameLocal.h"
#pragma hdrstop


RoomMatrix::RoomMatrix()
: m_rooms(WORLD_WIDTH_IN_ROOMS * WORLD_HEIGHT_IN_ROOMS)
{
	for(int i = 0; i < WORLD_WIDTH_IN_ROOMS * WORLD_HEIGHT_IN_ROOMS; i++)
	{
		m_rooms[i].number = i;
	}
}

void RoomMatrix::LoadFromFile(const char *roomsFileName)
{
	PTString fileContent;
	FS_GetFileContentAsText(roomsFileName, fileContent);

	//Timer timer(true);

	Tokenizer tokenizer(fileContent.c_str());
	PTString token;
	while(tokenizer.Next(token, false))
	{
		// room index (x, y)
		int roomIndexX, roomIndexY;
		if(sscanf(token.c_str(), "%i %i", &roomIndexX, &roomIndexY) != 2)
		{
			throw PTException(VABuf("%s line #%i: bad room-index: '%s'", roomsFileName, tokenizer.CurrentLineNum(), token.c_str()));
		}
		if(roomIndexX < 0 || roomIndexY < 0 || roomIndexX >= WORLD_WIDTH_IN_ROOMS || roomIndexY >= WORLD_HEIGHT_IN_ROOMS)
		{
			LOG_Warning("%s line #%i: room index %i %i is outside world boundaries\n", roomsFileName, tokenizer.CurrentLineNum(), roomIndexX, roomIndexY);
			roomIndexX = roomIndexY = 0;
		}

		Room &targetRoom = m_rooms[roomIndexY * WORLD_WIDTH_IN_ROOMS + roomIndexX];

		// bgColor
		token = tokenizer.Next();
		int r, g, b;
		if(sscanf(token.c_str(), "%i %i %i", &r, &g, &b) != 3)
		{
			throw PTException(VABuf("%s line #%i: bad bgColor format: '%s'\n", roomsFileName, tokenizer.CurrentLineNum(), token.c_str()));
		}
		targetRoom.bgColor.SetBytes((byte_t)r, (byte_t)g, (byte_t)b);

		// connection to upper room
		token = tokenizer.Next();
		if(Str_ParseBool(token.c_str(), false))
			targetRoom.scrollFlags |= kRoomScrollFlag_Up;

		// connection to lower room
		token = tokenizer.Next();
		if(Str_ParseBool(token.c_str(), false))
			targetRoom.scrollFlags |= kRoomScrollFlag_Down;

		// connection to left room
		token = tokenizer.Next();
		if(Str_ParseBool(token.c_str(), false))
			targetRoom.scrollFlags |= kRoomScrollFlag_Left;

		// connection to right room
		token = tokenizer.Next();
		if(Str_ParseBool(token.c_str(), false))
			targetRoom.scrollFlags |= kRoomScrollFlag_Right;
	}
	//LOG_Printf(COLOR_STR_CYAN "RoomMatrix::LoadFromFile: %f milliseconds\n", timer.Milliseconds());
}

void RoomMatrix::SaveToFile(const char *roomsFileName) const
{
	OutputFile f(roomsFileName, true);

	f.PutStr("//\n");
	f.PutStr("// room index\n");
	f.PutStr("// bgColor\n");
	f.PutStr("// can scroll to upper room\n");
	f.PutStr("// can scroll to below room\n");
	f.PutStr("// can scroll to left room\n");
	f.PutStr("// can scroll to right room\n");
	f.PutStr("//\n");
	f.PutStr("\n");

	for(int ix = 0; ix < WORLD_WIDTH_IN_ROOMS; ix++)
	{
		for(int iy = 0; iy < WORLD_HEIGHT_IN_ROOMS; iy++)
		{
			const Room &room = m_rooms[iy * WORLD_WIDTH_IN_ROOMS + ix];
			f.PutStr(VABuf("\"%i %i\"\n", ix, iy));
			f.PutStr(VABuf("\"%i %i %i\"\n", room.bgColor.r, room.bgColor.g, room.bgColor.b));
			f.PutStr(VABuf("%s\n", (room.scrollFlags & kRoomScrollFlag_Up) ? "true" : "false"));
			f.PutStr(VABuf("%s\n", (room.scrollFlags & kRoomScrollFlag_Down) ? "true" : "false"));
			f.PutStr(VABuf("%s\n", (room.scrollFlags & kRoomScrollFlag_Left) ? "true" : "false"));
			f.PutStr(VABuf("%s\n", (room.scrollFlags & kRoomScrollFlag_Right) ? "true" : "false"));
			f.PutStr("\n");
		}
	}
}

const Room &RoomMatrix::GetRoomByIndex(int roomIndexX, int roomIndexY) const
{
	pt_assert(roomIndexX >= 0);
	pt_assert(roomIndexY >= 0);
	pt_assert(roomIndexX < WORLD_WIDTH_IN_ROOMS);
	pt_assert(roomIndexY < WORLD_HEIGHT_IN_ROOMS);
	return m_rooms[roomIndexY * WORLD_WIDTH_IN_ROOMS + roomIndexX];
}

const Room *RoomMatrix::TryGetRoomByIndex(int roomIndexX, int roomIndexY) const
{
	if(roomIndexX < 0)
		return NULL;
	if(roomIndexY < 0)
		return NULL;
	if(roomIndexX >= WORLD_WIDTH_IN_ROOMS)
		return NULL;
	if(roomIndexY >= WORLD_HEIGHT_IN_ROOMS)
		return NULL;
	return &m_rooms[roomIndexY * WORLD_WIDTH_IN_ROOMS + roomIndexX];
}

const Room *RoomMatrix::TryGetRoomAtWorldPos(const Vec2 &pos, int *outRoomIndexX /* = NULL */, int *outRoomIndexY /* = NULL */) const
{
	int roomIndexX = (int)pos.x / ROOM_WIDTH_IN_PIXELS;
	int roomIndexY = (int)pos.y / ROOM_HEIGHT_IN_PIXELS;

	if(outRoomIndexX != NULL)
		*outRoomIndexX = roomIndexX;
	if(outRoomIndexY != NULL)
		*outRoomIndexY = roomIndexY;

	if(roomIndexX < 0)
		return NULL;
	if(roomIndexY < 0)
		return NULL;
	if(roomIndexX >= WORLD_WIDTH_IN_ROOMS)
		return NULL;
	if(roomIndexY >= WORLD_HEIGHT_IN_ROOMS)
		return NULL;
	return &m_rooms[roomIndexY * WORLD_WIDTH_IN_ROOMS + roomIndexX];
}

void RoomMatrix::ChangeRoomBgColor(int roomNumber, const Color &newBgColor)
{
	pt_assert(roomNumber >= 0);
	pt_assert(roomNumber < WORLD_WIDTH_IN_ROOMS * WORLD_HEIGHT_IN_ROOMS);
	m_rooms[roomNumber].bgColor = newBgColor;
}

void RoomMatrix::ToggleRoomScroll(int roomNumber, byte_t scrollFlag)
{
	pt_assert(roomNumber >= 0);
	pt_assert(roomNumber < WORLD_WIDTH_IN_ROOMS * WORLD_HEIGHT_IN_ROOMS);
	m_rooms[roomNumber].scrollFlags ^= scrollFlag;
}
