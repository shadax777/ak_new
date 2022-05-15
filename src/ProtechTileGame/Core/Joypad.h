#ifndef __JOYPAD_H
#define __JOYPAD_H


// simulation of the Sega Master System joypad


enum JoypadButton
{
	kJoypadButton_None	= -1,
	kJoypadButton_Up	= 0,
	kJoypadButton_Down,
	kJoypadButton_Left,
	kJoypadButton_Right,
	kJoypadButton_Fire1,	// attack
	kJoypadButton_Fire2,	// jump
	kJoypadButton_Pause,
	kJoypadButtonCount
};


class JoypadConfig
{
private:
	static PTString		s_cvars[kJoypadButtonCount];

public:
	static void			ExposeToCommandSystem();
	static JoypadButton	GetJoypadButtonForPTKeyNum(PTKeyNum keyNum);	// returns kJoypadButton_None if given PTKeyNum is not mapped to a JoypadButton
	static void			GetPTKeyNumsForJoypadButton(JoypadButton button, PTKeyNum *outKeyNums, int maxKeys);
	static void			SetPTKeyNumsForJoypadButton(JoypadButton button, const PTKeyNum *keyNums, int numKeys);
};


class JoypadState
{
private:
	bool	m_currentButtons[kJoypadButtonCount];
	bool	m_oldButtons[kJoypadButtonCount];

public:
			JoypadState();
	void	UpdateFromGameInput(const GameInput &gameInput);
	bool	ButtonIsDown(JoypadButton button) const      { return m_currentButtons[button]; }
	bool	ButtonToggledDown(JoypadButton button) const { return !m_oldButtons[button] && m_currentButtons[button]; }
	bool	ButtonToggledUp(JoypadButton button) const   { return m_oldButtons[button] && !m_currentButtons[button]; }
};


#endif	// __JOYPAD_H
