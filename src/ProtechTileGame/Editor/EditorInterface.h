#ifndef __EDITORINTERFACE_H
#define __EDITORINTERFACE_H

// interface of the editor used by the game

struct LevelData;

class EditorInterface
{
public:
	virtual void				Init(const GameContext &gameContext) = 0;
	virtual void				Shutdown() = 0;
	virtual void				RunFrame(const GameInput &gameInput) = 0;
	virtual void				Draw() = 0;
	virtual void				OnEditorActivated(const Vec2 &suggestedCameraPos) = 0;
	static EditorInterface &	GetImplementation();
};


#endif	// __EDITORINTERFACE_H
