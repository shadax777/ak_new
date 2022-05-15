#ifndef __EDINPUT_H
#define __EDINPUT_H

// customized GameInput

struct EdInput
{
	bool	currentKeyMatrix[PT_NUM_KEYS];
	bool	oldKeyMatrix[PT_NUM_KEYS];

	bool	currentMouseButtons[NUM_MOUSE_BUTTONS];
	bool	oldMouseButtons[NUM_MOUSE_BUTTONS];

	float	mouseMoveDX;
	float	mouseMoveDY;

	int		mouseWheelDelta;		// 0 (not scrolling), >0 (scrolling down) or <0 (scroll up)

			EdInput()											{ static EdInput defaulted; *this = defaulted; }

	void	UpdateFromGameInput(const GameInput &gameInput);

	bool	KeyToggledDown(PTKeyNum keyNum) const				{ return !oldKeyMatrix[keyNum] && currentKeyMatrix[keyNum]; }
	bool	KeyToggledUp(PTKeyNum keyNum) const					{ return oldKeyMatrix[keyNum] && !currentKeyMatrix[keyNum]; }

	bool	MouseButtonToggledDown(MouseButtonNum btn) const	{ return !oldMouseButtons[btn] && currentMouseButtons[btn]; }
	bool	MouseButtonToggledUp(MouseButtonNum btn) const		{ return oldMouseButtons[btn] && !currentMouseButtons[btn]; }
};


inline void EdInput::UpdateFromGameInput(const GameInput &gameInput)
{
	// backup old input states
	MEM_CopyArray(this->oldKeyMatrix, this->currentKeyMatrix);
	MEM_CopyArray(this->oldMouseButtons, this->currentMouseButtons);

	// refresh current input states
	MEM_CopyArray(this->currentKeyMatrix, gameInput.currentKeyMatrix);
	MEM_CopyArray(this->currentMouseButtons, gameInput.currentMouseButtons);

	// update current mouse movement and wheel delta
	this->mouseMoveDX = 0.0f;
	this->mouseMoveDY = 0.0f;
	this->mouseWheelDelta = 0;
	for(size_t i = 0; i < gameInput.mouseEvents.size(); i++)
	{
		if(gameInput.mouseEvents[i].type == MouseEvent::TYPE_MOTION)
		{
			this->mouseMoveDX += gameInput.mouseEvents[i].motion.dx;
			this->mouseMoveDY += gameInput.mouseEvents[i].motion.dy;
		}
		else if(gameInput.mouseEvents[i].type == MouseEvent::TYPE_WHEEL)
		{
			this->mouseWheelDelta += gameInput.mouseEvents[i].wheel.delta;
		}
	}
}


#endif	// __EDINPUT_H
