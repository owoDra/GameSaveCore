// Copyright (C) 2024 owoDra

#pragma once

#include "GameFramework/SaveGame.h"

#include "PlayerSave.generated.h"

class ULocalPlayer;


/**
 * Save game class that requires only one per game.
 *
 * Tips:
 *	This save game is basically used to save the entire game progress, etc.
 */
UCLASS(Abstract, Blueprintable, BlueprintType)
class GCSAVE_API UPlayerSave : public USaveGame
{
public:
	GENERATED_BODY()
public:
	UPlayerSave() {}


	/////////////////////////////////////////////////////////////////////////////////////
	// Save Data Info
protected:
	//
	// The game instance this is connected to, can be null if it hasn't been initialized
	//
	UPROPERTY(Transient)
	TWeakObjectPtr<const ULocalPlayer> OwningPlayer;

	//
	// The slot name this was loaded from and that will be used to save to in the future
	//
	UPROPERTY(Transient)
	FString SaveSlotName;

	//
	// The value of GetLatestDataVersion when this was last saved.
	// 
	// Tips:
	//	Subclasses can override GetLatestDataVersion and then handle fixups in HandlePostLoad.
	//	This defaults to 0 so old save games that didn't previously subclass ULocalPlayerSaveGame will have 0 instead of the invalid version.
	//
	UPROPERTY()
	int32 SavedDataVersion{ 0 };

	//
	// The value of SavedDataVersion when a save was last loaded.
	// 
	// 
	// Tips:
	//	 This will be -1 for newly created saves
	//
	UPROPERTY(Transient)
	int32 LoadedDataVersion{ -1 };

	//
	// Integer that is incremented every time a save has been requested in the current session.
	// 
	// Tips:
	//	It can be used to know if one is in progress
	//
	UPROPERTY(Transient)
	int32 CurrentSaveRequest{ 0 };

	//
	// Integer that is set when a save completes successfully
	// 
	// Tips:
	//	If this equals RequestedSaveCount then the last save was successful
	//
	UPROPERTY(Transient)
	int32 LastSuccessfulSaveRequest{ 0 };

	//
	// Integer that is set when a save fails
	//
	UPROPERTY(Transient)
	int32 LastErrorSaveRequest = 0;


public:
	/**
	 * Returns the local paler this is associated with, this will be valid if it is ready to save
	 */
	UFUNCTION(BlueprintCallable, Category = "Save Game|Info")
	virtual const ULocalPlayer* GetLocalPlayer() const { return OwningPlayer.Get(); }

	/** 
	 * Returns the platform user to save to, based on Local Player by default 
	 */
	UFUNCTION(BlueprintCallable, Category = "Save Game|Info")
	virtual FPlatformUserId GetPlatformUserId() const;

	/** 
	 * Returns the user index to save to, based on Local Player by default 
	 */
	UFUNCTION(BlueprintCallable, Category = "Save Game|Info")
	virtual int32 GetPlatformUserIndex() const;

	/**
	 * Returns the save slot name to use
	 *
	 * Tips:
	 *	If you want to use a fixed name, please override this function in your derived class.
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Save Game|Info")
	FString GetSaveSlotName() const;
	virtual FString GetSaveSlotName_Implementation() const { return SaveSlotName; }

	static FString GetDefaultSaveSlotName(TSubclassOf<UPlayerSave> PlayerSaveClass)
	{
		return PlayerSaveClass ? PlayerSaveClass.GetDefaultObject()->GetSaveSlotName() : FString();
	}

	/**
	 * Returns the game-specific version number this was last saved/loaded as
	 */
	UFUNCTION(BlueprintCallable, Category = "Save Game|Info")
	virtual int32 GetSavedDataVersion() const { return SavedDataVersion; }

	/**
	 * Returns the invalid save data version, which means it has never been saved/loaded
	 */
	UFUNCTION(BlueprintCallable, Category = "Save Game|Info")
	virtual int32 GetInvalidDataVersion() const { return -1; }

	/**
	 * Returns the latest save data version, this is used when the new data is saved
	 *
	 * Tips:
	 *	Can manage versions by overriding this function in your derived class
	 */
	UFUNCTION(BlueprintCallable, Category = "Save Game|Info")
	virtual int32 GetLatestDataVersion() const { return 0; }

	/**
	 * Returns true if this was loaded from an existing save
	 */
	UFUNCTION(BlueprintCallable, Category = "Save Game|Info")
	virtual bool WasLoaded() const { return LoadedDataVersion != GetInvalidDataVersion(); }

	/**
	 * Returns true if a save is in progress
	 */
	UFUNCTION(BlueprintCallable, Category = "Save Game|Info")
	virtual bool IsSaveInProgress() const { return (CurrentSaveRequest > LastErrorSaveRequest) || (CurrentSaveRequest > LastSuccessfulSaveRequest); }

	/**
	 * Returns true if a save was ever requested, may still be in progress
	 */
	UFUNCTION(BlueprintCallable, Category = "Save Game|Info")
	virtual bool WasSaveRequested() const { return CurrentSaveRequest > 0; }

	/**
	 * Returns true if it has been saved at least once and the last save was successful
	 */
	UFUNCTION(BlueprintCallable, Category = "Save Game|Info")
	virtual bool WasLastSaveSuccessful() const { return (WasSaveRequested() && LastSuccessfulSaveRequest > LastErrorSaveRequest); }


	/////////////////////////////////////////////////////////////////////////////////////
	// Initialization
public:
	/**
	 * Initializes this save after either loading or initial creation, automatically called by load/create functions above
	 */
	virtual void InitializeSaveGame(const ULocalPlayer* LocalPlayer, FString InSlotName);

	/**
	 * Resets all saved data to default values, called when the load fails or manually
	 */
	UFUNCTION(BlueprintCallable, Category = "Save Game")
	virtual void ResetToDefault();

	/**
	 * Called after loading, this is not called for newly created saves
	 */
	virtual void HandlePostLoad();

	/**
	 * Called before saving, do any game-specific fixup here
	 */
	virtual void HandlePreSave();

	/**
	 * Called after saving finishes with success/failure result
	 */
	virtual void HandlePostSave(bool bSuccess);

protected:
	UFUNCTION(BlueprintImplementableEvent, Category = "Save Game")
	void OnResetToDefault();

	UFUNCTION(BlueprintImplementableEvent, Category = "Save Game")
	void OnPostLoad();

	UFUNCTION(BlueprintImplementableEvent, Category = "Save Game")
	void OnPreSave();

	UFUNCTION(BlueprintImplementableEvent, Category = "Save Game")
	void OnPostSave(bool bSuccess);

};

