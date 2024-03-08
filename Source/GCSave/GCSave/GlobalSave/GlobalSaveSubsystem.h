// Copyright (C) 2024 owoDra

#pragma once

#include "Subsystems/GameInstanceSubsystem.h"

#include "GlobalSaveSubsystem.generated.h"

class USaveGame;
class UGlobalSave;


/**
 * Delegate notifies that the save has been loaded
 */
DECLARE_DELEGATE_TwoParams(FGlobalSaveEventDelegate, UGlobalSave*, bool);


/**
 * Subsystems that manage GlobalSave
 */
UCLASS()
class GCSAVE_API UGlobalSaveSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
public:
	UGlobalSaveSubsystem() {}

	static const int32 SLOT_GlobalSave{ 0 };

	//////////////////////////////////////////////////////////////////
	// Initialization
public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

protected:
	void LoadInitialGlobalSaves();


	//////////////////////////////////////////////////////////////////
	// Load Get Create
protected:
	//
	// List of saved game objects currently loaded
	//
	UPROPERTY()
	TMap<FString, TObjectPtr<UGlobalSave>> ActiveSaves;

public:
	/**
	 * Get loaded saved game object from slot name
	 * 
	 * Note:
	 *	Return nullptr if not loaded
	 */
	template<typename T>
	T* GetActiveSave(const FString& SlotName) const
	{
		return Cast<T>(ActiveSaves.FindRef(SlotName));
	}

	/**
	 * Get loaded saved game object from class
	 *
	 * Note:
	 *	Return nullptr if not loaded
	 */
	template<typename T>
	T* GetActiveSave() const
	{
		return Cast<T>(ActiveSaves.FindRef(T::GetDefaultSaveSlotName(T::StaticClass())));
	}

	/**
	 * Get save game object from class
	 */
	template<typename T>
	T* GetSave(bool bShouldLoadIfNotLoaded = true)
	{
		return Cast<T>(GetGlobalSave(T::StaticClass(), FString(), bShouldLoadIfNotLoaded));
	}

	/**
	 * Load save games synchronously.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Global Save", meta = (AdvancedDisplay = "bShouldLoadIfNotLoaded", DeterminesOutputType = "GlobalSaveClass"))
	UGlobalSave* GetGlobalSave(
		TSubclassOf<UGlobalSave> GlobalSaveClass
		, const FString& SlotName
		, bool bShouldLoadIfNotLoaded = false);

	/**
	 * Load save games synchronously.
	 */
	UFUNCTION(BlueprintCallable, Category = "Global Save", meta = (AdvancedDisplay = "bForceLoad", DeterminesOutputType = "GlobalSaveClass"))
	UGlobalSave* SyncLoadGlobalSave(
		TSubclassOf<UGlobalSave> GlobalSaveClass
		, const FString& SlotName
		, bool bForceLoad = false);

	/**
	 * Load save games asynchronously.
	 */
	bool AsyncLoadGlobalSave(
		TSubclassOf<UGlobalSave> GlobalSaveClass
		, const FString& SlotName
		, bool bForceLoad = false
		, FGlobalSaveEventDelegate Delegate = FGlobalSaveEventDelegate());

	/**
	 * Saves the specified loaded save game object
	 *
	 * Note:
	 *	Cannot save if the specified save game has not yet been loaded
	 */
	UFUNCTION(BlueprintCallable, Category = "Global Save", meta = (DisplayName = "SaveGameToSlot"))
	bool SyncSaveGameToSlot(
		TSubclassOf<UGlobalSave> GlobalSaveClass
		, const FString& SlotName);

	/**
	 * Save loaded save game object specified asynchronously
	 *
	 * Note:
	 *	Cannot save if the specified save game has not yet been loaded
	 */
	UFUNCTION(BlueprintCallable, Category = "Global Save", meta = (DisplayName = "Async Save Global Save"))
	bool AsyncSaveGameToSlot(
		TSubclassOf<UGlobalSave> GlobalSaveClass
		, const FString& SlotName);

	bool AsyncSaveGameToSlot(
		TSubclassOf<UGlobalSave> GlobalSaveClass
		, const FString& SlotName
		, FGlobalSaveEventDelegate Delegate);

	/**
	 * Create new save games.
	 */
	UFUNCTION(BlueprintCallable, Category = "Global Save", meta = (DeterminesOutputType = "GlobalSaveClass"))
	UGlobalSave* CreateSave(TSubclassOf<UGlobalSave> GlobalSaveClass, const FString& SlotName);

	/**
	 * Release loaded save games.
	 */
	UFUNCTION(BlueprintCallable, Category = "Global Save")
	bool ReleaseSave(TSubclassOf<UGlobalSave> GlobalSaveClass, const FString& SlotName);


protected:
	void AsyncLoadGlobalSaveInternal(
		TSubclassOf<UGlobalSave> GlobalSaveClass
		, const FString& SlotName
		, int32 Slot
		, FGlobalSaveEventDelegate Delegate);

	void AsyncSaveGameToSlotInternal(
		UGlobalSave* SaveObject
		, const FString& SlotName
		, int32 Slot
		, FGlobalSaveEventDelegate Delegate);

protected:
	void HandleGlobalSaveLoaded(const FString& Slotname, UGlobalSave* SaveObject);

	UGlobalSave* ProcessLoadedSave(USaveGame* BaseSave, const FString& SlotName, TSubclassOf<UGlobalSave> SaveGameClass);
	UGlobalSave* CreateNewSaveObject(TSubclassOf<UGlobalSave> GlobalSaveClass, const FString& Slotname);

	FString ResolveSlotName(TSubclassOf<UGlobalSave> GlobalSaveClass, const FString& SlotName) const;


	//////////////////////////////////////////////////////////////////
	// Pending Load List
protected:
	//
	// List of currently loading slot names
	//
	UPROPERTY()
	TMap<FString, TSubclassOf<UGlobalSave>> PendingLoadList;

protected:
	void AddPendingLoad(const FString& Slotname, const TSubclassOf<UGlobalSave>& Class);
	void RemovePendingLoad(const FString& Slotname);

	/**
	 * Returns whether or not the saved game with the specified slot name is currently loading.
	 */
	UFUNCTION(BlueprintCallable, Category = "Global Save|Pending Load")
	bool IsPendingLoad(const FString& Slotname) const;

	/**
	 * Returns whether there is a GlobalSave currently loaded
	 */
	UFUNCTION(BlueprintCallable, Category = "Global Save|Pending Load")
	bool HasPendingLoad() const;


	//////////////////////////////////////////////////////////////////
	// Pending Save List
protected:
	//
	// List of currently saving slot names
	//
	TSet<FString> PendingSaveList;

protected:
	void AddPendingSave(const FString& Slotname);
	void RemovePendingSave(const FString& Slotname);

	/**
	 * Returns whether or not the saved game with the specified slot name is currently saving.
	 */
	UFUNCTION(BlueprintCallable, Category = "Global Save|Pending Save")
	bool IsPendingSave(const FString& Slotname) const;

	/**
	 * Returns whether there is a GlobalSave currently saved
	 */
	UFUNCTION(BlueprintCallable, Category = "Global Save|Pending Save")
	bool HasPendingSave() const;

};
