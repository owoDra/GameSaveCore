// Copyright (C) 2024 owoDra

#include "AsyncAction_AsyncPlayerSaveEvent.h"

#include "PlayerSave/PlayerSaveSubsystem.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AsyncAction_AsyncPlayerSaveEvent)


UAsyncAction_AsyncPlayerSaveEvent* UAsyncAction_AsyncPlayerSaveEvent::AsyncLoadPlayerSave(UPlayerSaveSubsystem* Subsystem, TSubclassOf<UPlayerSave> PlayerSaveClass, const FString& SlotName, bool bForceLoad)
{
	auto* Action{ NewObject<UAsyncAction_AsyncPlayerSaveEvent>() };
	Action->Operation = EPlayerSaveOperation::Load;
	Action->InPlayerSaveClass = PlayerSaveClass;
	Action->InSlotName = SlotName;
	Action->bInForceLoad = bForceLoad;
	Action->InSubsystem = Subsystem;
	Action->RegisterWithGameInstance(Subsystem);
	
	return Action;
}

UAsyncAction_AsyncPlayerSaveEvent* UAsyncAction_AsyncPlayerSaveEvent::AsyncSavePlayerSave(UPlayerSaveSubsystem* Subsystem, TSubclassOf<UPlayerSave> PlayerSaveClass, const FString& SlotName)
{
	auto* Action{ NewObject<UAsyncAction_AsyncPlayerSaveEvent>() };
	Action->Operation = EPlayerSaveOperation::Save;
	Action->InPlayerSaveClass = PlayerSaveClass;
	Action->InSlotName = SlotName;
	Action->InSubsystem = Subsystem;
	Action->RegisterWithGameInstance(Subsystem);

	return Action;
}


void UAsyncAction_AsyncPlayerSaveEvent::Activate()
{
	auto bActivationSuccess{ false };

	if (InSubsystem.IsValid())
	{
		if (Operation == EPlayerSaveOperation::Load)
		{
			bActivationSuccess = 
				InSubsystem->AsyncLoadPlayerSave(InPlayerSaveClass, InSlotName, bInForceLoad, FPlayerSaveEventDelegate::CreateUObject(this, &ThisClass::HandleAsyncEvent));
		}
		else if (Operation == EPlayerSaveOperation::Save)
		{
			bActivationSuccess = 
				InSubsystem->AsyncSaveGameToSlot(InPlayerSaveClass, InSlotName, FPlayerSaveEventDelegate::CreateUObject(this, &ThisClass::HandleAsyncEvent));
		}
	}

	if (!bActivationSuccess)
	{
		HandleFailedActivation();
	}
}


void UAsyncAction_AsyncPlayerSaveEvent::HandleAsyncEvent(UPlayerSave* PlayerSaveObject, bool bSuccess)
{
	Completed.Broadcast(PlayerSaveObject, bSuccess);
	SetReadyToDestroy();
}

void UAsyncAction_AsyncPlayerSaveEvent::HandleFailedActivation()
{
	Completed.Broadcast(nullptr, false);
	SetReadyToDestroy();
}
