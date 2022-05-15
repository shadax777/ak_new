#include "../GameLocal.h"
#pragma hdrstop


/////////////////////////////////////////////////////////////////////
//
// JoypadConfig
//
/////////////////////////////////////////////////////////////////////

PTString JoypadConfig::s_cvars[];

void JoypadConfig::ExposeToCommandSystem()
{
	// set default PTKeyNums for all game buttons
	s_cvars[kJoypadButton_Up] = Key_Num2Str(PT_KEY_UP);
	s_cvars[kJoypadButton_Down] = Key_Num2Str(PT_KEY_DOWN);
	s_cvars[kJoypadButton_Left] = Key_Num2Str(PT_KEY_LEFT);
	s_cvars[kJoypadButton_Right] = Key_Num2Str(PT_KEY_RIGHT);
	s_cvars[kJoypadButton_Fire1] = Key_Num2Str(PT_KEY_CONTROL);
	s_cvars[kJoypadButton_Fire2] = Key_Num2Str(PT_KEY_ALT);
	s_cvars[kJoypadButton_Pause] = Key_Num2Str(PT_KEY_ENTER);

	for(int i = 0; i < kJoypadButtonCount; i++)
	{
		CmdVar::RegisterStdString(va("_g_gamePadButton_%i", i), s_cvars[i], true, "");
	}
}

JoypadButton JoypadConfig::GetJoypadButtonForPTKeyNum(PTKeyNum keyNum)
{
	for(int button = 0; button < kJoypadButtonCount; button++)
	{
		const char *start = s_cvars[button].c_str();
		char keyName[64];
		while((start = Str_NextWord(start, keyName)) != NULL && keyName[0] != '\0')
		{
			if(Key_Str2Num(keyName) == keyNum)
				return (JoypadButton)button;
		}
	}
	return kJoypadButton_None;
}

void JoypadConfig::GetPTKeyNumsForJoypadButton(JoypadButton button, PTKeyNum *outKeyNums, int maxKeys)
{
	std::fill_n(outKeyNums, maxKeys, PT_KEY_NONE);

	const char *start = s_cvars[button].c_str();
	char keyName[64];
	int n = 0;
	while((start = Str_NextWord(start, keyName)) != NULL && keyName[0] != '\0' && n < maxKeys)
	{
		outKeyNums[n++] = Key_Str2Num(keyName);
	}
}

void JoypadConfig::SetPTKeyNumsForJoypadButton(JoypadButton button, const PTKeyNum *keyNums, int numKeys)
{
	s_cvars[button] = "";
	for(int i = 0; i < numKeys; i++)
	{
		if(keyNums[i] != PT_KEY_NONE)
		{
			s_cvars[button] += Key_Num2Str(keyNums[i]);
			if(i < numKeys - 1)
				s_cvars[button] += " ";
		}
	}
}


/////////////////////////////////////////////////////////////////////
//
// JoypadState
//
/////////////////////////////////////////////////////////////////////

JoypadState::JoypadState()
{
	MEM_FillArray(m_currentButtons, false);
	MEM_FillArray(m_oldButtons, false);
}

void JoypadState::UpdateFromGameInput(const GameInput &gameInput)
{
	// backup old button states
	MEM_CopyArray(m_oldButtons, m_currentButtons);

	// update current button states
	for(size_t i = 0; i < gameInput.keyEvents.size(); i++)
	{
		const KeyEvent &ev = gameInput.keyEvents[i];
		JoypadButton button = JoypadConfig::GetJoypadButtonForPTKeyNum(ev.keyNum);
		if(button != kJoypadButton_None)
		{
			m_currentButtons[button] = ev.down;
		}
	}
}
