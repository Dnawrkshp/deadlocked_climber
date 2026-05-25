#include <libdl/moby.h>
#include <libdl/dialog.h>
#include <libdl/utils.h>
#include <libdl/string.h>
#include <libdl/stdio.h>
#include <libdl/random.h>
#include <libdl/spawnpoint.h>
#include "game.h"
#include "config.h"
#include "climber.h"
#include "platform.h"
#include "cgm_score.h"
#include "cgm_netmsg.h"
#include "cgm_rounds.h"

//--------------------------------------------------------------------------
int climberBranchDialogs[] = { DIALOG_ID_CLANK_YOU_HAVE_A_CHOICE_OF_2_PATHS, DIALOG_ID_DALLAS_WOAH_THIS_IS_GETTING_INTERESTING, DIALOG_ID_DALLAS_KICKING_PROVERBIAL_BUTT_IDK_WHAT_THAT_MEANS, DIALOG_ID_DALLAS_DARKSTAR_TIGHT_SPOTS_BEFORE  };
int climberStartDialogs[] = { DIALOG_ID_TEAM_DEADSTAR, DIALOG_ID_DALLAS_SHOWTIME, DIALOG_ID_DALLAS_RATCHET_LAST_WILL_AND_TESTAMENT, DIALOG_ID_DALLAS_WHO_PACKED_YOUR_PARACHUTE, };
int climberIncrementalDialogs[] = { DIALOG_ID_PLEASE_TAKE_YOUR_TIME, DIALOG_ID_DALLAS_SHOWOFF, DIALOG_ID_JUANITA_MORON, DIALOG_ID_JUANITA_I_CANT_BEAR_TO_LOOK_YES_I_CAN, DIALOG_ID_DALLAS_CARNAGE_LAST_RELATIONSHIP, DIALOG_ID_DALLAS_LOOK_AT_THAT_LITTLE_GUY_GO,  };

//--------------------------------------------------------------------------
int climberRand(int mod)
{
  State.RandSeed = (1664525 * State.RandSeed + 1013904223);
  return State.RandSeed % mod;
}

//--------------------------------------------------------------------------
void climberSeed(int seed)
{
  State.RandSeed = seed;
}

//--------------------------------------------------------------------------
float climberRandRange(float min, float max)
{
	int v = climberRand(0xFFFF);
	float value = ((float)v / 0xFFFF);

	return (value + min) * (max - min);
}

//--------------------------------------------------------------------------
short climberRandRangeShort(short min, short max)
{
	int v = climberRand(0xFFFF);
	return (v % (max - min)) + min;
}

//--------------------------------------------------------------------------
void climberPlayRandomDialog(int* options, int count)
{
  if (count <= 0)
    return;

  dialogPlaySound(options[climberRand(count)], 0);
}

//--------------------------------------------------------------------------
void climberPlayRandomStartDialog(void)
{
  climberPlayRandomDialog(climberStartDialogs, COUNT_OF(climberStartDialogs));
}

//--------------------------------------------------------------------------
void climberPlayRandomBranchDialog(void)
{
  climberPlayRandomDialog(climberBranchDialogs, COUNT_OF(climberBranchDialogs));
}

//--------------------------------------------------------------------------
void climberPlayRandomIncrementalDialog(void)
{
  climberPlayRandomDialog(climberIncrementalDialogs, COUNT_OF(climberIncrementalDialogs));
}

//--------------------------------------------------------------------------
void climberDestroySubmergedClimberMobys(float waterHeight)
{
	Moby * moby = mobyListGetStart();
	Moby * mEnd = mobyListGetEnd();

	while (moby < mEnd)
	{
		if (!mobyIsDestroyed(moby) && moby->Bolts == 0x123)
		{
      // special case when platform is not in default state ignore
      // in case falling platform went underneath water, we want it to go back up
      if (!platformMobyIsPlatform(moby) || moby->State == 0)
      {
        if (moby->Position[2] < waterHeight)
        {
          mobyDestroy(moby);
          moby->Bolts = 0;
          --State.MobyCount;
        }
      }
		}

		++moby;
	}
}

//--------------------------------------------------------------------------
void climberInitChain(ClimberChain_t* chain, VECTOR position, float theta)
{
  memset(chain, 0, sizeof(ClimberChain_t));
  vector_copy(chain->CurrentPosition, position);
  chain->LastBranch = gameGetTime();
  chain->LastTheta = theta;
  chain->Active = 1;
  chain->NextItem = -1;
}

//--------------------------------------------------------------------------
ClimberChain_t* climberGetFreeChain(void)
{
	int i = 0;
	for (; i < MAX_CHAINS; ++i)
		if (!State.Chains[i].Active)
			return &State.Chains[i];

	return NULL;
}

//--------------------------------------------------------------------------
void climberSetWaterHeight(float waterHeight)
{
	((float*)State.WaterMoby->PVar)[19] = waterHeight;
	gameSetDeathHeight(waterHeight);
  climberDestroySubmergedClimberMobys(waterHeight);
}

//--------------------------------------------------------------------------
Moby* climberSpawnMoby(ClimberMobyDef_t* def, int gameTime)
{
  switch (def->SpawnType)
  {
    case CLIMBER_SPAWN_DEFAULT: break;
    case CLIMBER_SPAWN_PLATFORM:
    {
      Moby* refTarget = mobyFindByUID(def->InstanceUid);
      if (!refTarget)
        break;

      Moby* spawnedMoby = mobySpawn(def->OClass, def->PVarSize);
      memcpy(spawnedMoby->PVar, refTarget->PVar, def->PVarSize);
      spawnedMoby->UpdateDist = 0xFF;
      spawnedMoby->PUpdate = refTarget->PUpdate;

      if (platformMobyIsPlatform(spawnedMoby))
        platformInitMoby(spawnedMoby, gameTime);
      return spawnedMoby;
    }
    case CLIMBER_SPAWN_WITH_PUPDATE:
    {
      Moby* refTarget = mobyFindByUID(def->InstanceUid);
      if (!refTarget)
        break;

      Moby* spawnedMoby = mobySpawn(def->OClass, def->PVarSize);
      memcpy(spawnedMoby->PVar, refTarget->PVar, def->PVarSize);
      spawnedMoby->UpdateDist = 0xFF;
      spawnedMoby->PUpdate = refTarget->PUpdate;
      return spawnedMoby;
    }
  }

  return mobySpawn(def->OClass, 0);
}

//--------------------------------------------------------------------------
Moby* climberSpawn(ClimberMobyDef_t* def, VECTOR position, VECTOR rotation, float scale, int gameTime)
{
  Moby* moby = climberSpawnMoby(def, gameTime);
  if (!moby)
    return NULL;

  DPRINTF("climberSpawn(%d, (%.1f, %.1f, %.1f), %f) => %08X\n", def->OClass, position[0], position[1], position[2], scale, (u32)moby);

	vector_copy(moby->Position, position);
	vector_copy(moby->Rotation, rotation);

	// moby->UpdateDist = 0xFF;
	moby->DrawDist = 0x0080;
  moby->Bolts = 0x123;
	moby->ModeBits = MOBY_MODE_BIT_HAS_GLOW | MOBY_MODE_BIT_UNK_40;
	moby->GlowRGBA = 0x808C8C8C;
  if (def->SpawnType == CLIMBER_SPAWN_DEFAULT)
    moby->PUpdate = NULL;
 	moby->Scale *= scale * def->ObjectScale;
  mobyUpdateTransform(moby);

  ++State.MobyCount;
	return moby;
}

//--------------------------------------------------------------------------
void climberUpdateChainForNextSpawn(ClimberChain_t* chain, ClimberMobyDef_t* currentItem, float scale)
{
	// determine next spawn
	chain->NextItem = climberRandRangeShort(0, climberMobyDefsCount);
	ClimberMobyDef_t* nextItem = &climberMobyDefs[chain->NextItem];
  float verticalOffset = 0;
  float horizontalOffset = 0;

	// adjust scale by current item
	if (currentItem)
  {
		horizontalOffset = (currentItem->SpawnOutHorizontal + nextItem->SpawnInHorizontal) / 2;
    verticalOffset = nextItem->SpawnInVertical + currentItem->SpawnOutVertical;
  }

	// generate next position
	chain->LastTheta = clampAngle(climberRandRange(chain->LastTheta - (MATH_PI/4), chain->LastTheta + (MATH_PI/4)));
	chain->CurrentPosition[0] += scale * horizontalOffset * cosf(chain->LastTheta);
	chain->CurrentPosition[1] += scale * horizontalOffset * sinf(chain->LastTheta);
	chain->CurrentPosition[2] += verticalOffset * climberRandRange(1.5, 3);
}

//--------------------------------------------------------------------------
void climberRunSpawnPass(int gameTime)
{
  VECTOR rot;
  int chainIndex;
	for (chainIndex = 0; chainIndex < MAX_CHAINS; ++chainIndex)
	{
		ClimberChain_t* chain = &State.Chains[chainIndex];
		if (!chain->Active)
			continue;

    // stop
    if (chain->CurrentPosition[2] >= MAX_WATER_HEIGHT)
      continue;

    // ensure we have a valid item
    if (chain->NextItem < 0 || chain->NextItem >= climberMobyDefsCount)
      chain->NextItem = climberRandRangeShort(0, climberMobyDefsCount);

		// Generate new random parameters
		float scale = climberRandRange(1, 2);
		rot[0] = climberRandRange(-0.3, 0.3);
		rot[1] = climberRandRange(-0.3, 0.3);
		rot[2] = climberRandRange(-1, 1);
    rot[3] = 0;
		ClimberMobyDef_t* currentItem = &climberMobyDefs[chain->NextItem];

    // spawn
		climberSpawn(currentItem, chain->CurrentPosition, rot, scale, gameTime);

		// branch
		if ((gameTime - chain->LastBranch) > BRANCH_PERIOD)
		{
			ClimberChain_t* branchChain = climberGetFreeChain();
			if (branchChain)
			{
        climberInitChain(branchChain, chain->CurrentPosition, clampAngle(MATH_PI + chain->LastTheta));
				branchChain->LastBranch = gameTime;

				// Determine next spawn
				climberUpdateChainForNextSpawn(branchChain, currentItem, scale);
        climberPlayRandomBranchDialog();
			}

			chain->LastBranch = gameTime;
		}

		// determine next spawn
		climberUpdateChainForNextSpawn(chain, currentItem, scale);
	}
}

//--------------------------------------------------------------------------
void climberOnSpawnClimberMobyMessageReceived(int fromClientId, struct SpawnClimberMobyMessage *msg)
{
  // force seed
  climberSeed(msg->Seed);

	// update last spawn time
	State.LastSpawnTime = msg->GameTime;

  // spawn climber moby
  climberRunSpawnPass(msg->GameTime);

  // play periodic dialog sound
	if (State.MobyCount == 0)
		climberPlayRandomStartDialog();
	else if (State.MobyCount % 60 == 0)
		climberPlayRandomIncrementalDialog();

  // Update spawn rates
	if (State.SpawnRate > MAX_SPAWN_RATE)
		State.SpawnRate -= State.MobyCount * 6.5;
	else
		State.SpawnRate = MAX_SPAWN_RATE;

  State.WaterRaiseRate = clamp(State.WaterRaiseRate * climberResolveWaterSpeed(), MIN_WATER_RATE, climberResolveMaxWaterSpeed());
  // printf("%.1f %f\n", cgmRoundsGetRoundElapsedTime() / 1000.0, State.WaterRaiseRate);
}

//--------------------------------------------------------------------------
void climberProcessPlayer(int playerIdx)
{
  Player* player = playerGetFromIndex(playerIdx);
  if (!playerIsValid(player))
    return;

  if (playerIsDead(player))
    return;

  float currentScore = player->PlayerPosition[2];

  // update stats
  float bestScore = maxf(currentScore, cgmScoreGetCustomPlayerFloatStat(playerIdx, CSTAT_DISTANCE));
  cgmScoreSetCustomPlayerFloatStat(playerIdx, CSTAT_DISTANCE, bestScore, 0);
  float bestTotalScore = maxf(currentScore, cgmScoreGetCustomPlayerFloatStat(playerIdx, CSTAT_BEST_DISTANCE));
  cgmScoreSetCustomPlayerFloatStat(playerIdx, CSTAT_BEST_DISTANCE, bestTotalScore, 0);

  // kill if beneath water
  if (player->PlayerPosition[2] <= State.WaterHeight)
    playerSetHealth(player, 0);
}

//--------------------------------------------------------------------------
void climberResetRound(void)
{
  VECTOR startPos = { 500, 500, 12 };

  // destroy all climber mobys
  climberDestroySubmergedClimberMobys(MAX_WATER_HEIGHT);

  // reset chains
  memset(State.Chains, 0, sizeof(State.Chains));
  climberInitChain(&State.Chains[0], startPos, 0);

  // reset necessary state
  State.LastSpawnTime = gameGetTime();
  State.SpawnRate = INIT_SPAWN_RATE;
  State.MobyCount = 0;
  State.WaterRaiseRate = MIN_WATER_RATE;
  State.WaterHeight = 0;
  climberSetWaterHeight(0);
}

//--------------------------------------------------------------------------
void climberUpdate(void)
{
  // static int init = 0;
  // if (!init)
  // {
  //   init = 1;
  //   Moby* m = climberSpawn(&climberMobyDefs[0], (VECTOR){500, 500, 12, 0}, (VECTOR){0,0,0,0}, 1, gameGetTime());
  //   printf("spawned moby %08X\n", (u32)m);
  // }
  // return;

  // handle spawning
  int gameTime = gameGetTime();
	if ((gameTime - State.LastSpawnTime) > State.SpawnRate)
	{
    struct SpawnClimberMobyMessage msg = { .GameTime = gameTime, .Seed = rand(0x7fffffff) };
    cgmNetMsgBroadcast_SpawnClimberMoby(&msg);
	}

  // update water height
	State.WaterHeight += State.WaterRaiseRate;
  climberSetWaterHeight(State.WaterHeight);
  
  // update players
  int i;
  for (i = 0; i < GAME_MAX_PLAYERS; ++i)
    climberProcessPlayer(i);
}
