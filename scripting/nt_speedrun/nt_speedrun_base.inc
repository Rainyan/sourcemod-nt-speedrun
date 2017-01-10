#if defined GUARD_SPEEDRUN_BASE
  #endinput
#endif
#define GUARD_SPEEDRUN_BASE

#define PLUGIN_VERSION "0.1"

#define TEAM_ENUM_COUNT 4
#define HALO_LIFE 2.0
#define FIRST_GOAL TEAM_NSF

enum {
  POS_X = 0,
  POS_Y,
  POS_Z,
  POS_ENUM_COUNT
};

enum {
  PROP_POSITION = 0,
  PROP_RADIUS,
  PROP_ENUM_COUNT
};

enum {
  MENU_MAIN_TELEPORT = 1,
  MENU_MAIN_EXIT
};

enum {
  RUNMSG_PB = 0,
  RUNMSG_FINISHED,
  RUNMSG_CHECKPOINT
};

new const String:g_sSound_Start[] = "ui/buttonclickrelease.wav";
new const String:g_sSound_MenuCancel[] = "buttons/combine_button7.wav";
new const String:g_sSound_MenuOK[] = "buttons/button14.wav";

new const String:g_sFX_Laser[] = "sprites/bluelaser1.vmt";
new const String:g_sFX_Halo[] = "sprites/blueglow1.vmt";

new const String:g_sRunEndMsg[][] = {
  "[SM] Improved personal record!",
  "[SM] Run finished.",
  "[SM] Checkpoint."
};

float g_fCapzone[TEAM_ENUM_COUNT][PROP_ENUM_COUNT][POS_ENUM_COUNT];
float g_fRunTime_PB[MAXPLAYERS+1][TEAM_ENUM_COUNT];

float g_fRunTime_Start[MAXPLAYERS+1];
float g_fRunTime_Current[MAXPLAYERS+1];

int g_iLaserRef;
int g_iHaloRef;
int g_iHaloFrame;

int g_iGoal[MAXPLAYERS+1];
int g_iHaloColor[4] = { 0, 0, 255, 255 };

Handle g_hTimer_Speedrun = null;
Handle g_hTimer_TempHalo = null;

void ClearClientVariables(int client)
{
  PrintToServer("ClearClientVariables");

  g_iGoal[client] = TEAM_NONE;
  g_fRunTime_Start[client] = 0.0;
  g_fRunTime_Current[client] = 0.0;
  g_fRunTime_PB[client][TEAM_JINRAI] = 0.0;
  g_fRunTime_PB[client][TEAM_NSF] = 0.0;
}

void TeleportPlayer(int client)
{
  if (!IsValidTeam(g_iGoal[client]))
  {
    g_iGoal[client] = TEAM_JINRAI;
  }

  g_fRunTime_Start[client] = 0.0;
  SetEntPropVector(client, Prop_Send, "m_vecOrigin", g_fCapzone[g_iGoal[client]][PROP_POSITION]);
  // TODO: player angles, client entprops for rotation seem weird?

  //TeleportEntity(client,
  //  g_fCapzone[g_iGoal[client]][PROP_POSITION], NULL_VECTOR, zeroSpeed);
}

bool IsNullVector(float[] vector)
{
  if (vector[POS_X] != 0 || vector[POS_Y] != 0 || vector[POS_Z] != 0)
  {
    return false;
  }
  return true;
}

void StartSpeedrunMode()
{
  if (g_hTimer_TempHalo != null)
  {
    delete g_hTimer_TempHalo;
    g_hTimer_TempHalo = null;
  }
  if (g_hTimer_Speedrun != null)
  {
    delete g_hTimer_Speedrun;
    g_hTimer_Speedrun = null;
  }

  // Triple layered halo
  g_hTimer_TempHalo = CreateTimer(HALO_LIFE, Timer_TempHalo, _, TIMER_REPEAT);

  g_hTimer_Speedrun = CreateTimer(0.0, Timer_Speedrun, _, TIMER_REPEAT);
}

void CheckRunnerGoal(int client)
{
  // TODO: Is this necessary?
  /*
  if (!IsValidClient(client) || IsFakeClient(client))
    return;
  */

  // Client has no speedrun goal set yet
  if (g_iGoal[client] == TEAM_NONE)
  {
    SetClientGoal(client, FIRST_GOAL);
    return;
  }

  int reachedGoal = GetReachedGoal(client);

  if (reachedGoal == g_iGoal[client])
  {
    SplitTime(client);
  }
  else if (reachedGoal == GetOppositeTeam(g_iGoal[client]))
  {
    if (g_fRunTime_Start[client] != 0)
    {
      ResetTime(client);
    }
  }
  else if (g_fRunTime_Start[client] == 0)
  {
    StartTime(client);
  }
}

void ResetTime(int client)
{
  g_fRunTime_Start[client] = 0.0;
  PrintToChat(client, "[SM] Timer reset");
}

void StartTime(int client)
{
  g_fRunTime_Start[client] = GetGameTime();
  EmitSoundToClient(client, g_sSound_Start);
  PrintToChat(client, "[SM] Run started");
}

void FinishTime(int client)
{
  float finishTime = GetGameTime();
  g_fRunTime_Current[client] = finishTime - g_fRunTime_Start[client];
  EmitSoundToClient(client, g_sSound_Start);
}

void SplitTime(int client)
{
  FinishTime(client);
  SetPlayerAUX(client, 100.0);

  int goal = g_iGoal[client];
  if (!IsValidTeam(goal))
  {
    ThrowError("Client %i has invalid goal %i", client, goal);
  }

  float record = g_fRunTime_PB[client][goal];
  float current = g_fRunTime_Current[client];

  // Midway checkpoint
  if (goal == FIRST_GOAL)
  {
    // First time running this
    if (record == 0)
    {
      //PrintToChat(client, "1st, record %f current %f", record, current);
      PrintToChat(client, "%s %.2f", g_sRunEndMsg[RUNMSG_CHECKPOINT], current);
      g_fRunTime_PB[client][goal] = current;
    }
    // Log time, show difference to personal best time
    else
    {
      //PrintToChat(client, "2nd, record %f current %f", record, current);
      PrintToChat(client, "%s %.2f (PB %.2f)",
        g_sRunEndMsg[RUNMSG_CHECKPOINT], current, current - record);
    }
  }
  // Finish
  else
  {
    // First time running this
    if (record == 0)
    {
      //PrintToChat(client, "1st, record %f current %f", record, current);
      PrintToChat(client, "%s %.2f", g_sRunEndMsg[RUNMSG_FINISHED], current);
      g_fRunTime_PB[client][goal] = current;
    }
    // New personal best
    else if (current < record)
    {
      //PrintToChat(client, "2nd, record %f current %f", record, current);
      PrintToChat(client, "%s %.2f (PB %.2f)",
        g_sRunEndMsg[RUNMSG_PB], current, current - record);

      g_fRunTime_PB[client][goal] = current;
    }
    // No record time
    else
    {
      //PrintToChat(client, "3rd, record %f current %f", record, current);
      PrintToChat(client, "%s %.2f (PB +%.2f)",
        g_sRunEndMsg[RUNMSG_FINISHED], current, current - record);
    }
  }

  FlipClientGoal(client);
  //PrintToChatAll("New goal is to %i", g_iGoal[client]);
}

void SetClientGoal(int client, int desiredGoal = TEAM_NONE)
{
  if (desiredGoal != TEAM_NONE)
  {
    g_iGoal[client] = desiredGoal;
    return;
  }

  int reachedGoal = GetReachedGoal(client);
  // Client reached goal, flip goal
  if (reachedGoal != TEAM_NONE)
  {
    FlipClientGoal(client);
  }
}

void FlipClientGoal(int client)
{
  if (!IsValidTeam(g_iGoal[client]))
  {
    ThrowError("Client %i has an invalid goal %i", client, g_iGoal[client]);
  }
  g_iGoal[client] = GetOppositeTeam(g_iGoal[client]);
}

bool IsValidTeam(int team)
{
  if (team != TEAM_JINRAI && team != TEAM_NSF)
  {
    return false;
  }
  return true;
}

int GetOppositeTeam(int team)
{
  if (!IsValidTeam(team))
  {
    ThrowError("Invalid team %i", team);
  }
  if (team == TEAM_JINRAI)
  {
    return TEAM_NSF;
  }

  return TEAM_JINRAI;
}

bool IsPosWithinRadius(float[3] pos, float[3] targetPos, float radius)
{
  for (int i = 0; i < POS_ENUM_COUNT; i++)
  {
    //PrintToChatAll("pos %f vs target %f. radius %f.", pos[i], targetPos[i], radius);
    if (pos[i] > targetPos[i] + radius || pos[i] < targetPos[i] - radius)
      return false;
  }
  //PrintToChatAll("radius TRUE");
  return true;
}

int GetReachedGoal(int client)
{
  float clientPos[POS_ENUM_COUNT];
  GetClientEyePosition(client, clientPos);

  for (int team = TEAM_JINRAI; team <= TEAM_NSF; team++)
  {
    int pos;
    for (pos = 0; pos < POS_ENUM_COUNT; pos++)
    {
      if (!IsPosWithinRadius(clientPos, g_fCapzone[team][PROP_POSITION],
        g_fCapzone[team][PROP_RADIUS][0]))
      {
        break;
      }
    }
    // pos loop passed all the way; we are inside the capzone radius
    if (pos == POS_ENUM_COUNT)
    {
      return team;
    }
  }
  return TEAM_NONE;
}

void Capzones_Clear()
{
  for (int i = 0; i < sizeof(g_fCapzone); i++)
  {
    for (int j = 0; j < 3; j++)
    {
      g_fCapzone[i][PROP_POSITION][j] = 0.0;
    }
  }
}
