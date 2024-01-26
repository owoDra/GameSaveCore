// Copyright (C) 2024 owoDra

#include "AsyncAction_AsyncGlobalSaveEvent.h"

#include "GlobalSave/GlobalSaveSubsystem.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AsyncAction_AsyncGlobalSaveEvent)


UAsyncAction_AsyncGlobalSaveEvent* UAsyncAction_AsyncGlobalSaveEvent::AsyncLoadGlobalSave(UGlobalSaveSubsystem* Subsystem, TSubclassOf<UGlobalSave> GlobalSaveClass, const FString& SlotName, bool bForceLoad)
{
	auto* Action{ NewObject<UAsyncAction_AsyncGlobalSaveEvent>() };
	Action->Operation = EGlobalSaveOperation::Load;
	Action->InGlobalSaveClass = GlobalSaveClass;
	Action->InSlotName = SlotName;
	Action->bInForceLoad = bForceLoad;
	Action->InSubsystem = Subsystem;

	if (Subsystem)
	{
		Action->RegisterWithGameInstance(Subsystem->GetGameInstance());
	}
	
	return Action;
}

UAsyncAction_AsyncGlobalSaveEvent* UAsyncAction_AsyncGlobalSaveEvent::AsyncSaveGlobalSave(UGlobalSaveSubsystem* Subsystem, TSubclassOf<UGlobalSave> GlobalSaveClass, const FString& SlotName)
{
	auto* Action{ NewObject<UAsyncAction_AsyncGlobalSaveEvent>() };
	Action->Operation = EGlobalSaveOperation::Save;
	Action->InGlobalSaveClass = GlobalSaveClass;
	Action->InSlotName = SlotName;
	Action->InSubsystem = Subsystem;

	if (Subsystem)
	{
		Action->RegisterWithGameInstance(Subsystem->GetGameInstance());
	}

	return Action;
}


void UAsyncAction_AsyncGlobalSaveEvent::Activate()
{
	auto bActivationSuccess{ false };

	if (InSubsystem.IsValid())
	{
		if (Operation == EGlobalSaveOperation::Load)
		{
			bActivationSuccess = 
				InSubsystem->AsyncLoadGlobalSave(InGlobalSaveClass, InSlotName, bInForceLoad, FGlobalSaveEventDelegate::CreateUObject(this, &ThisClass::HandleAsyncEvent));
		}
		else if (Operation == EGlobalSaveOperation::Save)
		{
			bActivationSuccess = 
				InSubsystem->AsyncSaveGameToSlot(InGlobalSaveClass, InSlotName, FGlobalSaveEventDelegate::CreateUObject(this, &ThisClass::HandleAsyncEvent));
		}
	}

	if (!bActivationSuccess)
	{
		HandleFailedActivation();
	}
}


void UAsyncAction_AsyncGlobalSaveEvent::HandleAsyncEvent(UGlobalSave* GlobalSaveObject, bool bSuccess)
{
	Completed.Broadcast(GlobalSaveObject, bSuccess);
	SetReadyToDestroy();
}

void UAsyncAction_AsyncGlobalSaveEvent::HandleFailedActivation()
{
	Completed.Broadcast(nullptr, false);
	SetReadyToDestroy();
}
