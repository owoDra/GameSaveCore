// Copyright (C) 2024 owoDra

#pragma once

#include "Kismet/BlueprintAsyncActionBase.h"

#include "AsyncAction_AsyncGlobalSaveEvent.generated.h"

class UGlobalSave;
class UGlobalSaveSubsystem;


/**
 * Delegate to signal the global save event callback
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FGlobalSaveDynamicEventDelegate, UGlobalSave*, GlobalSave, bool, bSuccess);


/**
 * Async action for handle async load or save of a global save
 */
UCLASS()
class GCSAVE_API UAsyncAction_AsyncGlobalSaveEvent : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()
public:
	UAsyncAction_AsyncGlobalSaveEvent() {}

public:
	//
	// Delegate called when the save/load completes
	//
	UPROPERTY(BlueprintAssignable)
	FGlobalSaveDynamicEventDelegate Completed;

protected:
	enum class EGlobalSaveOperation : uint8 { Save, Load };

	//
	// Which operation is being run
	//
	EGlobalSaveOperation Operation{ EGlobalSaveOperation::Save };

protected:
	UPROPERTY(Transient)
	TWeakObjectPtr<UGlobalSaveSubsystem> InSubsystem{ nullptr };

	UPROPERTY(Transient)
	TSubclassOf<UGlobalSave> InGlobalSaveClass{ nullptr };

	UPROPERTY(Transient)
	FString InSlotName;

	UPROPERTY(Transient)
	bool bInForceLoad{ false };

public:
	/**
	 * Load global save asynchronously.
	 */
	UFUNCTION(BlueprintCallable, Category = "Global Save", meta = (AdvancedDisplay = "bForceLoad", AutoCreateRefTerm = "SlotName", BlueprintInternalUseOnly = "true"))
	static UAsyncAction_AsyncGlobalSaveEvent* AsyncLoadGlobalSave(UGlobalSaveSubsystem* Subsystem, TSubclassOf<UGlobalSave> GlobalSaveClass, const FString& SlotName, bool bForceLoad = false);

	/**
	 * Save global save asynchronously.
	 */
	UFUNCTION(BlueprintCallable, Category = "Global Save", meta = (AutoCreateRefTerm = "SlotName", BlueprintInternalUseOnly = "true", DisplayName = "Wait Async Save Global Save"))
	static UAsyncAction_AsyncGlobalSaveEvent* AsyncSaveGlobalSave(UGlobalSaveSubsystem* Subsystem, TSubclassOf<UGlobalSave> GlobalSaveClass, const FString& SlotName);
	

public:
	virtual void Activate() override;

protected:
	virtual void HandleAsyncEvent(UGlobalSave* GlobalSaveObject, bool bSuccess);
	virtual void HandleFailedActivation();

};
