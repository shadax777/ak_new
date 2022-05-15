#include "../../GameLocal.h"
#pragma hdrstop
#include "../EditorLocal.h"


/////////////////////////////////////////////////////////////////////
//
// EdState_RoomPickBGColor::MyColorLibraryItem
//
/////////////////////////////////////////////////////////////////////

const int	EdState_RoomPickBGColor::MyColorLibraryItem::s_sizeWithoutBorder = 32;
const int	EdState_RoomPickBGColor::MyColorLibraryItem::s_spacingSize = 4;

EdState_RoomPickBGColor::MyColorLibraryItem::MyColorLibraryItem(const Color &color)
: m_color(color),
m_name(VABuf("%u %u %u", color.r, color.g, color.b))
{
}

const char *EdState_RoomPickBGColor::MyColorLibraryItem::GetName() const
{
	return m_name.c_str();
}

int EdState_RoomPickBGColor::MyColorLibraryItem::GetWidth() const
{
	return s_sizeWithoutBorder + s_spacingSize * 2;
}

int EdState_RoomPickBGColor::MyColorLibraryItem::GetHeight() const
{
	return s_sizeWithoutBorder + s_spacingSize * 2;
}

void EdState_RoomPickBGColor::MyColorLibraryItem::Draw(int screenX, int screenY) const
{
	int realX = screenX + s_spacingSize;
	int realY = screenY + s_spacingSize;
	g_renderSystem->DrawRectangle2D(realX, realY, realX + s_sizeWithoutBorder, realY + s_sizeWithoutBorder, m_color, true);
}


/////////////////////////////////////////////////////////////////////
//
// EdState_RoomPickBGColor
//
/////////////////////////////////////////////////////////////////////

EdState_RoomPickBGColor::EdState_RoomPickBGColor()
{
	m_colorLibraryItems.push_back(new MyColorLibraryItem(Color::FromBytes(  0,   0, 255)));	// blue
	m_colorLibraryItems.push_back(new MyColorLibraryItem(Color::FromBytes(0, 88, 248)));	// slightly dark blue
	m_colorLibraryItems.push_back(new MyColorLibraryItem(Color::FromBytes( 85,  85, 170)));	// light purple
	m_colorLibraryItems.push_back(new MyColorLibraryItem(Color::FromBytes(0, 127, 127)));	// cyan
	m_colorLibraryItems.push_back(new MyColorLibraryItem(Color::FromBytes(0, 232, 216)));	// slightly dark cyan

	m_colorLibraryItems.push_back(new MyColorLibraryItem(Color::FromBytes(228, 92, 16)));	// light orange
	m_colorLibraryItems.push_back(new MyColorLibraryItem(Color::FromBytes(136, 20, 0)));	// orange / brown
	m_colorLibraryItems.push_back(new MyColorLibraryItem(Color::FromBytes(127,  0,  0)));	// dark red
	m_colorLibraryItems.push_back(new MyColorLibraryItem(Color::FromBytes( 85,  0,  0)));	// even darker red
	m_colorLibraryItems.push_back(new MyColorLibraryItem(Color::FromBytes(80, 48, 0)));		// dark brown

	m_colorLibraryItems.push_back(new MyColorLibraryItem(Color::FromBytes(  0,  85,   0)));	// very dark green
	m_colorLibraryItems.push_back(new MyColorLibraryItem(Color::FromBytes(  0, 127,   0)));	// mid green dark green
	m_colorLibraryItems.push_back(new MyColorLibraryItem(Color::FromBytes(  0, 184,   0)));	// slightly light green
	m_colorLibraryItems.push_back(new MyColorLibraryItem(Color::FromBytes(184, 248, 24)));	// light green

	m_colorLibraryItems.push_back(new MyColorLibraryItem(Color::FromBytes(248, 184, 0)));	// dark yellow
	m_colorLibraryItems.push_back(new MyColorLibraryItem(Color::FromBytes(172, 124, 0)));	// very light brown

	for(size_t i = 0; i < m_colorLibraryItems.size(); i++)
	{
		m_colorLibraryManager.RegisterItem(m_colorLibraryItems[i]);
	}
}

EdState_RoomPickBGColor::~EdState_RoomPickBGColor()
{
	pt_for_each(m_colorLibraryItems, pt_delete_and_nullify<MyColorLibraryItem>);
}

void EdState_RoomPickBGColor::Enter(const EdContext &ctx)
{
	// select the item matching the currently selected bg color
	m_colorLibraryManager.UnselectItem();
	for(size_t i = 0; i < m_colorLibraryItems.size(); i++)
	{
		if(m_colorLibraryItems[i]->Col() == ctx.stateBB->roomBgColor)
		{
			m_colorLibraryManager.SelectItemByIndex((int)i);
			break;
		}
	}
}

void EdState_RoomPickBGColor::Update(const EdContext &ctx)
{
	m_colorLibraryManager.UpdateScroll(ctx);

	if(ctx.currentInput->MouseButtonToggledDown(MOUSEBUTTON_LEFT))
	{
		m_colorLibraryManager.TrySelectTouchedItem(ctx);
		if(m_colorLibraryManager.SelectedItemIndex() != -1)
		{
			ctx.stateBB->roomBgColor = m_colorLibraryItems[m_colorLibraryManager.SelectedItemIndex()]->Col();
		}
	}
}


void EdState_RoomPickBGColor::Draw(const EdContext &ctx)
{
	m_colorLibraryManager.Draw(ctx);
	EdStateHelper::DrawMouseCursor(ctx);
}

void EdState_RoomPickBGColor::Exit(const EdContext &ctx)
{
}
