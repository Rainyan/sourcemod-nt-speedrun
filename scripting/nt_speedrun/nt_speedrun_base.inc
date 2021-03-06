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
  RUNMSG_SPLIT = 0,
  RUNMSG_SPLIT_PB,
  RUNMSG_GOAL,
  RUNMSG_GOAL_PB,
  RUNMSG_COMBINED,
  RUNMSG_COMBINED_PB
};

new const String:g_sSound_Start[] = "ui/buttonclickrelease.wav";
new const String:g_sSound_MenuCancel[] = "buttons/combine_button7.wav";
new const String:g_sSound_MenuOK[] = "buttons/button14.wav";

new const String:g_sFX_Laser[] = "sprites/bluelaser1.vmt";
new const String:g_sFX_Halo[] = "sprites/blueglow1.vmt";

new const String:g_sRunEndMsg[][] = {
  "[RUN] Split 1/2",
  "[RECORD] Split 1/2",
  "[RUN] Split 2/2",
  "[RECORD] Split 2/2",
  "[RUN] Goal",
  "[RECORD] Goal"
};

bool g_bIsTeleporting[MAXPLAYERS+1];

float g_fCapzone[TEAM_ENUM_COUNT][PROP_ENUM_COUNT][POS_ENUM_COUNT];
float g_fRunTime_Split_PB[MAXPLAYERS+1];
float g_fRunTime_Goal_PB[MAXPLAYERS+1];
float g_fRunTime_Combined_PB[MAXPLAYERS+1];

float g_fRunTime_Split_Start[MAXPLAYERS+1];
float g_fRunTime_Split_End[MAXPLAYERS+1];

float g_fRunTime_Goal_Start[MAXPLAYERS+1];
float g_fRunTime_Goal_End[MAXPLAYERS+1];


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

  g_bIsTeleporting[client] = false;

  g_iGoal[client] = TEAM_NONE;

  g_fRunTime_Split_Start[client] = 0.0;
  g_fRunTime_Split_End[client] = 0.0;
  g_fRunTime_Goal_Start[client] = 0.0;
  g_fRunTime_Goal_End[client] = 0.0;

  g_fRunTime_Split_PB[client] = 0.0;
  g_fRunTime_Goal_PB[client] = 0.0;
  g_fRunTime_Combined_PB[client] = 0.0;
}

void TeleportPlayer(int client)
{
  if (!IsValidClient(client))
  {
    return;
  }

  PrintToChat(client, "[RUN] Teleporting...");
  g_bIsTeleporting[client] = true;

  // Block player movement as it interferes with player ticks while teleporting
  int buttons = GetEntityFlags(client);
  if(!(buttons & FL_FROZEN))
  {
    SetEntityFlags(client, (buttons |= FL_FROZEN));
  }

  CreateTimer(2.5, Timer_Teleport, GetClientUserId(client));
}

public Action Timer_Teleport(Handle timer, int userid)
{
  int client = GetClientOfUserId(userid);
  if (!IsValidClient(client))
  {
    return Plugin_Stop;
  }

  g_iGoal[client] = FIRST_GOAL;

  float location[3];
  location = g_fCapzone[GetOppositeTeam(FIRST_GOAL)][PROP_POSITION];
  // Increase height to avoid players stucking to the ground
  location[POS_Z] += 32;

  SetEntPropVector(client, Prop_Send, "m_vecOrigin", location);

  // Teleporting complete, allow player to move again
  CreateTimer(0.1, Timer_AllowMovement, userid);
  return Plugin_Handled;
}

public Action Timer_AllowMovement(Handle timer, int userid)
{
  int client = GetClientOfUserId(userid);
  if (!IsValidClient(client))
  {
    return Plugin_Stop;
  }

  g_bIsTeleporting[client] = false;
  g_fRunTime_Split_Start[client] = 0.0;

  int buttons = GetEntityFlags(client);
  if (buttons & FL_FROZEN)
  {
    SetEntityFlags(client, (buttons &= ~FL_FROZEN));
  }
  return Plugin_Handled;
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
  if (g_bIsTeleporting[client])
  {
    return;
  }
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
    if (g_fRunTime_Split_Start[client] != 0)
    {
      ResetTime(client);
    }
  }
  else if (
    (g_iGoal[client] == FIRST_GOAL && g_fRunTime_Split_Start[client] == 0) ||
    (g_iGoal[client] == GetOppositeTeam(FIRST_GOAL) && g_fRunTime_Goal_Start[client] == 0)
  )
  {
    StartTime(client);
  }
}

void ResetTime(int client)
{
  if (g_iGoal[client] == FIRST_GOAL && g_fRunTime_Split_Start[client] != 0)
  {
    g_fRunTime_Split_Start[client] = 0.0;
    PrintToChat(client, "[RUN] Timer stopped");
  }
  else if (g_fRunTime_Goal_Start[client] != 0)
  {
    g_fRunTime_Goal_Start[client] = 0.0;
    PrintToChat(client, "[RUN] Timer paused");
  }
}

void StartTime(int client)
{
  if (g_iGoal[client] == FIRST_GOAL)
  {
    g_fRunTime_Split_Start[client] = GetGameTime();
    PrintToChat(client, "[RUN] Run started");
  }
  else
  {
    g_fRunTime_Goal_Start[client] = GetGameTime();
    PrintToChat(client, "[RUN] Run continued");
  }
  EmitSoundToClient(client, g_sSound_Start);
}

void SplitTime(int client)
{
  float gametime = GetGameTime();

  int goal = g_iGoal[client];
  if (!IsValidTeam(goal))
  {
    ThrowError("Client %i has invalid goal %i", client, goal);
  }

  EmitSoundToClient(client, g_sSound_Start);
  SetPlayerAUX(client, 100.0);

  float splitPB = g_fRunTime_Split_PB[client];
  float goalPB = g_fRunTime_Goal_PB[client];
  float combinedPB = g_fRunTime_Combined_PB[client];

  // Midway split
  if (goal == FIRST_GOAL)
  {
    g_fRunTime_Split_End[client] = gametime - g_fRunTime_Split_Start[client];
    /*PrintToChat(client, "Split %f from %f - %f",
      g_fRunTime_Split_End[client], gametime, g_fRunTime_Split_Start[client]);*/
    // First time running this
    if (splitPB == 0)
    {
      PrintToChat(client, "%s = %.2f", g_sRunEndMsg[RUNMSG_SPLIT],
        g_fRunTime_Split_End[client]);

      g_fRunTime_Split_PB[client] = g_fRunTime_Split_End[client];
    }
    // New personal best
    else if (g_fRunTime_Split_End[client] < splitPB)
    {
      PrintToChat(client, "%s = %.2f (PB %.2f)",
        g_sRunEndMsg[RUNMSG_SPLIT_PB], g_fRunTime_Split_End[client],
        g_fRunTime_Split_End[client] - splitPB);

      g_fRunTime_Split_PB[client] = g_fRunTime_Split_End[client];
    }
    // Not a record time
    else
    {
      PrintToChat(client, "%s = %.2f (PB +%.2f)",
        g_sRunEndMsg[RUNMSG_SPLIT], g_fRunTime_Split_End[client],
        g_fRunTime_Split_End[client] - splitPB);
    }
  }
  // Goal split
  else
  {
    g_fRunTime_Goal_End[client] = gametime - g_fRunTime_Goal_Start[client];
    /*PrintToChat(client, "Goal %f from %f - %f",
      g_fRunTime_Goal_End[client], gametime, g_fRunTime_Goal_Start[client]);*/
    // Check split time
    {
      // First time running this
      if (goalPB == 0)
      {
        PrintToChat(client, "%s = %.2f", g_sRunEndMsg[RUNMSG_GOAL],
          g_fRunTime_Goal_End[client]);

        g_fRunTime_Goal_PB[client] = g_fRunTime_Goal_End[client];
      }
      // New personal best
      else if (g_fRunTime_Goal_End[client] < goalPB)
      {
        PrintToChat(client, "%s = %.2f (PB %.2f)",
          g_sRunEndMsg[RUNMSG_GOAL_PB], g_fRunTime_Goal_End[client],
          g_fRunTime_Goal_End[client] - goalPB);

        g_fRunTime_Goal_PB[client] = g_fRunTime_Goal_End[client];
      }
      // Not a record time
      else
      {
        PrintToChat(client, "%s = %.2f (PB +%.2f)",
          g_sRunEndMsg[RUNMSG_GOAL], g_fRunTime_Goal_End[client],
          g_fRunTime_Goal_End[client] - goalPB);
      }
    }
    // Check final time
    {
      float combinedTime = g_fRunTime_Goal_End[client] + g_fRunTime_Split_End[client];
      /*PrintToChat(client, "Combined %f from %f + %f",
        combinedTime, g_fRunTime_Goal_End[client], g_fRunTime_Split_End[client]);*/

      // First time finishing
      if (combinedPB == 0)
      {
        PrintToChat(client, "%s = %.2f", g_sRunEndMsg[RUNMSG_COMBINED], combinedTime);

        g_fRunTime_Combined_PB[client] = combinedTime;
      }
      // New personal best
      else if (combinedTime < combinedPB)
      {
        PrintToChat(client, "%s = %.2f (PB %.2f)",
          g_sRunEndMsg[RUNMSG_COMBINED_PB], combinedTime, combinedTime - combinedPB);

        g_fRunTime_Combined_PB[client] = combinedTime;
      }
      // Not a record time
      else
      {
        PrintToChat(client, "%s = %.2f (PB +%.2f)",
          g_sRunEndMsg[RUNMSG_COMBINED], combinedTime, combinedTime - combinedPB);
      }
    }
  }

  FlipClientGoal(client);
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
