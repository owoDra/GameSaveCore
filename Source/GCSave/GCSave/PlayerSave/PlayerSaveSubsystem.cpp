// Copyright (C) 2024 owoDra

#include "PlayerSaveSubsystem.h"

#include "PlayerSave/PlayerSave.h"
#include "GameSaveDeveloperSettings.h"
#include "GCSaveLogs.h"

#include "Kismet/GameplayStatics.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(PlayerSaveSubsystem)


void UPlayerSaveSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	LoadInitialPlayerSaves();
}

void UPlayerSaveSubsystem::LoadInitialPlayerSaves()
{
	auto* DevSetting{ GetDefault<UGameSaveDeveloperSettings>() };

	UE_LOG(LogGameCore_PlayerSave, Log, TEXT("UPlayerSaveSubsystem::LoadInitialPlayerSaves: Start auto loading global saves"));

	for (const auto& KVP : DevSetting->PlayerSaveToAutoLoad)
	{
		auto* PlayerSaveClass{ KVP.Key.TryLoadClass<UPlayerSave>() };
		const auto SlotName{ KVP.Value };

		// SKip if no valid slot name

		const auto SlotNameToUse{ ResolveSlotName(PlayerSaveClass, SlotName) };
		if (SlotNameToUse.IsEmpty())
		{
			UE_LOG(LogGameCore_PlayerSave, Error, TEXT("UPlayerSaveSubsystem::LoadInitialPlayerSaves: No valid slot name"));
			continue;
		}

		AsyncLoadPlayerSaveInternal(PlayerSaveClass, SlotNameToUse, GetLocalPlayer()->GetPlatformUserIndex(), FPlayerSaveEventDelegate());
	}
}


UPlayerSave* UPlayerSaveSubsystem::GetPlayerSave(TSubclassOf<UPlayerSave> PlayerSaveClass, const FString& SlotName, bool bShouldLoadIfNotLoaded)
{
	// Suspend if no valid slot name

	const auto SlotNameToUse{ ResolveSlotName(PlayerSaveClass, SlotName) };
	if (SlotNameToUse.IsEmpty())
	{
		UE_LOG(LogGameCore_PlayerSave, Error, TEXT("UPlayerSaveSubsystem::GetPlayerSave: No valid slot name"));
		return nullptr;
	}

	// If already loaded, return it.

	if (auto FoundSave{ ActiveSaves.FindRef(SlotNameToUse) })
	{
		return FoundSave;
	}

	if (bShouldLoadIfNotLoaded)
	{
		return SyncLoadPlayerSave(PlayerSaveClass, SlotName, true);
	}

	return nullptr;
}

UPlayerSave* UPlayerSaveSubsystem::SyncLoadPlayerSave(TSubclassOf<UPlayerSave> PlayerSaveClass, const FString& SlotName, bool bForceLoad)
{
	// Suspend if no valid slot name

	const auto SlotNameToUse{ ResolveSlotName(PlayerSaveClass, SlotName) };
	if (SlotNameToUse.IsEmpty())
	{
		UE_LOG(LogGameCore_PlayerSave, Error, TEXT("UPlayerSaveSubsystem::SyncLoadPlayerSave: No valid slot name"));
		return nullptr;
	}

	if (!bForceLoad)
	{
		// If already loaded, return it.

		if (auto FoundSave{ ActiveSaves.FindRef(SlotNameToUse) })
		{
			return FoundSave;
		}
	}

	// If loading is allowed, try to load.

	if (UGameplayStatics::DoesSaveGameExist(SlotNameToUse, GetLocalPlayer()->GetPlatformUserIndex()))
	{
		if (auto* LoadedSave{ UGameplayStatics::LoadGameFromSlot(SlotNameToUse, GetLocalPlayer()->GetPlatformUserIndex()) })
		{
			return ProcessLoadedSave(LoadedSave, SlotNameToUse, PlayerSaveClass);
		}
	}

	// Create new

	return CreateNewSaveObject(PlayerSaveClass, SlotNameToUse);
}

bool UPlayerSaveSubsystem::AsyncLoadPlayerSave(TSubclassOf<UPlayerSave> PlayerSaveClass, const FString& SlotName, bool bForceLoad, FPlayerSaveEventDelegate Delegate)
{
	// Suspend if no valid slot name

	const auto SlotNameToUse{ ResolveSlotName(PlayerSaveClass, SlotName) };
	if (SlotNameToUse.IsEmpty())
	{
		UE_LOG(LogGameCore_PlayerSave, Error, TEXT("UPlayerSaveSubsystem::AsyncLoadPlayerSave: No valid slot name"));
		return false;
	}

	if (!bForceLoad)
	{
		// If already loaded, return it.

		if (auto FoundSave{ ActiveSaves.FindRef(SlotNameToUse) })
		{
			Delegate.ExecuteIfBound(FoundSave, true);
			return true;
		}
	}

	AsyncLoadPlayerSaveInternal(PlayerSaveClass, SlotNameToUse, GetLocalPlayer()->GetPlatformUserIndex(), Delegate);
	return true;
}

bool UPlayerSaveSubsystem::SyncSaveGameToSlot(TSubclassOf<UPlayerSave> PlayerSaveClass, const FString& SlotName)
{
	// Suspend if no valid slot name

	const auto SlotNameToUse{ ResolveSlotName(PlayerSaveClass, SlotName) };
	if (SlotNameToUse.IsEmpty())
	{
		UE_LOG(LogGameCore_PlayerSave, Error, TEXT("UPlayerSaveSubsystem::SyncSaveGameToSlot: No valid slot name"));
		return false;
	}

	// If already loaded, return it.

	if (auto FoundSave{ ActiveSaves.FindRef(SlotNameToUse) })
	{
		FoundSave->HandlePreSave();

		const auto bSuccess{ UGameplayStatics::SaveGameToSlot(FoundSave, SlotNameToUse, GetLocalPlayer()->GetPlatformUserIndex()) };

		FoundSave->HandlePostSave(bSuccess);

		return bSuccess;
	}

	return false;
}

bool UPlayerSaveSubsystem::AsyncSaveGameToSlot(TSubclassOf<UPlayerSave> PlayerSaveClass, const FString& SlotName)
{
	return AsyncSaveGameToSlot(PlayerSaveClass, SlotName, FPlayerSaveEventDelegate());
}

bool UPlayerSaveSubsystem::AsyncSaveGameToSlot(TSubclassOf<UPlayerSave> PlayerSaveClass, const FString& SlotName, FPlayerSaveEventDelegate Delegate)
{
	// Suspend if no valid slot name

	const auto SlotNameToUse{ ResolveSlotName(PlayerSaveClass, SlotName) };
	if (SlotNameToUse.IsEmpty())
	{
		UE_LOG(LogGameCore_PlayerSave, Error, TEXT("UPlayerSaveSubsystem::AsyncSaveGameToSlot: No valid slot name"));
		return false;
	}

	// If already loaded, return it.

	if (auto FoundSave{ ActiveSaves.FindRef(SlotNameToUse) })
	{
		AsyncSaveGameToSlotInternal(FoundSave, SlotNameToUse, GetLocalPlayer()->GetPlatformUserIndex(), Delegate);

		return true;
	}

	return false;
}

UPlayerSave* UPlayerSaveSubsystem::CreateSave(TSubclassOf<UPlayerSave> PlayerSaveClass, const FString& SlotName)
{
	// Suspend if no valid slot name

	const auto SlotNameToUse{ ResolveSlotName(PlayerSaveClass, SlotName) };
	if (SlotNameToUse.IsEmpty())
	{
		UE_LOG(LogGameCore_PlayerSave, Error, TEXT("UPlayerSaveSubsystem::CreateSave: No valid slot name"));
		return nullptr;
	}

	// Create new

	return CreateNewSaveObject(PlayerSaveClass, SlotNameToUse);
}


void UPlayerSaveSubsystem::AsyncLoadPlayerSaveInternal(TSubclassOf<UPlayerSave> PlayerSaveClass, const FString& SlotName, int32 Slot, FPlayerSaveEventDelegate Delegate)
{
	if (UGameplayStatics::DoesSaveGameExist(SlotName, Slot))
	{
		AddPendingLoad(SlotName);

		auto Lambda
		{
			FAsyncLoadGameFromSlotDelegate::CreateWeakLambda(this,
				[this, PlayerSaveClass, Delegate](const FString& SlotName, const int32 Slot, USaveGame* BaseSave)
				{
					auto* LoadedSave{ this->ProcessLoadedSave(BaseSave, SlotName, PlayerSaveClass) };

					this->RemovePendingLoad(SlotName);

					Delegate.ExecuteIfBound(LoadedSave, IsValid(LoadedSave));
				}
			)
		};

		UGameplayStatics::AsyncLoadGameFromSlot(SlotName, Slot, Lambda);
	}
	else
	{
		Delegate.ExecuteIfBound(CreateNewSaveObject(PlayerSaveClass, SlotName), true);
	}
}

void UPlayerSaveSubsystem::AsyncSaveGameToSlotInternal(UPlayerSave* SaveObject, const FString& SlotName, int32 Slot, FPlayerSaveEventDelegate Delegate)
{
	AddPendingSave(SlotName);

	SaveObject->HandlePreSave();

	auto SavedDelegate
	{
		FAsyncSaveGameToSlotDelegate::CreateWeakLambda(this,
			[this, SaveObject, Delegate](const FString& SlotName, const int32 UserIndex, bool bSuccess)
			{
				SaveObject->HandlePostSave(bSuccess);

				Delegate.ExecuteIfBound(SaveObject, bSuccess);

				this->RemovePendingSave(SlotName);
			}
		)
	};

	UGameplayStatics::AsyncSaveGameToSlot(SaveObject, SlotName, Slot, SavedDelegate);
}


void UPlayerSaveSubsystem::HandlePlayerSaveLoaded(const FString& Slotname, UPlayerSave* SaveObject)
{
	SaveObject->InitializeSaveGame(GetLocalPlayer(), Slotname);

	ActiveSaves.Emplace(Slotname, SaveObject);
}

UPlayerSave* UPlayerSaveSubsystem::ProcessLoadedSave(USaveGame* BaseSave, const FString& SlotName, TSubclassOf<UPlayerSave> SaveGameClass)
{
	auto* LoadedSave{ Cast<UPlayerSave>(BaseSave) };

	if (SaveGameClass && (!LoadedSave || !LoadedSave->IsA(SaveGameClass.Get())))
	{
		UE_LOG(LogGameCore_PlayerSave, Warning, TEXT("UPlayerSaveSubsystem::ProcessLoadedSave: Found invalid save game object(%s) in slot(%s)"), *GetNameSafe(LoadedSave), *SlotName);
		LoadedSave = nullptr;
	}

	if (!LoadedSave)
	{
		LoadedSave = CreateNewSaveObject(SaveGameClass, SlotName);
	}
	else
	{
		HandlePlayerSaveLoaded(SlotName, LoadedSave);
	}

	return LoadedSave;
}

UPlayerSave* UPlayerSaveSubsystem::CreateNewSaveObject(TSubclassOf<UPlayerSave> PlayerSaveClass, const FString& Slotname)
{
	auto* LoadedSave{ Cast<UPlayerSave>(UGameplayStatics::CreateSaveGameObject(PlayerSaveClass)) };

	if (ensure(LoadedSave))
	{
		HandlePlayerSaveLoaded(Slotname, LoadedSave);
	}

	return LoadedSave;
}

FString UPlayerSaveSubsystem::ResolveSlotName(TSubclassOf<UPlayerSave> PlayerSaveClass, const FString& SlotName) const
{
	// Returns the slot name from the class if available

	auto SlotName_FromClass{ PlayerSaveClass ? PlayerSaveClass.GetDefaultObject()->GetSaveSlotName() : FString() };

	if (!SlotName_FromClass.IsEmpty())
	{
		return SlotName_FromClass;
	}

	// Returns the slot name of the argument if it could not be obtained from the class

	return SlotName;
}


void UPlayerSaveSubsystem::AddPendingLoad(const FString& Slotname)
{
	UE_LOG(LogGameCore_PlayerSave, Log, TEXT("Start loading slot(%s)"), *Slotname);

	PendingLoadList.Add(Slotname);
}

void UPlayerSaveSubsystem::RemovePendingLoad(const FString& Slotname)
{
	UE_LOG(LogGameCore_PlayerSave, Log, TEXT("Finish loading slot(%s)"), *Slotname);

	PendingLoadList.Remove(Slotname);
}

bool UPlayerSaveSubsystem::IsPendingLoad(const FString& Slotname) const
{
	return PendingLoadList.Contains(Slotname);
}

bool UPlayerSaveSubsystem::HasPendingLoad() const
{
	return PendingLoadList.Num() > 0;
}


void UPlayerSaveSubsystem::AddPendingSave(const FString& Slotname)
{
	UE_LOG(LogGameCore_PlayerSave, Log, TEXT("Start saving slot(%s)"), *Slotname);

	PendingSaveList.Add(Slotname);
}

void UPlayerSaveSubsystem::RemovePendingSave(const FString& Slotname)
{
	UE_LOG(LogGameCore_PlayerSave, Log, TEXT("Finish saving slot(%s)"), *Slotname);

	PendingSaveList.Remove(Slotname);
}

bool UPlayerSaveSubsystem::IsPendingSave(const FString& Slotname) const
{
	return PendingSaveList.Contains(Slotname);
}

bool UPlayerSaveSubsystem::HasPendingSave() const
{
	return PendingSaveList.Num() > 0;
}
