#ifndef CONFIG_H
#define CONFIG_H

#include <libdl/moby.h>

//--------------------------------------------------------------------------
enum ClimberSpawnType
{
  CLIMBER_SPAWN_DEFAULT = 0,
  CLIMBER_SPAWN_PLATFORM,
  CLIMBER_SPAWN_WITH_PUPDATE,
};

//--------------------------------------------------------------------------
typedef struct ClimberMobyDef
{
	float SpawnInHorizontal;
	float SpawnOutHorizontal;
	float SpawnOutVertical;
	float SpawnInVertical;
	float ObjectScale;

	u32 InstanceUid;
	u16 OClass;
  short PVarSize;
	char SpawnType;
} ClimberMobyDef_t;

//--------------------------------------------------------------------------
extern ClimberMobyDef_t climberMobyDefs[];
extern int climberMobyDefsCount;

#endif // CONFIG_H
