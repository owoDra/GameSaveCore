// Copyright (C) 2024 owoDra

#include "GlobalSaveSubsystem.h"

#include "GlobalSave/GlobalSave.h"
#include "GameSaveDeveloperSettings.h"
#include "GCSaveLogs.h"

#include "Kismet/GameplayStatics.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GlobalSaveSubsystem)


void UGlobalSaveSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	LoadInitialGlobalSaves();
}

void UGlobalSaveSubsystem::LoadInitialGlobalSaves()
{
	auto* DevSetting{ GetDefault<UGameSaveDeveloperSettings>() };

	UE_LOG(LogGameCore_GlobalSave, Log, TEXT("UGlobalSaveSubsystem::LoadInitialGlobalSaves: Start auto loading global saves"));

	for (const auto& KVP : DevSetting->GlobalSaveToAutoLoad)
	{
		auto* GlobalSaveClass{ KVP.Key.TryLoadClass<UGlobalSave>() };
		const auto SlotName{ KVP.Value };

		// SKip if no valid slot name

		const auto SlotNameToUse{ ResolveSlotName(GlobalSaveClass, SlotName) };
		if (SlotNameToUse.IsEmpty())
		{
			UE_LOG(LogGameCore_GlobalSave, Error, TEXT("UGlobalSaveSubsystem::LoadInitialGlobalSaves: No valid slot name"));
			continue;
		}

		AsyncLoadGlobalSaveInternal(GlobalSaveClass, SlotNameToUse, UGlobalSaveSubsystem::SLOT_GlobalSave, FGlobalSaveEventDelegate());
	}
}


UGlobalSave* UGlobalSaveSubsystem::GetGlobalSave(TSubclassOf<UGlobalSave> GlobalSaveClass, const FString& SlotName, bool bShouldLoadIfNotLoaded)
{
	// Suspend if no valid slot name

	const auto SlotNameToUse{ ResolveSlotName(GlobalSaveClass, SlotName) };
	if (SlotNameToUse.IsEmpty())
	{
		UE_LOG(LogGameCore_GlobalSave, Error, TEXT("UGlobalSaveSubsystem::GetGlobalSave: No valid slot name"));
		return nullptr;
	}

	// If already loaded, return it.

	if (auto FoundSave{ ActiveSaves.FindRef(SlotNameToUse) })
	{
		return FoundSave;
	}

	if (bShouldLoadIfNotLoaded)
	{
		return SyncLoadGlobalSave(GlobalSaveClass, SlotName, true);
	}

	return nullptr;
}

UGlobalSave* UGlobalSaveSubsystem::SyncLoadGlobalSave(TSubclassOf<UGlobalSave> GlobalSaveClass, const FString& SlotName, bool bForceLoad)
{
	// Suspend if no valid slot name

	const auto SlotNameToUse{ ResolveSlotName(GlobalSaveClass, SlotName) };
	if (SlotNameToUse.IsEmpty())
	{
		UE_LOG(LogGameCore_GlobalSave, Error, TEXT("UGlobalSaveSubsystem::SyncLoadGlobalSave: No valid slot name"));
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

	if (UGameplayStatics::DoesSaveGameExist(SlotNameToUse, UGlobalSaveSubsystem::SLOT_GlobalSave))
	{
		if (auto* LoadedSave{ UGameplayStatics::LoadGameFromSlot(SlotNameToUse, UGlobalSaveSubsystem::SLOT_GlobalSave) })
		{
			return ProcessLoadedSave(LoadedSave, SlotNameToUse, GlobalSaveClass);
		}
	}

	// Create new

	return CreateNewSaveObject(GlobalSaveClass, SlotNameToUse);
}

bool UGlobalSaveSubsystem::AsyncLoadGlobalSave(TSubclassOf<UGlobalSave> GlobalSaveClass, const FString& SlotName, bool bForceLoad, FGlobalSaveEventDelegate Delegate)
{
	// Suspend if no valid slot name

	const auto SlotNameToUse{ ResolveSlotName(GlobalSaveClass, SlotName) };
	if (SlotNameToUse.IsEmpty())
	{
		UE_LOG(LogGameCore_GlobalSave, Error, TEXT("UGlobalSaveSubsystem::AsyncLoadGlobalSave: No valid slot name"));
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

	AsyncLoadGlobalSaveInternal(GlobalSaveClass, SlotNameToUse, UGlobalSaveSubsystem::SLOT_GlobalSave, Delegate);
	return true;
}

bool UGlobalSaveSubsystem::SyncSaveGameToSlot(TSubclassOf<UGlobalSave> GlobalSaveClass, const FString& SlotName)
{
	// Suspend if no valid slot name

	const auto SlotNameToUse{ ResolveSlotName(GlobalSaveClass, SlotName) };
	if (SlotNameToUse.IsEmpty())
	{
		UE_LOG(LogGameCore_GlobalSave, Error, TEXT("UGlobalSaveSubsystem::SyncSaveGameToSlot: No valid slot name"));
		return false;
	}

	// If already loaded, return it.

	if (auto FoundSave{ ActiveSaves.FindRef(SlotNameToUse) })
	{
		FoundSave->HandlePreSave();

		const auto bSuccess{ UGameplayStatics::SaveGameToSlot(FoundSave, SlotNameToUse, UGlobalSaveSubsystem::SLOT_GlobalSave) };

		FoundSave->HandlePostSave(bSuccess);

		return bSuccess;
	}

	return false;
}

bool UGlobalSaveSubsystem::AsyncSaveGameToSlot(TSubclassOf<UGlobalSave> GlobalSaveClass, const FString& SlotName)
{
	return AsyncSaveGameToSlot(GlobalSaveClass, SlotName, FGlobalSaveEventDelegate());
}

bool UGlobalSaveSubsystem::AsyncSaveGameToSlot(TSubclassOf<UGlobalSave> GlobalSaveClass, const FString& SlotName, FGlobalSaveEventDelegate Delegate)
{
	// Suspend if no valid slot name

	const auto SlotNameToUse{ ResolveSlotName(GlobalSaveClass, SlotName) };
	if (SlotNameToUse.IsEmpty())
	{
		UE_LOG(LogGameCore_GlobalSave, Error, TEXT("UGlobalSaveSubsystem::AsyncSaveGameToSlot: No valid slot name"));
		return false;
	}

	// If already loaded, return it.

	if (auto FoundSave{ ActiveSaves.FindRef(SlotNameToUse) })
	{
		AsyncSaveGameToSlotInternal(FoundSave, SlotNameToUse, UGlobalSaveSubsystem::SLOT_GlobalSave, Delegate);

		return true;
	}

	return false;
}

UGlobalSave* UGlobalSaveSubsystem::CreateSave(TSubclassOf<UGlobalSave> GlobalSaveClass, const FString& SlotName)
{
	// Suspend if no valid slot name

	const auto SlotNameToUse{ ResolveSlotName(GlobalSaveClass, SlotName) };
	if (SlotNameToUse.IsEmpty())
	{
		UE_LOG(LogGameCore_GlobalSave, Error, TEXT("UGlobalSaveSubsystem::CreateSave: No valid slot name"));
		return nullptr;
	}

	// Create new

	return CreateNewSaveObject(GlobalSaveClass, SlotNameToUse);
}

bool UGlobalSaveSubsystem::ReleaseSave(TSubclassOf<UGlobalSave> GlobalSaveClass, const FString& SlotName)
{
	// Suspend if no valid slot name

	const auto SlotNameToUse{ ResolveSlotName(GlobalSaveClass, SlotName) };
	if (SlotNameToUse.IsEmpty())
	{
		UE_LOG(LogGameCore_GlobalSave, Error, TEXT("UGlobalSaveSubsystem::CreateSave: No valid slot name"));
		return false;
	}

	ActiveSaves.Remove(SlotNameToUse);

	return true;
}


void UGlobalSaveSubsystem::AsyncLoadGlobalSaveInternal(TSubclassOf<UGlobalSave> GlobalSaveClass, const FString& SlotName, int32 Slot, FGlobalSaveEventDelegate Delegate)
{
	if (UGameplayStatics::DoesSaveGameExist(SlotName, Slot))
	{
		AddPendingLoad(SlotName);

		auto Lambda
		{
			FAsyncLoadGameFromSlotDelegate::CreateWeakLambda(this,
				[this, GlobalSaveClass, Delegate](const FString& SlotName, const int32 Slot, USaveGame* BaseSave)
				{
					auto* LoadedSave{ this->ProcessLoadedSave(BaseSave, SlotName, GlobalSaveClass) };

					this->RemovePendingLoad(SlotName);

					Delegate.ExecuteIfBound(LoadedSave, IsValid(LoadedSave));
				}
			)
		};

		UGameplayStatics::AsyncLoadGameFromSlot(SlotName, Slot, Lambda);
	}
	else
	{
		Delegate.ExecuteIfBound(CreateNewSaveObject(GlobalSaveClass, SlotName), true);
	}
}

void UGlobalSaveSubsystem::AsyncSaveGameToSlotInternal(UGlobalSave* SaveObject, const FString& SlotName, int32 Slot, FGlobalSaveEventDelegate Delegate)
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


void UGlobalSaveSubsystem::HandleGlobalSaveLoaded(const FString& Slotname, UGlobalSave* SaveObject)
{
	SaveObject->InitializeSaveGame(GetGameInstance(), Slotname);

	ActiveSaves.Emplace(Slotname, SaveObject);
}

UGlobalSave* UGlobalSaveSubsystem::ProcessLoadedSave(USaveGame* BaseSave, const FString& SlotName, TSubclassOf<UGlobalSave> SaveGameClass)
{
	auto* LoadedSave{ Cast<UGlobalSave>(BaseSave) };

	if (SaveGameClass && (!LoadedSave || (LoadedSave && !LoadedSave->IsA(SaveGameClass.Get()))))
	{
		UE_LOG(LogGameCore_GlobalSave, Warning, TEXT("UGlobalSaveSubsystem::ProcessLoadedSave: Found invalid save game object(%s) in slot(%s)"), *GetNameSafe(LoadedSave), *SlotName);
		LoadedSave = nullptr;
	}

	if (!LoadedSave)
	{
		LoadedSave = CreateNewSaveObject(SaveGameClass, SlotName);
	}
	else
	{
		HandleGlobalSaveLoaded(SlotName, LoadedSave);
	}

	return LoadedSave;
}

UGlobalSave* UGlobalSaveSubsystem::CreateNewSaveObject(TSubclassOf<UGlobalSave> GlobalSaveClass, const FString& Slotname)
{
	auto* LoadedSave{ Cast<UGlobalSave>(UGameplayStatics::CreateSaveGameObject(GlobalSaveClass)) };

	if (ensure(LoadedSave))
	{
		LoadedSave->ResetToDefault();
		HandleGlobalSaveLoaded(Slotname, LoadedSave);
	}

	return LoadedSave;
}

FString UGlobalSaveSubsystem::ResolveSlotName(TSubclassOf<UGlobalSave> GlobalSaveClass, const FString& SlotName) const
{
	// Returns the slot name from the class if available

	auto SlotName_FromClass{ GlobalSaveClass ? GlobalSaveClass.GetDefaultObject()->GetSaveSlotName() : FString() };

	if (!SlotName_FromClass.IsEmpty())
	{
		return SlotName_FromClass;
	}

	// Returns the slot name of the argument if it could not be obtained from the class

	return SlotName;
}


void UGlobalSaveSubsystem::AddPendingLoad(const FString& Slotname)
{
	UE_LOG(LogGameCore_GlobalSave, Log, TEXT("Start loading slot(%s)"), *Slotname);

	PendingLoadList.Add(Slotname);
}

void UGlobalSaveSubsystem::RemovePendingLoad(const FString& Slotname)
{
	UE_LOG(LogGameCore_GlobalSave, Log, TEXT("Finish loading slot(%s)"), *Slotname);

	PendingLoadList.Remove(Slotname);
}

bool UGlobalSaveSubsystem::IsPendingLoad(const FString& Slotname) const
{
	return PendingLoadList.Contains(Slotname);
}

bool UGlobalSaveSubsystem::HasPendingLoad() const
{
	return PendingLoadList.Num() > 0;
}


void UGlobalSaveSubsystem::AddPendingSave(const FString& Slotname)
{
	UE_LOG(LogGameCore_GlobalSave, Log, TEXT("Start saving slot(%s)"), *Slotname);

	PendingSaveList.Add(Slotname);
}

void UGlobalSaveSubsystem::RemovePendingSave(const FString& Slotname)
{
	UE_LOG(LogGameCore_GlobalSave, Log, TEXT("Finish saving slot(%s)"), *Slotname);

	PendingSaveList.Remove(Slotname);
}

bool UGlobalSaveSubsystem::IsPendingSave(const FString& Slotname) const
{
	return PendingSaveList.Contains(Slotname);
}

bool UGlobalSaveSubsystem::HasPendingSave() const
{
	return PendingSaveList.Num() > 0;
}
