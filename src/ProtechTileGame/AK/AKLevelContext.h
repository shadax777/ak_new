#ifndef __AKLEVELCONTEXT_H
#define __AKLEVELCONTEXT_H


struct AKLevelContext
{
	const GameContext *			gameContext;
	LevelData *					levelData;
	AKCamera *					camera;
	GameRenderWorld2D *			gameRenderWorld;
	Color *						backgroundColor;
	AKWorld *					world;
	AKLevelBlackboard *			blackboard;
	AKTextBox *					textBox;
	AKShopManager *				shopManager;
	AKJankenGameBlackboard *	jankenGameBlackboard;

	AKLevelContext() { static AKLevelContext defaulted; *this = defaulted; }
};


#endif	// __AKLEVELCONTEXT_H
