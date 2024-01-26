// Copyright (C) 2024 owoDra

#pragma once

#include "Engine/DeveloperSettings.h"

#include "GameSaveDeveloperSettings.generated.h"


/**
 * Settings for a Game framework.
 */
UCLASS(Config = "Game", Defaultconfig, meta = (DisplayName = "Game Save Core"))
class UGameSaveDeveloperSettings : public UDeveloperSettings
{
	GENERATED_BODY()
public:
	UGameSaveDeveloperSettings();

	///////////////////////////////////////////////
	// Save Games
public:
	//
	// List of global save automatically loaded at game startup
	//
	UPROPERTY(Config, EditAnywhere, Category = "Save Game", meta = (ForceInlineRow, MetaClass = "/Script/GCSave.GlobalSave"))
	TMap<FSoftClassPath, FString> GlobalSaveToAutoLoad;

	//
	// List of player save automatically loaded at game startup
	//
	UPROPERTY(Config, EditAnywhere, Category = "Save Game", meta = (ForceInlineRow, MetaClass = "/Script/GCSave.PlayerSave"))
	TMap<FSoftClassPath, FString> PlayerSaveToAutoLoad;

};

