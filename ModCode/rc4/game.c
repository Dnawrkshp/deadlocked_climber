#include <libdl/stdio.h>
#include <libdl/random.h>
#include <libdl/net.h>
#include <libdl/spawnpoint.h>
#include <libdl/area.h>
#include <libdl/color.h>
#include <libdl/collision.h>
#include <libdl/time.h>
#include <libdl/utils.h>
#include "game.h"
#include "climber.h"
#include "config.h"
#include "cgm.h"
#include "cgm_score.h"
#include "cgm_netmsg.h"

//--------------------------------------------------------------------------
struct GameState State = {};

//--------------------------------------------------------------------------
int climberResolveCParamPointsToWin(void)
{
  int paramValue = cgmGetParameter(CPARAM_ID_POINTS_TO_WIN);
  switch (paramValue)
  {
    case CPARAM_POINTS_TO_WIN_250: return 250;
    case CPARAM_POINTS_TO_WIN_500: return 500;
    case CPARAM_POINTS_TO_WIN_750: return 750;
    case CPARAM_POINTS_TO_WIN_1000: return 1000;
    case CPARAM_POINTS_TO_WIN_1250: return 1250;
    case CPARAM_POINTS_TO_WIN_1500: return 1500;
    default: return 500;
  }
}

//--------------------------------------------------------------------------
float climberResolveWaterSpeed(void)
{
  int paramValue = cgmGetParameter(CPARAM_ID_WATER_SPEED);
  switch (paramValue)
  {
    default: return 1.1;
    case CPARAM_WATER_SPEED_NORMAL: return 1.1;
    case CPARAM_WATER_SPEED_FAST: return 1.15;
  }
}

//--------------------------------------------------------------------------
float climberResolveMaxWaterSpeed(void)
{
  int paramValue = cgmGetParameter(CPARAM_ID_WATER_SPEED);
  switch (paramValue)
  {
    default: return 1.2 * MATH_DT;
    case CPARAM_WATER_SPEED_NORMAL: return 1.2 * MATH_DT;
    case CPARAM_WATER_SPEED_FAST: return 2.0 * MATH_DT;
  }
}

//--------------------------------------------------------------------------
void modDraw(void)
{
  
}

//--------------------------------------------------------------------------
void modUpdate(void)
{
  climberUpdate();
}

//--------------------------------------------------------------------------
void modCleanup(void)
{
  
}

//--------------------------------------------------------------------------
void modStart(void)
{
}

//--------------------------------------------------------------------------
void modInit(void)
{
  // set target points
	cgmScoreTarget.CustomTarget = climberResolveCParamPointsToWin() * 1024;
  gameScoreboardSetScoreMax(climberResolveCParamPointsToWin());

  // find water moby
  State.WaterMoby = mobyFindNextByOClass(mobyListGetStart(), MOBY_ID_WATER);

  //
  GameSettings* gameSettings = gameGetSettings();
  State.RandSeed = gameSettings->GameLoadStartTime;
}

//--------------------------------------------------------------------------
void modLoad(void)
{
}
