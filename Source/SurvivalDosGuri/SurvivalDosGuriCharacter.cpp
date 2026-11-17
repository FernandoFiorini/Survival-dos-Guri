// Copyright Epic Games, Inc. All Rights Reserved.

#include "SurvivalDosGuriCharacter.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "EnhancedInputComponent.h"
#include "InputActionValue.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "SurvivalDosGuri.h"
#include "Survival/SurvivalStatsComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "InputAction.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/GameModeBase.h"
#include "Engine/CollisionProfile.h"
#include "TimerManager.h"
#include "UObject/ConstructorHelpers.h"

ASurvivalDosGuriCharacter::ASurvivalDosGuriCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(55.f, 96.0f);
	
	// Create the first person mesh that will be viewed only by this character's owner
	FirstPersonMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("First Person Mesh"));

	FirstPersonMesh->SetupAttachment(GetMesh());
	FirstPersonMesh->SetOnlyOwnerSee(true);
	FirstPersonMesh->FirstPersonPrimitiveType = EFirstPersonPrimitiveType::FirstPerson;
	FirstPersonMesh->SetCollisionProfileName(FName("NoCollision"));

	// Create the Camera Component	
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("First Person Camera"));
	FirstPersonCameraComponent->SetupAttachment(FirstPersonMesh, FName("head"));
	FirstPersonCameraComponent->SetRelativeLocationAndRotation(FVector(-2.8f, 5.89f, 0.0f), FRotator(0.0f, 90.0f, -90.0f));
	FirstPersonCameraComponent->bUsePawnControlRotation = true;
	FirstPersonCameraComponent->bEnableFirstPersonFieldOfView = true;
	FirstPersonCameraComponent->bEnableFirstPersonScale = true;
	FirstPersonCameraComponent->FirstPersonFieldOfView = 70.0f;
	FirstPersonCameraComponent->FirstPersonScale = 0.6f;

	// configure the character comps
	GetMesh()->SetOwnerNoSee(true);
	GetMesh()->FirstPersonPrimitiveType = EFirstPersonPrimitiveType::WorldSpaceRepresentation;

	GetCapsuleComponent()->SetCapsuleSize(34.0f, 96.0f);

	// Configure character movement
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;
	GetCharacterMovement()->AirControl = 0.5f;

	// Vida e stamina
	SurvivalStats = CreateDefaultSubobject<USurvivalStatsComponent>(TEXT("Survival Stats"));

	// Corrida: usa a Input Action IA_Sprint que já veio no projeto, na tecla Shift
	static ConstructorHelpers::FObjectFinder<UInputAction> SprintActionFinder(TEXT("/Game/Variant_Horror/Input/IA_Sprint.IA_Sprint"));
	if (SprintActionFinder.Succeeded())
	{
		SurvivalSprintAction = SprintActionFinder.Object;
	}
	SprintKey = EKeys::LeftShift;
	CrouchKey = EKeys::C;

	// Agachar
	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;
	GetCharacterMovement()->MaxWalkSpeedCrouched = 250.f;
}

void ASurvivalDosGuriCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{	
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ASurvivalDosGuriCharacter::DoJumpStart);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ASurvivalDosGuriCharacter::DoJumpEnd);

		// Crouching (a Input Action é criada aqui mesmo, sem asset)
		if (!SurvivalCrouchAction)
		{
			SurvivalCrouchAction = NewObject<UInputAction>(this, TEXT("SurvivalCrouchAction"));
		}
		EnhancedInputComponent->BindAction(SurvivalCrouchAction, ETriggerEvent::Started, this, &ASurvivalDosGuriCharacter::DoCrouchToggle);

		// Sprinting
		if (SurvivalSprintAction)
		{
			EnhancedInputComponent->BindAction(SurvivalSprintAction, ETriggerEvent::Started, this, &ASurvivalDosGuriCharacter::DoSprintStart);
			EnhancedInputComponent->BindAction(SurvivalSprintAction, ETriggerEvent::Completed, this, &ASurvivalDosGuriCharacter::DoSprintEnd);
		}

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ASurvivalDosGuriCharacter::MoveInput);

		// Looking/Aiming
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ASurvivalDosGuriCharacter::LookInput);
		EnhancedInputComponent->BindAction(MouseLookAction, ETriggerEvent::Triggered, this, &ASurvivalDosGuriCharacter::LookInput);
	}
	else
	{
		UE_LOG(LogSurvivalDosGuri, Error, TEXT("'%s' Failed to find an Enhanced Input Component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}


void ASurvivalDosGuriCharacter::MoveInput(const FInputActionValue& Value)
{
	// get the Vector2D move axis
	FVector2D MovementVector = Value.Get<FVector2D>();

	// pass the axis values to the move input
	DoMove(MovementVector.X, MovementVector.Y);

}

void ASurvivalDosGuriCharacter::LookInput(const FInputActionValue& Value)
{
	// get the Vector2D look axis
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	// pass the axis values to the aim input
	DoAim(LookAxisVector.X, LookAxisVector.Y);

}

void ASurvivalDosGuriCharacter::DoAim(float Yaw, float Pitch)
{
	if (GetController())
	{
		// pass the rotation inputs
		AddControllerYawInput(Yaw);
		AddControllerPitchInput(Pitch);
	}
}

void ASurvivalDosGuriCharacter::DoMove(float Right, float Forward)
{
	if (GetController() && !IsDead())
	{
		// pass the move inputs
		AddMovementInput(GetActorRightVector(), Right);
		AddMovementInput(GetActorForwardVector(), Forward);
	}
}

void ASurvivalDosGuriCharacter::DoJumpStart()
{
	//Se esta vivo pula, mas se está agachado levanta
	if (!IsDead())
	{
		if (bIsCrouched) {
			UnCrouch();
		}
		else {
			Jump();

		}

	}
}

void ASurvivalDosGuriCharacter::DoJumpEnd()
{
	// pass StopJumping to the character
	StopJumping();
}

void ASurvivalDosGuriCharacter::DoSprintStart()
{
	if (bIsCrouched)
	{
		UnCrouch();
	}
	SurvivalStats->SetSprinting(true);
}

void ASurvivalDosGuriCharacter::DoSprintEnd()
{
	SurvivalStats->SetSprinting(false);
}

void ASurvivalDosGuriCharacter::PawnClientRestart()
{
	Super::PawnClientRestart();

	const APlayerController* PC = Cast<APlayerController>(GetController());
	if (!PC || !PC->GetLocalPlayer())
	{
		return;
	}

	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
	{
		if (!SurvivalMappingContext)
		{
			SurvivalMappingContext = NewObject<UInputMappingContext>(this, TEXT("SurvivalMappingContext"));
			if (SurvivalSprintAction)
			{
				SurvivalMappingContext->MapKey(SurvivalSprintAction, SprintKey);
			}
			if (SurvivalCrouchAction)
			{
				SurvivalMappingContext->MapKey(SurvivalCrouchAction, CrouchKey);
			}
		}
		Subsystem->AddMappingContext(SurvivalMappingContext, 1);
	}
}

void ASurvivalDosGuriCharacter::DebugDamage(float Amount)
{
	ServerDebugDamage(Amount);
}

void ASurvivalDosGuriCharacter::ServerDebugDamage_Implementation(float Amount)
{
	UGameplayStatics::ApplyDamage(this, Amount, GetController(), this, nullptr);
}

void ASurvivalDosGuriCharacter::DebugHeal(float Amount)
{
	ServerDebugHeal(Amount);
}

void ASurvivalDosGuriCharacter::ServerDebugHeal_Implementation(float Amount)
{
	SurvivalStats->Heal(Amount);
}

// ================== Etapa 1: pulo, agachar, morte e renascer ==================

void ASurvivalDosGuriCharacter::BeginPlay()
{
	Super::BeginPlay();

	FirstPersonMeshBaseLocation = FirstPersonMesh->GetRelativeLocation();

	// OnDeath dispara no servidor e também nos clientes (via replicação da vida)
	SurvivalStats->OnDeath.AddDynamic(this, &ASurvivalDosGuriCharacter::HandleDeath);
}

void ASurvivalDosGuriCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// Desce/sobe a câmera suavemente ao agachar
	if (!IsDead())
	{
		const FVector Target = FirstPersonMeshBaseLocation - FVector(0.f, 0.f, bIsCrouched ? CrouchCameraDrop : 0.f);
		const FVector Current = FirstPersonMesh->GetRelativeLocation();
		if (!Current.Equals(Target, 0.1f))
		{
			FirstPersonMesh->SetRelativeLocation(FMath::VInterpTo(Current, Target, DeltaSeconds, 12.f));
		}
	}
}

bool ASurvivalDosGuriCharacter::IsDead() const
{
	return SurvivalStats && SurvivalStats->IsDead();
}

void ASurvivalDosGuriCharacter::DoCrouchToggle()
{
	if (IsDead())
	{
		return;
	}

	if (bIsCrouched)
	{
		UnCrouch();
	}
	else
	{
		SurvivalStats->SetSprinting(false);
		Crouch();
	}
}

bool ASurvivalDosGuriCharacter::CanJumpInternal_Implementation() const
{
	// Sem stamina suficiente, não pula
	return Super::CanJumpInternal_Implementation()
		&& !IsDead()
		&& SurvivalStats->GetStamina() >= JumpStaminaCost;
}

void ASurvivalDosGuriCharacter::OnJumped_Implementation()
{
	Super::OnJumped_Implementation();

	if (HasAuthority())
	{
		SurvivalStats->ConsumeStamina(JumpStaminaCost);
	}
}

void ASurvivalDosGuriCharacter::HandleDeath(AController* Killer)
{
	if (bDeathHandled)
	{
		return;
	}
	bDeathHandled = true;

	// Para tudo
	GetCharacterMovement()->StopMovementImmediately();
	GetCharacterMovement()->DisableMovement();
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);

	// A câmera fica onde estava a cabeça; os braços somem
	FirstPersonCameraComponent->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
	FirstPersonMesh->SetVisibility(false);

	// O corpo cai (ragdoll) e o próprio jogador consegue vê-lo
	GetMesh()->SetOwnerNoSee(false);
	GetMesh()->SetCollisionProfileName(FName(TEXT("Ragdoll")));
	GetMesh()->SetSimulatePhysics(true);

	if (HasAuthority())
	{
		GetWorldTimerManager().SetTimer(RespawnTimer, this, &ASurvivalDosGuriCharacter::Respawn, FMath::Max(RespawnDelay, 0.1f), false);
	}
}

void ASurvivalDosGuriCharacter::Respawn()
{
	AController* OldController = GetController();
	AGameModeBase* GameMode = GetWorld()->GetAuthGameMode();
	if (!OldController || !GameMode)
	{
		return;
	}

	// Solta o corpo e cria um personagem novo num PlayerStart
	OldController->UnPossess();
	GameMode->RestartPlayer(OldController);

	// O corpo some depois de um tempo
	SetLifeSpan(20.f);
}
