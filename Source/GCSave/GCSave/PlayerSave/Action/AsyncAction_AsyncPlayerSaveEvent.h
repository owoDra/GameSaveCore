// Copyright (C) 2024 owoDra

#pragma once

#include "Kismet/BlueprintAsyncActionBase.h"

#include "AsyncAction_AsyncPlayerSaveEvent.generated.h"

class UPlayerSave;
class UPlayerSaveSubsystem;


/**
 * Delegate to signal the global save event callback
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FPlayerSaveDynamicEventDelegate, UPlayerSave*, PlayerSave, bool, bSuccess);


/**
 * Async action for handle async load or save of a global save
 */
UCLASS()
class GCSAVE_API UAsyncAction_AsyncPlayerSaveEvent : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()
public:
	UAsyncAction_AsyncPlayerSaveEvent() {}

public:
	//
	// Delegate called when the save/load completes
	//
	UPROPERTY(BlueprintAssignable)
	FPlayerSaveDynamicEventDelegate Completed;

protected:
	enum class EPlayerSaveOperation : uint8 { Save, Load };

	//
	// Which operation is being run
	//
	EPlayerSaveOperation Operation{ EPlayerSaveOperation::Save };

protected:
	UPROPERTY(Transient)
	TWeakObjectPtr<UPlayerSaveSubsystem> InSubsystem{ nullptr };

	UPROPERTY(Transient)
	TSubclassOf<UPlayerSave> InPlayerSaveClass{ nullptr };

	UPROPERTY(Transient)
	FString InSlotName;

	UPROPERTY(Transient)
	bool bInForceLoad{ false };

public:
	/**
	 * Load global save asynchronously.
	 */
	UFUNCTION(BlueprintCallable, Category = "Global Save", meta = (AdvancedDisplay = "bForceLoad", AutoCreateRefTerm = "SlotName", BlueprintInternalUseOnly = "true"))
	static UAsyncAction_AsyncPlayerSaveEvent* AsyncLoadPlayerSave(UPlayerSaveSubsystem* Subsystem, TSubclassOf<UPlayerSave> PlayerSaveClass, const FString& SlotName, bool bForceLoad = false);

	/**
	 * Save global save asynchronously.
	 */
	UFUNCTION(BlueprintCallable, Category = "Global Save", meta = (AutoCreateRefTerm = "SlotName", BlueprintInternalUseOnly = "true", DisplayName = "Wait Async Save Private Save"))
	static UAsyncAction_AsyncPlayerSaveEvent* AsyncSavePlayerSave(UPlayerSaveSubsystem* Subsystem, TSubclassOf<UPlayerSave> PlayerSaveClass, const FString& SlotName);
	

public:
	virtual void Activate() override;

protected:
	virtual void HandleAsyncEvent(UPlayerSave* PlayerSaveObject, bool bSuccess);
	virtual void HandleFailedActivation();

};
