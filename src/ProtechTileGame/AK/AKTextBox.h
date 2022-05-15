#ifndef __AKTEXTBOX_H
#define __AKTEXTBOX_H


class AKTextBox
{
private:
	enum MyState
	{
		kMyState_NotRunning,
		kMyState_RevealingCells,
		kMyState_AllCellsRevealed
	};

	struct MyCell
	{
		int				x, y;	// on screen
		char			ch;		// single character of the whole text at x/y
						MyCell(int _x, int _y, char _ch) : x(_x), y(_y), ch(_ch) {}
	};

	MyState				m_state;
	std::vector<MyCell>	m_cells;
	size_t				m_numRevealedCellsSoFar;
	SoundSource			m_sound;

	void				addCell(int x, int y, char ch);
	void				buildCells(const char *text);

public:
						AKTextBox();
	void				Start(const char *text);
	bool				IsActive() const { return (m_state != kMyState_NotRunning); }
	void				Update(const JoypadState &joypad);
	void				Stop();
	void				Draw(const Font &font) const;
};


#endif	// __AKTEXTBOX_H
