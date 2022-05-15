#ifndef __EDSTATETRANSITIONCONDITION_H
#define __EDSTATETRANSITIONCONDITION_H


class EdStateTransitionCondition
{
public:
	virtual			~EdStateTransitionCondition() {}
	virtual bool	Check(const EdContext &ctx) = 0;
};


class EdStateTransitionCondition_KeyToggled : public EdStateTransitionCondition
{
private:
	PTKeyNum		m_keyNumToCheckForToggle;
	bool			m_checkForToggledDown;	// if true: check for toggled down, if false: check for toggled up

public:
					EdStateTransitionCondition_KeyToggled(PTKeyNum keyNum, bool checkForToggledDown)
						: m_keyNumToCheckForToggle(keyNum),
						m_checkForToggledDown(checkForToggledDown)
					{
					}

	virtual bool	Check(const EdContext &ctx) { return m_checkForToggledDown ? ctx.currentInput->KeyToggledDown(m_keyNumToCheckForToggle) : ctx.currentInput->KeyToggledUp(m_keyNumToCheckForToggle); }
};


#endif	// __EDSTATETRANSITIONCONDITION_H
