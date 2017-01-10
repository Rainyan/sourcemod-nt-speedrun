#pragma semicolon 1

#include <sourcemod>
#include <sdkhooks>
#include <sdktools>
#include <neotokyo>
#include "nt_speedrun/nt_speedrun_base.inc"
#include "nt_speedrun/nt_speedrun_timers.inc"
#include "nt_speedrun/nt_speedrun_menu.inc"

public void OnPluginStart()
{
  CreateConVar("sm_neorun_version", PLUGIN_VERSION,
    "Plugin version.", FCVAR_DONTRECORD);

  RegConsoleCmd("sm_run", Command_RunMenu);
}

public void OnClientDisconnected(int client)
{
  ClearClientVariables(client);
}

public void OnMapStart()
{
  PrecacheSound(g_sSound_MenuCancel);
  PrecacheSound(g_sSound_MenuOK);
  PrecacheSound(g_sSound_Start);

  int laserIndex = PrecacheModel(g_sFX_Laser);
  int haloIndex = PrecacheModel(g_sFX_Halo);
  g_iLaserRef = EntIndexToEntRef(laserIndex);
  g_iHaloRef = EntIndexToEntRef(haloIndex);

  CreateTimer(1.0, Timer_CheckCapzones);
}

public void OnMapEnd()
{
  // Clear capzone positions when map ends.
  // OnEntityCreated will create new ones on next map.
  Capzones_Clear();
  // TODO: Is OnClientDisconnected called here,
  // or should players be looped for ClearClientVariables?
}

public void OnEntityCreated(int entity, const char[] classname)
{
  if (StrEqual(classname, "neo_ghost_retrieval_point"))
  {
    int ref = EntIndexToEntRef(entity);
    // Small timer because this gets called before the entity finishes spawning
    CreateTimer(0.1, Timer_ReadCapzone, ref);
  }
}

public Action Command_RunMenu(int client, int args)
{
  ShowMenu_Main(client);
}
