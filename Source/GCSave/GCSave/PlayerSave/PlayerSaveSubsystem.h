// Copyright (C) 2024 owoDra

#pragma once

#include "Subsystems/LocalPlayerSubsystem.h"

#include "PlayerSaveSubsystem.generated.h"

class USaveGame;
class UPlayerSave;


/**
 * Delegate notifies that the save has been loaded
 */
DECLARE_DELEGATE_TwoParams(FPlayerSaveEventDelegate, UPlayerSave*, bool);


/**
 * Subsystems that manage PlayerSave
 */
UCLASS()
class GCSAVE_API UPlayerSaveSubsystem : public ULocalPlayerSubsystem
{
	GENERATED_BODY()
public:
	UPlayerSaveSubsystem() {}

	//////////////////////////////////////////////////////////////////
	// Initialization
public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

protected:
	void LoadInitialPlayerSaves();


	//////////////////////////////////////////////////////////////////
	// Load Get Create
protected:
	//
	// List of saved game objects currently loaded
	//
	TMap<FString, TObjectPtr<UPlayerSave>> ActiveSaves;

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
	 * Load save games synchronously.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Global Save", meta = (AdvancedDisplay = "bShouldLoadIfNotLoaded", DeterminesOutputType = "PlayerSaveClass"))
	UPlayerSave* GetPlayerSave(
		TSubclassOf<UPlayerSave> PlayerSaveClass
		, const FString& SlotName
		, bool bShouldLoadIfNotLoaded = false);

	/**
	 * Load save games synchronously.
	 */
	UFUNCTION(BlueprintCallable, Category = "Global Save", meta = (AdvancedDisplay = "bForceLoad", DeterminesOutputType = "PlayerSaveClass"))
	UPlayerSave* SyncLoadPlayerSave(
		TSubclassOf<UPlayerSave> PlayerSaveClass
		, const FString& SlotName
		, bool bForceLoad = false);

	/**
	 * Load save games asynchronously.
	 */
	bool AsyncLoadPlayerSave(
		TSubclassOf<UPlayerSave> PlayerSaveClass
		, const FString& SlotName
		, bool bForceLoad = false
		, FPlayerSaveEventDelegate Delegate = FPlayerSaveEventDelegate());

	/**
	 * Saves the specified loaded save game object
	 *
	 * Note:
	 *	Cannot save if the specified save game has not yet been loaded
	 */
	UFUNCTION(BlueprintCallable, Category = "Global Save", meta = (DisplayName = "SaveGameToSlot"))
	bool SyncSaveGameToSlot(
		TSubclassOf<UPlayerSave> PlayerSaveClass
		, const FString& SlotName);

	/**
	 * Save loaded save game object specified asynchronously
	 *
	 * Note:
	 *	Cannot save if the specified save game has not yet been loaded
	 */
	UFUNCTION(BlueprintCallable, Category = "Global Save", meta = (DisplayName = "Async Save Global Save"))
	bool AsyncSaveGameToSlot(
		TSubclassOf<UPlayerSave> PlayerSaveClass
		, const FString& SlotName);

	bool AsyncSaveGameToSlot(
		TSubclassOf<UPlayerSave> PlayerSaveClass
		, const FString& SlotName
		, FPlayerSaveEventDelegate Delegate);

	/**
	 * Create new save games.
	 */
	UFUNCTION(BlueprintCallable, Category = "Global Save", meta = (DeterminesOutputType = "PlayerSaveClass"))
	UPlayerSave* CreateSave(TSubclassOf<UPlayerSave> PlayerSaveClass, const FString& SlotName);

protected:
	void AsyncLoadPlayerSaveInternal(
		TSubclassOf<UPlayerSave> PlayerSaveClass
		, const FString& SlotName
		, int32 Slot
		, FPlayerSaveEventDelegate Delegate);

	void AsyncSaveGameToSlotInternal(
		UPlayerSave* SaveObject
		, const FString& SlotName
		, int32 Slot
		, FPlayerSaveEventDelegate Delegate);

protected:
	void HandlePlayerSaveLoaded(const FString& Slotname, UPlayerSave* SaveObject);

	UPlayerSave* ProcessLoadedSave(USaveGame* BaseSave, const FString& SlotName, TSubclassOf<UPlayerSave> SaveGameClass);
	UPlayerSave* CreateNewSaveObject(TSubclassOf<UPlayerSave> PlayerSaveClass, const FString& Slotname);

	FString ResolveSlotName(TSubclassOf<UPlayerSave> PlayerSaveClass, const FString& SlotName) const;


	//////////////////////////////////////////////////////////////////
	// Pending Load List
protected:
	//
	// List of currently loading slot names
	//
	TSet<FString> PendingLoadList;

protected:
	void AddPendingLoad(const FString& Slotname);
	void RemovePendingLoad(const FString& Slotname);

	/**
	 * Returns whether or not the saved game with the specified slot name is currently loading.
	 */
	UFUNCTION(BlueprintCallable, Category = "Global Save|Pending Load")
	bool IsPendingLoad(const FString& Slotname) const;

	/**
	 * Returns whether there is a PlayerSave currently loaded
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
	 * Returns whether there is a PlayerSave currently saved
	 */
	UFUNCTION(BlueprintCallable, Category = "Global Save|Pending Save")
	bool HasPendingSave() const;

};
