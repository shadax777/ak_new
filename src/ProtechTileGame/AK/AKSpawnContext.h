#ifndef __AKSPAWNCONTEXT_H
#define __AKSPAWNCONTEXT_H


struct AKSpawnContext
{
	const AKLevelContext &	levelContext;
	const Dict &			spawnArgs;
	bool					removeIfClipped;

	AKSpawnContext(const AKLevelContext &_levelContext, const Dict &_spawnArgs, bool _removeIfClipped)
		: levelContext(_levelContext),
		spawnArgs(_spawnArgs),
		removeIfClipped(_removeIfClipped)
	{
		// nothing
	}
};


#endif	// __AKSPAWNCONTEXT_H
