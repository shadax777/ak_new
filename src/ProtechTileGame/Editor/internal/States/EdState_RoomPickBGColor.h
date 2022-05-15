#ifndef __EDSTATE_ROOMPICKBGCOLOR_H
#define __EDSTATE_ROOMPICKBGCOLOR_H


class EdState_RoomPickBGColor : public EdState
{
private:
	class MyColorLibraryItem : public EdLibraryItem
	{
	private:
		static const int				s_sizeWithoutBorder;	// width and height alike
		static const int				s_spacingSize;

		Color							m_color;
		PTString						m_name;

	public:
		explicit						MyColorLibraryItem(const Color &color);

		// implement EdLibraryItem
		virtual const char *			GetName() const;
		virtual int						GetWidth() const;
		virtual int						GetHeight() const;
		virtual void					Draw(int screenX, int screenY) const;

		const Color &					Col() const { return m_color; }
	};

	//---------------------------------------------------------------

	std::vector<MyColorLibraryItem *>	m_colorLibraryItems;
	EdLibraryManager					m_colorLibraryManager;

public:
										EdState_RoomPickBGColor();
										~EdState_RoomPickBGColor();

	// implement EdState
	virtual void						Enter(const EdContext &ctx);
	virtual void						Update(const EdContext &ctx);
	virtual void						Draw(const EdContext &ctx);
	virtual void						Exit(const EdContext &ctx);
};


#endif	// __EDSTATE_ROOMPICKBGCOLOR_H
