// Copyright (C) 2024 owoDra

#include "PlayerSave.h"

#include "GCSaveLogs.h"

#include "Engine/LocalPlayer.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(PlayerSave)


FPlatformUserId UPlayerSave::GetPlatformUserId() const
{
	return OwningPlayer.IsValid() ? OwningPlayer->GetPlatformUserId() : FPlatformUserId();
}

int32 UPlayerSave::GetPlatformUserIndex() const
{
	return OwningPlayer.IsValid() ? OwningPlayer->GetPlatformUserIndex() : INDEX_NONE;
}


void UPlayerSave::InitializeSaveGame(const ULocalPlayer* LocalPlayer, FString InSlotName)
{
	OwningPlayer = LocalPlayer;
	SaveSlotName = InSlotName;

	LoadedDataVersion = SavedDataVersion;
	HandlePostLoad();
}

void UPlayerSave::ResetToDefault()
{
	SavedDataVersion = GetInvalidDataVersion();
	LoadedDataVersion = SavedDataVersion;

	OnResetToDefault();
}

void UPlayerSave::HandlePostLoad()
{
	OnPostLoad();
}

void UPlayerSave::HandlePreSave()
{
	OnPreSave();

	// Set the save data version and increment the requested count

	SavedDataVersion = GetLatestDataVersion();
	CurrentSaveRequest++;

	UE_LOG(LogGameCore_PlayerSave, Log, TEXT("Starting to save game(%s) request(%d) to slot(%s) for user(%d)"), *GetName(), CurrentSaveRequest, *GetSaveSlotName(), GetPlatformUserIndex());
}

void UPlayerSave::HandlePostSave(bool bSuccess)
{
	if (bSuccess)
	{
		ensure(CurrentSaveRequest > LastSuccessfulSaveRequest);
		LastSuccessfulSaveRequest = CurrentSaveRequest;

		UE_LOG(LogGameCore_PlayerSave, Log, TEXT("Successfully saved game(%s) request(%d) to slot(%s) for user(%d)"), *GetName(), LastSuccessfulSaveRequest, *GetSaveSlotName(), GetPlatformUserIndex());
	}
	else
	{
		ensure(CurrentSaveRequest > LastErrorSaveRequest);
		LastErrorSaveRequest = CurrentSaveRequest;

		UE_LOG(LogGameCore_PlayerSave, Error, TEXT("Failed to save game(%s) request(%d) to slot(%s) for user(%d)"), *GetName(), LastErrorSaveRequest, *GetSaveSlotName(), GetPlatformUserIndex());
	}

	OnPostSave(bSuccess);
}
