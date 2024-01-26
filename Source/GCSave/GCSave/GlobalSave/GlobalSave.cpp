// Copyright (C) 2024 owoDra

#include "GlobalSave.h"

#include "GCSaveLogs.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GlobalSave)


void UGlobalSave::InitializeSaveGame(UGameInstance* GameInstance, FString InSlotName)
{
	OwningGameInstance = GameInstance;
	SaveSlotName = InSlotName;

	LoadedDataVersion = SavedDataVersion;
	HandlePostLoad();
}

void UGlobalSave::ResetToDefault()
{
	SavedDataVersion = GetInvalidDataVersion();
	LoadedDataVersion = SavedDataVersion;

	OnResetToDefault();
}

void UGlobalSave::HandlePostLoad()
{
	OnPostLoad();
}

void UGlobalSave::HandlePreSave()
{
	OnPreSave();

	// Set the save data version and increment the requested count

	SavedDataVersion = GetLatestDataVersion();
	CurrentSaveRequest++;

	UE_LOG(LogGameCore_GlobalSave, Log, TEXT("Starting to save game(%s) request(%d) to slot(%s) "), *GetName(), CurrentSaveRequest, *GetSaveSlotName());
}

void UGlobalSave::HandlePostSave(bool bSuccess)
{
	if (bSuccess)
	{
		ensure(CurrentSaveRequest > LastSuccessfulSaveRequest);
		LastSuccessfulSaveRequest = CurrentSaveRequest;

		UE_LOG(LogGameCore_GlobalSave, Log, TEXT("Successfully saved game(%s) request(%d) to slot(%s)"), *GetName(), LastSuccessfulSaveRequest, *GetSaveSlotName());
	}
	else
	{
		ensure(CurrentSaveRequest > LastErrorSaveRequest);
		LastErrorSaveRequest = CurrentSaveRequest;

		UE_LOG(LogGameCore_GlobalSave, Error, TEXT("Failed to save game(%s) request(%d) to slot(%s)"), *GetName(), LastErrorSaveRequest, *GetSaveSlotName());
	}

	OnPostSave(bSuccess);
}
