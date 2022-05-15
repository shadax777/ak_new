#ifndef __EDFSM_H
#define __EDFSM_H


class EdFSM
{
private:
	struct MyStateInfo;

	struct MyStateTransition
	{
		MyStateInfo *					destination;
		EdStateTransitionCondition *	condition;

		MyStateTransition(MyStateInfo *_dest, EdStateTransitionCondition *_cond) : destination(_dest), condition(_cond) {}
		~MyStateTransition() { delete condition; }
	};

	struct MyStateInfo
	{
		EdState *							state;
		std::vector<MyStateTransition *>	transitions;

		explicit MyStateInfo(EdState *_state) : state(_state) {}
		~MyStateInfo() { delete state; pt_for_each(transitions, pt_delete_and_nullify<MyStateTransition>); }
	};

	typedef std::map<PTString, MyStateInfo *>	MyStateMap;

	MyStateMap		m_registeredStates;
	MyStateInfo *	m_initialState;
	MyStateInfo *	m_currentState;

	// not supported
					EdFSM(const EdFSM &);
	EdFSM &			operator=(const EdFSM &);

public:
					EdFSM();
					~EdFSM();

	void			RegisterState(const char *name, EdState *state);
	void			AddTransition(const char *from, const char *to, EdStateTransitionCondition *condition);
	void			Update(const EdContext &ctx);
	void			Draw(const EdContext &ctx);
};


#endif	// __EDFSM_H
