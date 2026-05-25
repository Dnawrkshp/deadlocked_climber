#ifndef GAME_H
#define GAME_H

#include <libdl/game.h>
#include <libdl/player.h>
#include <libdl/time.h>

//--------------------------------------------------------------------------
#define MAX_CHAINS          (2)
#define MIN_WATER_RATE			(0.1 * MATH_DT)
#define INIT_SPAWN_RATE			(TIME_SECOND * 3.0)
#define MAX_SPAWN_RATE			(TIME_SECOND * 1.1)
#define MAX_WATER_HEIGHT    (1000)
#define BRANCH_PERIOD       (30 * TIME_SECOND)

//--------------------------------------------------------------------------
enum GameCustomStatIds
{
  CSTAT_DISTANCE,
  CSTAT_BEST_DISTANCE,
};

//--------------------------------------------------------------------------
enum GameCustomParameterIds
{
  CPARAM_ID_POINTS_TO_WIN,
  CPARAM_ID_WATER_SPEED,
};

//--------------------------------------------------------------------------
enum GameCustomParameterPointsToWinValues
{
  CPARAM_POINTS_TO_WIN_500,
  CPARAM_POINTS_TO_WIN_750,
  CPARAM_POINTS_TO_WIN_1000,
  CPARAM_POINTS_TO_WIN_1250,
  CPARAM_POINTS_TO_WIN_1500,
  CPARAM_POINTS_TO_WIN_250,
  CPARAM_POINTS_TO_WIN_COUNT
};

//--------------------------------------------------------------------------
enum GameCustomParameterWaterSpeedValues
{
  CPARAM_WATER_SPEED_NORMAL,
  CPARAM_WATER_SPEED_FAST,
  CPARAM_SPLEEF_LAYERS_COUNT
};

//--------------------------------------------------------------------------
typedef struct ClimberChain
{
	int Active;
	int NextItem;
	float LastTheta;
	int LastBranch;
	VECTOR CurrentPosition;
} ClimberChain_t;

//--------------------------------------------------------------------------
struct GameState
{
  u32 RandSeed;
  int MobyCount;
  Moby* WaterMoby;
  float WaterRaiseRate;
  float WaterHeight;
  int SpawnRate;
  int LastSpawnTime;
  ClimberChain_t Chains[MAX_CHAINS];
};

//--------------------------------------------------------------------------
extern struct GameState State;

//--------------------------------------------------------------------------
int climberResolveCParamPointsToWin(void);
float climberResolveWaterSpeed(void);
float climberResolveMaxWaterSpeed(void);

#endif // GAME_H
