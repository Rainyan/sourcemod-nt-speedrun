#if defined GUARD_SPEEDRUN_MENU
  #endinput
#endif
#define GUARD_SPEEDRUN_MENU

void ShowMenu_Main(int client)
{
  char titleText[24];
  Format(titleText, sizeof(titleText), "NT Speedrun (v%s)", PLUGIN_VERSION);

  Panel panel = new Panel();
  panel.SetTitle(titleText);
  panel.DrawText(" ");

  panel.DrawItem("Teleport to start");
  panel.DrawItem("Exit");

  panel.Send(client, PanelHandler_Main, MENU_TIME_FOREVER);
  delete panel;
}

public int PanelHandler_Main(Menu menu, MenuAction action, int client, int choice)
{
  if (action != MenuAction_Select)
    return;

  switch (choice)
  {
    case MENU_MAIN_TELEPORT:
    {
      EmitSoundToClient(client, g_sSound_MenuOK);
      TeleportPlayer(client);
    }
    case MENU_MAIN_EXIT:
    {
      EmitSoundToClient(client, g_sSound_MenuCancel);
    }
  }
}
