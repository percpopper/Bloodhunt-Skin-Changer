#include "MinHook/MinHook.h"
#include "Engine.h"

/*
 We will fill the FTigerCharacterAppearance in the parameters and apply it to our own character 
 instead of the actor its supposed to be applying to when previewing in the store. This can be called
 manually again later so you can use in game. This can be customized and improved a ton, even called without a hook 
 (im lazy af), but this is just an example.

 This is the function we will be looking into.
 Class Tiger.TigerCharCustomizationComponent
 Function Tiger.TigerCharCustomizationComponent.SetAppearance
 void SetAppearance(FTigerCharacterAppearance &NewAppearance)

 MinHook Was Used For Hooking: https://github.com/TsudaKageyu/minhook
 CheatIt Was Used For Engine.h/cpp & Util To Find The Function: https://github.com/guttir14/CheatIt
 */

#define BLOODRAINID 1435;
FTigerCharacterAppearance CurrentOutfitConfig;

// https://imgur.com/a/VEOjkDF
void(*OProcessEvent)(UObject*, UFunction*, void*) = nullptr;
void __fastcall ProcessEvent(UObject* thisUObject, UFunction* Function, void* Src)
{
	do {
		/*
			After checking it out, this function seems to be called in the store when
			going through battlepass or store items. You can print the data in
			CurrentOutfitConfig to get an idea of what your doing and what IDs your using.
			Call the function with our own 'this', being CharacterCustomizationComponent, which
			belongs to the same class as the function does and is for our own player, and with our own parameters that we
			stole or your own modified parameters.
		*/

		// Check if Function Tiger.TigerCharCustomizationComponent.SetAppearance is passed as the function
		if (Function == SetAppearanceUFunction) {

			// Save the information passed to the function in case you want to make your own call later and reapply the outfit.
			// OProcessEvent(CharacterCustomizationComponent, Function, &CurrentOutfitConfig);
			CurrentOutfitConfig = *((FTigerCharacterAppearance*)Src);

			// Modifying the parameters if you wish
			//((FTigerCharacterAppearance*)(Src))->PersistentAppearance.OutfitId = BLOODRAINID;

			// Get CharacterCustomizationComponent to pass as the 'this' pointer.

			UWorld* World = *(UWorld**)(WRLD);
			if (!World) break;

			UGameInstance* OwningGameInstance = World->OwningGameInstance;
			if (!OwningGameInstance) break;

			TArray<UPlayer*> LocalPlayers = OwningGameInstance->LocalPlayers;

			UPlayer* LocalPlayer = LocalPlayers[0];
			if (!LocalPlayer) break;

			APlayerController* MyController = LocalPlayer->PlayerController;
			if (!MyController) break;

			APawn* MyPlayer = MyController->AcknowledgedPawn;
			if (!MyPlayer) break;

			UTigerCharCustomizationComponent* CharacterCustomizationComponent = MyPlayer->CharacterCustomizationComponent;
			if (!CharacterCustomizationComponent) break;

			return OProcessEvent(CharacterCustomizationComponent, Function, Src);
		}

	} while (false);

	return OProcessEvent(thisUObject, Function, Src);
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
	if (ul_reason_for_call != DLL_PROCESS_ATTACH) return FALSE;

	if (!EngineInit()) return FALSE;

	if (MH_Initialize() != MH_STATUS::MH_OK) return FALSE;

	if (MH_CreateHook(ProcessEventAddress, ProcessEvent, reinterpret_cast<LPVOID*>(&OProcessEvent)) != MH_STATUS::MH_OK) return FALSE;

	if (MH_EnableHook(ProcessEventAddress) != MH_STATUS::MH_OK) return FALSE;

    return TRUE;
}
