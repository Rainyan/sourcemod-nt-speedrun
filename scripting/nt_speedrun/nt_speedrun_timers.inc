#if defined GUARD_TIMERS_BASE
  #endinput
#endif
#define GUARD_TIMERS_BASE

public Action Timer_CheckCapzones(Handle timer)
{
  /*Make plugin fail if current map has no capzones.
    It will try to load itself again on the next level.
    TODO: This should probably fail a bit more gracefully
    (eg. unload self?) Also, relying purely on OnEntityCreated
    to find capzones means this will also fail if the plugin
    is loaded outside of a mapchange or a server start.*/
  for (int i = 0; i < sizeof(g_fCapzone); i++)
  {
    if (IsNullVector(g_fCapzone[TEAM_JINRAI][PROP_POSITION]) ||
        IsNullVector(g_fCapzone[TEAM_NSF][PROP_POSITION]))
    {
      SetFailState("No capzones found to use!");
    }
  }

  StartSpeedrunMode();
}

public Action Timer_ReadCapzone(Handle timer, int entRef)
{
  if (!IsValidEntity(entRef))
    return Plugin_Stop;

  int team = GetEntProp(entRef, Prop_Send, "m_OwningTeamNumber");
  if (!IsValidTeam(team))
  {
    return Plugin_Stop;
  }

  // Capzone goal for this team is already
  // defined, delete this extra capzone.
  // Currently, only 1 capzone per team is used
  // for a simple A-B speedrun route.
  // TODO: Make sure this always results in the same route,
  // otherwise use distance or index or something to ensure it.
  if (!IsNullVector(g_fCapzone[team][PROP_POSITION]))
  {
    AcceptEntityInput(entRef, "kill");
    return Plugin_Stop;
  }

  // Read capzone position
  GetEntPropVector(entRef, Prop_Send, "m_Position", g_fCapzone[team][PROP_POSITION]);
  if (IsNullVector(g_fCapzone[team][PROP_POSITION]))
  {
    ThrowError("Failed reading capzone position from entref %i", entRef);
  }
  // Read capzone radius
  g_fCapzone[team][PROP_RADIUS][0] = GetEntProp(entRef, Prop_Send, "m_Radius") * 1.0;
  if (g_fCapzone[team][PROP_RADIUS][0] == 0)
  {
    ThrowError("Failed reading capzone radius from entref %i", entRef);
  }

  /*
  PrintToServer("Logged cap position %f %f %f with radius %f",
    g_fCapzone[team][PROP_POSITION][POS_X],
    g_fCapzone[team][PROP_POSITION][POS_Y],
    g_fCapzone[team][PROP_POSITION][POS_Z],
    g_fCapzone[team][PROP_RADIUS][0]
  );
  */

  return Plugin_Handled;
}

public Action Timer_TempHalo(Handle timer)
{
  //PrintToChatAll("TIMER");
  int haloIndex = EntRefToEntIndex(g_iHaloRef);
  int laserIndex = EntRefToEntIndex(g_iLaserRef);
  if (!IsValidEntity(haloIndex) || !IsValidEntity(laserIndex))
  {
    ThrowError("Invalid laser or halo effect index %i / %i from entrefs %i / %i",
      haloIndex, laserIndex, g_iHaloRef, g_iLaserRef);
  }

  int framerate = 120;
  float width = 5.0;
  float amplitude = 1.0;
  int speed = 1;

  for (int team = TEAM_JINRAI; team <= TEAM_NSF; team++)
  {
    if (g_iHaloFrame >= framerate)
    {
      g_iHaloFrame = 0;
    }

    //PrintToChatAll("Making halo %i", team);

    TE_SetupBeamRingPoint(g_fCapzone[team][PROP_POSITION], 0.0,
      g_fCapzone[team][PROP_RADIUS][0], haloIndex, laserIndex,
      0, g_iHaloFrame, HALO_LIFE*2, width, amplitude, g_iHaloColor, speed, 0
    );
    TE_SendToAll();

    g_iHaloFrame += RoundToNearest(framerate / 3 * 1.0);
  }
}

public Action Timer_Speedrun(Handle timer)
{
  if (g_hTimer_Speedrun == null)
  {
    return Plugin_Stop;
  }

  for (int i = 1; i <= MaxClients; i++)
  {
    if (!IsValidClient(i) || IsFakeClient(i))
      continue;

    CheckRunnerGoal(i);
  }

  return Plugin_Continue;
}
