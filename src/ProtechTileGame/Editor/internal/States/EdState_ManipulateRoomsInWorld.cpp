#include "../../GameLocal.h"
#pragma hdrstop
#include "../EditorLocal.h"


/////////////////////////////////////////////////////////////////////
//
// EdState_ManipulateRoomsInWorld::MyRoomScrollTypeButton
//
/////////////////////////////////////////////////////////////////////

const int	EdState_ManipulateRoomsInWorld::MyRoomScrollTypeButton::s_width = 48;
const int	EdState_ManipulateRoomsInWorld::MyRoomScrollTypeButton::s_height = 48;

EdState_ManipulateRoomsInWorld::MyRoomScrollTypeButton::MyRoomScrollTypeButton(byte_t scrollFlag)
: m_roomScrollFlag(scrollFlag)
{
	switch(m_roomScrollFlag)
	{
	case kRoomScrollFlag_Up:
		m_buttonPosXInRoom = ROOM_WIDTH_IN_PIXELS / 2 - s_width / 2;
		m_buttonPosYInRoom = 0;
		break;

	case kRoomScrollFlag_Down:
		m_buttonPosXInRoom = ROOM_WIDTH_IN_PIXELS / 2 - s_width / 2;
		m_buttonPosYInRoom = ROOM_HEIGHT_IN_PIXELS - s_height;
		break;

	case kRoomScrollFlag_Left:
		m_buttonPosXInRoom = 0;
		m_buttonPosYInRoom = ROOM_HEIGHT_IN_PIXELS / 2 - s_height / 2;
		break;

	case kRoomScrollFlag_Right:
		m_buttonPosXInRoom = ROOM_WIDTH_IN_PIXELS - s_width;
		m_buttonPosYInRoom = ROOM_HEIGHT_IN_PIXELS / 2 - s_height / 2;
		break;

	default:
		pt_assert(0);
	}
}

bool EdState_ManipulateRoomsInWorld::MyRoomScrollTypeButton::IsTouchedByMouse(int roomIndexX, int roomIndexY, const EdContext &ctx) const
{
	int x1 = roomIndexX * ROOM_WIDTH_IN_PIXELS + m_buttonPosXInRoom;
	int y1 = roomIndexY * ROOM_HEIGHT_IN_PIXELS + m_buttonPosYInRoom;
	int x2 = x1 + s_width;
	int y2 = y1 + s_height;

	if(ctx.mousePosInWorld[0] < x1)
		return false;

	if(ctx.mousePosInWorld[1] < y1)
		return false;

	if(ctx.mousePosInWorld[0] > x2)
		return false;

	if(ctx.mousePosInWorld[1] > y2)
		return false;

	return true;
}

void EdState_ManipulateRoomsInWorld::MyRoomScrollTypeButton::AddButtonBorderToGameRenderWorld(GameRenderWorld2D &rw, int roomIndexX, int roomIndexY) const
{
	int x1 = roomIndexX * ROOM_WIDTH_IN_PIXELS + m_buttonPosXInRoom;
	int y1 = roomIndexY * ROOM_HEIGHT_IN_PIXELS + m_buttonPosYInRoom;
	int x2 = x1 + s_width;
	int y2 = y1 + s_height;
	rw.AddDebugRect(x1, y1, x2, y2, g_color_blue);
}

void EdState_ManipulateRoomsInWorld::MyRoomScrollTypeButton::AddButtonTextToGameRenderWorld(GameRenderWorld2D &rw, int roomIndexX, int roomIndexY, const char *text, const EdContext &ctx) const
{
	int buttonCenterPosX = roomIndexX * ROOM_WIDTH_IN_PIXELS + m_buttonPosXInRoom + s_width / 2;
	int buttonCenterPosY = roomIndexY * ROOM_HEIGHT_IN_PIXELS + m_buttonPosYInRoom + s_height / 2;
	rw.AddDebugTextBig(buttonCenterPosX, buttonCenterPosY, true, true, text, *ctx.font);
}


/////////////////////////////////////////////////////////////////////
//
// EdState_ManipulateRoomsInWorld
//
/////////////////////////////////////////////////////////////////////


EdState_ManipulateRoomsInWorld::EdState_ManipulateRoomsInWorld()
{
	MEM_ZeroOutArray(m_buttons);
	for(int i = 0; i < 4; i++)
	{
		m_buttons[i] = new MyRoomScrollTypeButton(BIT(i));
	}
}

EdState_ManipulateRoomsInWorld::~EdState_ManipulateRoomsInWorld()
{
	for(int i = 0; i < NELEMSi(m_buttons); i++)
	{
		delete m_buttons[i];
	}
}

const EdState_ManipulateRoomsInWorld::MyRoomScrollTypeButton *EdState_ManipulateRoomsInWorld::getTouchedButton(const EdContext &ctx) const
{
	int roomIndexX, roomIndexY;
	if(getTouchedRoomIndex(ctx, &roomIndexX, &roomIndexY) != NULL)
	{
		for(int i = 0; i < 4; i++)
		{
			if(m_buttons[i]->IsTouchedByMouse(roomIndexX, roomIndexY, ctx))
			{
				return m_buttons[i];
			}
		}
	}
	return NULL;
}

const Room *EdState_ManipulateRoomsInWorld::getTouchedRoomIndex(const EdContext &ctx, int *outRoomIndexX, int *outRoomIndexY)
{
	if(ctx.mousePosInWorld[0] >= 0 &&
		ctx.mousePosInWorld[1] >= 0 &&
		ctx.mousePosInWorld[0] < WORLD_WIDTH_IN_PIXELS &&
		ctx.mousePosInWorld[1] < WORLD_HEIGHT_IN_PIXELS)
	{
		int roomIndexX = ctx.mousePosInWorld[0] / ROOM_WIDTH_IN_PIXELS;
		int roomIndexY = ctx.mousePosInWorld[1] / ROOM_HEIGHT_IN_PIXELS;
		if(outRoomIndexX != NULL)
			*outRoomIndexX = roomIndexX;
		if(outRoomIndexY != NULL)
			*outRoomIndexY = roomIndexY;
		return &ctx.levelToEdit->roomMatrix.GetRoomByIndex(roomIndexX, roomIndexY);
	}
	else
	{
		return NULL;
	}
}

void EdState_ManipulateRoomsInWorld::Enter(const EdContext &ctx)
{
}

void EdState_ManipulateRoomsInWorld::Update(const EdContext &ctx)
{
	EdStateHelper::MoveCameraByInput(ctx);

	// If the mouse is currently touching a room, see if the user wants to change any of the room's 4 connection scroll types
	// by clicking on the according button. If he clicks somewhere else, the background color of the room will be changed.
	if(ctx.currentInput->MouseButtonToggledDown(MOUSEBUTTON_LEFT))
	{
		if(const Room *touchedRoom = getTouchedRoomIndex(ctx, NULL, NULL))
		{
			if(const MyRoomScrollTypeButton *clickedButton = getTouchedButton(ctx))
			{
				// toggle scrolling to that direction on/off
				ctx.levelToEdit->roomMatrix.ToggleRoomScroll(touchedRoom->number, clickedButton->ScrollFlag());
			}
			else
			{
				// copy the bg color from the current room?
				if(ctx.currentInput->currentKeyMatrix[PT_KEY_SHIFT])
				{
					ctx.stateBB->roomBgColor = touchedRoom->bgColor;
				}
				else
				{
					// change the bg color of the current room
					ctx.levelToEdit->roomMatrix.ChangeRoomBgColor(touchedRoom->number, ctx.stateBB->roomBgColor);
				}
			}
		}
	}
	updateGameRenderWorld(ctx);
}

void EdState_ManipulateRoomsInWorld::updateGameRenderWorld(const EdContext &ctx) const
{
	EdStateHelper::AddRoomBackgroundsToGameRenderWorld(ctx);
	EdStateHelper::AddTileLayerToEditToGameRenderWorld(ctx);
	EdStateHelper::AddTileLayerGridLinesToGameRenderWorld(ctx);
	EdStateHelper::AddRoomRectangleToGameRenderWorld(ctx);

	// draw the borders of the currently touched room connection
	int roomIndexX, roomIndexY;
	if(const Room *touchedRoom = getTouchedRoomIndex(ctx, &roomIndexX, &roomIndexY))
	{
		for(int i = 0; i < 4; i++)
		{
			// button border if button is touched
			if(m_buttons[i]->IsTouchedByMouse(roomIndexX, roomIndexY, ctx))
			{
				m_buttons[i]->AddButtonBorderToGameRenderWorld(*ctx.gameRenderWorld, roomIndexX, roomIndexY);
			}
		}
	}

	// draw the button texts of all 4 connections of all rooms
	for(int roomIndexX = 0; roomIndexX < WORLD_WIDTH_IN_ROOMS; roomIndexX++)
	{
		for(int roomIndexY = 0; roomIndexY < WORLD_HEIGHT_IN_ROOMS; roomIndexY++)
		{
			const Room &room = ctx.levelToEdit->roomMatrix.GetRoomByIndex(roomIndexX, roomIndexY);
			for(int directionType = 0; directionType < 4; directionType++)
			{
				// button text for scrolling on/off
				const char *text = (room.scrollFlags & BIT(directionType)) ? "true" : "false";
				m_buttons[directionType]->AddButtonTextToGameRenderWorld(*ctx.gameRenderWorld, roomIndexX, roomIndexY, text, ctx);
			}
		}
	}
}

void EdState_ManipulateRoomsInWorld::Draw(const EdContext &ctx)
{
	ctx.gameRenderWorld->RenderScene(*ctx.gameRenderView);

	EdStateHelper::DrawCaption(ctx, "Room Editor");

	// check for whether the mouse cursor is touching one of the room connection buttons
	bool mouseTouchingAButton = false;
	int roomIndexX, roomIndexY;
	if(const Room *touchedRoom = getTouchedRoomIndex(ctx, &roomIndexX, &roomIndexY))
	{
		for(int i = 0; i < 4; i++)
		{
			if(m_buttons[i]->IsTouchedByMouse(roomIndexX, roomIndexY, ctx))
			{
				mouseTouchingAButton = true;
				break;
			}
		}
	}

	// mouse cursor
	if(mouseTouchingAButton)
	{
		EdStateHelper::DrawMouseCursor(ctx);
	}
	else
	{
		EdStateHelper::DrawMouseFloodFill(ctx, ctx.stateBB->roomBgColor);
		if(ctx.currentInput->currentKeyMatrix[PT_KEY_SHIFT])
		{
			EdStateHelper::DrawMouseCursorCopySymbol(ctx);
		}
	}
}

void EdState_ManipulateRoomsInWorld::Exit(const EdContext &ctx)
{
}
