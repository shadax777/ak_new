#ifndef __EDSTATE_MANIPULATEROOMSINWORLD_H
#define __EDSTATE_MANIPULATEROOMSINWORLD_H


class EdState_ManipulateRoomsInWorld : public EdState
{
private:
	class MyRoomScrollTypeButton
	{
	private:
		static const int	s_width;
		static const int	s_height;

		byte_t				m_roomScrollFlag;		// one of the kRollScrollFlag_*
		int					m_buttonPosXInRoom;		// left side
		int					m_buttonPosYInRoom;		// upper side

	public:
		explicit			MyRoomScrollTypeButton(byte_t scrollFlag);
		bool				IsTouchedByMouse(int roomIndexX, int roomIndexY, const EdContext &ctx) const;
		byte_t				ScrollFlag() const { return m_roomScrollFlag; }
		void				AddButtonBorderToGameRenderWorld(GameRenderWorld2D &rw, int roomIndexX, int roomIndexY) const;
		void				AddButtonTextToGameRenderWorld(GameRenderWorld2D &rw, int roomIndexX, int roomIndexY, const char *text, const EdContext &ctx) const;
	};

	//----------------------------------------------

	MyRoomScrollTypeButton *		m_buttons[4];	// buttons to change all 4 outgoing connections of the currently touched room

	void							updateGameRenderWorld(const EdContext &ctx) const;
	const MyRoomScrollTypeButton *	getTouchedButton(const EdContext &ctx) const;

	static const Room *				getTouchedRoomIndex(const EdContext &ctx, int *outRoomIndexX, int *outRoomIndexY);

public:
									EdState_ManipulateRoomsInWorld();
									~EdState_ManipulateRoomsInWorld();

	// implement EdState
	virtual void					Enter(const EdContext &ctx);
	virtual void					Update(const EdContext &ctx);
	virtual void					Draw(const EdContext &ctx);
	virtual void					Exit(const EdContext &ctx);
};


#endif	// __EDSTATE_MANIPULATEROOMSINWORLD_H
