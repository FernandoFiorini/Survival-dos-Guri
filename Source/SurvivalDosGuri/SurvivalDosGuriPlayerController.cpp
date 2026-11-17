// Copyright Epic Games, Inc. All Rights Reserved.


#include "SurvivalDosGuriPlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "InputMappingContext.h"
#include "SurvivalDosGuriCameraManager.h"
#include "Blueprint/UserWidget.h"
#include "SurvivalDosGuri.h"
#include "Survival/SurvivalHUDWidget.h"
#include "Widgets/Input/SVirtualJoystick.h"

ASurvivalDosGuriPlayerController::ASurvivalDosGuriPlayerController()
{
	// set the player camera manager class
	PlayerCameraManagerClass = ASurvivalDosGuriCameraManager::StaticClass();
}

void ASurvivalDosGuriPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// HUD de vida/stamina, só na tela do jogador local
	if (IsLocalPlayerController())
	{
		SurvivalHUD = CreateWidget<USurvivalHUDWidget>(this, USurvivalHUDWidget::StaticClass());
		if (SurvivalHUD)
		{
			SurvivalHUD->AddToPlayerScreen(0);
		}
	}

	
	// only spawn touch controls on local player controllers
	if (IsLocalPlayerController() && ShouldUseTouchControls())
	{
		// spawn the mobile controls widget
		MobileControlsWidget = CreateWidget<UUserWidget>(this, MobileControlsWidgetClass);

		if (MobileControlsWidget)
		{
			// add the controls to the player screen
			MobileControlsWidget->AddToPlayerScreen(0);

		} else {

			UE_LOG(LogSurvivalDosGuri, Error, TEXT("Could not spawn mobile controls widget."));

		}

	}
}

void ASurvivalDosGuriPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	// only add IMCs for local player controllers
	if (IsLocalPlayerController())
	{
		// Add Input Mapping Context
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
		{
			for (UInputMappingContext* CurrentContext : DefaultMappingContexts)
			{
				Subsystem->AddMappingContext(CurrentContext, 0);
			}

			// only add these IMCs if we're not using mobile touch input
			if (!ShouldUseTouchControls())
			{
				for (UInputMappingContext* CurrentContext : MobileExcludedMappingContexts)
				{
					Subsystem->AddMappingContext(CurrentContext, 0);
				}
			}
		}
	}
	
}

bool ASurvivalDosGuriPlayerController::ShouldUseTouchControls() const
{
	// are we on a mobile platform? Should we force touch?
	return SVirtualJoystick::ShouldDisplayTouchInterface() || bForceTouchControls;
}
