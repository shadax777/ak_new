#ifndef __EDSTATE_H
#define __EDSTATE_H


class EdState
{
public:
	virtual			~EdState() {}
	virtual void	Enter(const EdContext &ctx) = 0;
	virtual void	Update(const EdContext &ctx) = 0;
	virtual void	Draw(const EdContext &ctx) = 0;
	virtual void	Exit(const EdContext &ctx) = 0;
};


#endif	// __EDSTATE_H
