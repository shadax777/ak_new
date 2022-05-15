#include "../GameLocal.h"
#pragma hdrstop
#include "AKLocal.h"


AKEntityFactoryBase *AKEntityFactoryBase::s_list;

AKEntityFactoryBase::AKEntityFactoryBase(const char *entityClassName, bool removeEntityIfClipped, SpawnType spawnType)
: m_entityClassName(entityClassName),
m_spawnType(spawnType),
m_next(s_list),
m_removeEntityIfClipped(removeEntityIfClipped)
{
	s_list = this;
}

AKEntityFactoryBase *AKEntityFactoryBase::FindFactoryByEntityClassName(const char *entityClassName)
{
	for(AKEntityFactoryBase *cur = s_list; cur != NULL; cur = cur->m_next)
	{
		if(strcmp(entityClassName, cur->m_entityClassName) == 0)
			return cur;
	}
	return NULL;
}
