#include "MyUITags.h"

namespace MyUITags
{
	UE_DEFINE_GAMEPLAY_TAG(UI_Store_Inventory, "UI.Store.Inventory");
	UE_DEFINE_GAMEPLAY_TAG(UI_Store_Progression, "UI.Store.Progression");
	UE_DEFINE_GAMEPLAY_TAG(UI_Store_Battle, "UI.Store.Battle");
	UE_DEFINE_GAMEPLAY_TAG(UI_Store_PlayerVitals, "UI.Store.PlayerVitals");

	UE_DEFINE_GAMEPLAY_TAG(UI_Event_LevelUp, "UI.Event.LevelUp");
	UE_DEFINE_GAMEPLAY_TAG(UI_Event_ShowToast, "UI.Event.ShowToast");
	UE_DEFINE_GAMEPLAY_TAG(Gameplay_Event_RarePickup, "Gameplay.Event.RarePickup");

	UE_DEFINE_GAMEPLAY_TAG(UI_Request_Open_LevelUpDialog, "UI.Request.Open.LevelUpDialog");
	UE_DEFINE_GAMEPLAY_TAG(UI_Request_Show_RarePickupToast, "UI.Request.ShowToast.RarePickup");
}
