#include "../GameLocal.h"
#pragma hdrstop
#include "EditorLocal.h"


EdFSM::EdFSM()
: m_initialState(NULL),
m_currentState(NULL)
{
}

EdFSM::~EdFSM()
{
	// delete all state maps
	for(MyStateMap::iterator it = m_registeredStates.begin(); it != m_registeredStates.end(); ++it)
	{
		delete it->second;
	}
}

void EdFSM::RegisterState(const char *name, EdState *state)
{
	pt_assert_msg(m_registeredStates.find(name) == m_registeredStates.end(), VABuf("there is already a state called '%s'", name));

	if(m_registeredStates.empty())
	{
		m_initialState = m_registeredStates[name] = new MyStateInfo(state);
	}
	else
	{
		m_registeredStates[name] = new MyStateInfo(state);
	}
}

void EdFSM::AddTransition(const char *from, const char *to, EdStateTransitionCondition *condition)
{
	pt_assert_msg(m_registeredStates.find(from) != m_registeredStates.end(), VABuf("source state '%s' not found", from));
	pt_assert_msg(m_registeredStates.find(to)   != m_registeredStates.end(), VABuf("destination state '%s' not found", to));
	m_registeredStates[from]->transitions.push_back(new MyStateTransition(m_registeredStates[to], condition));
}

void EdFSM::Update(const EdContext &ctx)
{
	if(m_registeredStates.empty())
		return;

	// called for the very first time?
	if(m_currentState == NULL)
	{
		m_currentState = m_initialState;
		m_currentState->state->Enter(ctx);
	}

	m_currentState->state->Update(ctx);

	// check for transition to a different state
	for(size_t i = 0; i < m_currentState->transitions.size(); i++)
	{
		MyStateTransition *trans = m_currentState->transitions[i];
		if(trans->condition->Check(ctx))
		{
			m_currentState->state->Exit(ctx);
			m_currentState = trans->destination;
			m_currentState->state->Enter(ctx);
			break;
		}
	}
}

void EdFSM::Draw(const EdContext &ctx)
{
	if(m_currentState != NULL)
	{
		m_currentState->state->Draw(ctx);
	}
}
