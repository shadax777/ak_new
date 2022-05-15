#include "../GameLocal.h"
#pragma hdrstop
#include "AKLocal.h"


AKTextBox::AKTextBox()
: m_state(kMyState_NotRunning),
m_numRevealedCellsSoFar(0)
{
	// nothing
}

void AKTextBox::Start(const char *text)
{
	m_numRevealedCellsSoFar = 0;
	m_cells.clear();
	buildCells(text);
	m_state = kMyState_RevealingCells;
	m_sound.Play(AK_SND_TEXTBOX, true);
}

void AKTextBox::addCell(int x, int y, char ch)
{
	m_cells.push_back(MyCell(x, y, ch));
}

void AKTextBox::buildCells(const char *text)
{
	// compute the max. no. of chars per text line and the total no. of text lines
	int maxCharsPerTextLine = 0;
	int numTextLines = 1;	// one text line is always present (even for zero-length text)
	for(const char *c = text, *lineBeginning = text; ; c++)
	{
		if(*c == '\n' || *c == '\0')
		{
			int charsPerCurLine = (int)(c - lineBeginning);
			if(charsPerCurLine > maxCharsPerTextLine)
				maxCharsPerTextLine = charsPerCurLine;
			lineBeginning = c + 1;
			if(*c == '\n')
				numTextLines++;
			else
				break;	// reached end of string
		}
	}

	int numCols = maxCharsPerTextLine + 2;	// +2 for left and right border
	int numRows = 2 * numTextLines + 1;

	pt_assert(numCols > 0);
	pt_assert(numRows > 0);

	const int START_X = 16;
	const int START_Y = 16;

	int x, y;

	// build upper border
	x = START_X - 8;
	y = START_Y;
	for(int i = 0; i < numCols; i++)
	{
		x += 8;
		addCell(x, y, ' ');
	}

	// build right border (exclusive top and bottom cell - since got/get built/build by the top/bottom border)
	for(int i = 1; i < numRows - 1; i++)
	{
		y += 8;
		addCell(x, y, ' ');
	}

	// build bottom border
	y += 8;
	for(int i = 0; i < numCols; i++)
	{
		addCell(x, y, ' ');
		x -= 8;
	}

	// build left border (exclusive top and bottom cell, which got already built by top and bottom border)
	x += 8;
	for(int i = 1; i < numRows - 1; i++)
	{
		y -= 8;
		addCell(x, y, ' ');
	}

	// build text lines and empty lines in an alternating manner
	// (text lines go from left to right, empty lines from right to left)
	const char *curTextLineChar = text;
	for(int row = 1; row < numRows - 1; row++)
	{
		if(row & 1)
		{
			// text line (left to right)
			bool paddingUp = false;	// need to pad the cells after the text line's '\n' with white-spaces
			for(int i = 0; i < maxCharsPerTextLine; i++)
			{
				x += 8;
				if(*curTextLineChar == '\n' || *curTextLineChar == '\0')
					paddingUp = true;

				if(paddingUp)
					addCell(x, y, ' ');
				else
					addCell(x, y, *curTextLineChar++);
			}
			curTextLineChar++;	// behind '\n'
		}
		else
		{
			// empty line (right to left)
			for(int i = 0; i < maxCharsPerTextLine; i++)
			{
				addCell(x, y, ' ');
				x -= 8;
			}
		}
		y += 8;
	}
}

void AKTextBox::Update(const JoypadState &joypad)
{
	switch(m_state)
	{
	case kMyState_NotRunning:
		// nothing
		break;

	case kMyState_RevealingCells:
		m_numRevealedCellsSoFar++;	// reveal 1 cell every frame

		// revealed all cells now?
		if(m_numRevealedCellsSoFar >= m_cells.size())
		{
			m_numRevealedCellsSoFar = m_cells.size();
			m_sound.Stop();
			m_state = kMyState_AllCellsRevealed;
		}
		break;

	case kMyState_AllCellsRevealed:
		if(joypad.ButtonToggledDown(kJoypadButton_Fire1) || joypad.ButtonToggledDown(kJoypadButton_Fire2))
		{
			Stop();
		}
		break;
	}
}

void AKTextBox::Stop()
{
	m_sound.Stop();
	m_state = kMyState_NotRunning;
}

void AKTextBox::Draw(const Font &font) const
{
	for(size_t i = 0; i < m_numRevealedCellsSoFar; i++)
	{
		char str[2] = { m_cells[i].ch, '\0' };
		font.DrawString2D(str, m_cells[i].x, m_cells[i].y);
	}
}
