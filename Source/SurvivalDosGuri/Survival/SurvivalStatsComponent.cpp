// SurvivalDosGuri - Vida e Stamina replicadas

#include "Survival/SurvivalStatsComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Net/UnrealNetwork.h"
#include "Engine/Engine.h"

USurvivalStatsComponent::USurvivalStatsComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(true);
}

void USurvivalStatsComponent::BeginPlay()
{
	Super::BeginPlay();

	if (const ACharacter* Character = Cast<ACharacter>(GetOwner()))
	{
		BaseWalkSpeed = Character->GetCharacterMovement()->MaxWalkSpeed;
	}

	if (GetOwner()->HasAuthority())
	{
		Health = MaxHealth;
		Stamina = MaxStamina;
		GetOwner()->OnTakeAnyDamage.AddDynamic(this, &USurvivalStatsComponent::HandleTakeAnyDamage);
	}

	OnHealthChanged.Broadcast(Health, MaxHealth);
	OnStaminaChanged.Broadcast(Stamina, MaxStamina);
}

void USurvivalStatsComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(USurvivalStatsComponent, Health);
	DOREPLIFETIME(USurvivalStatsComponent, Stamina);
	DOREPLIFETIME(USurvivalStatsComponent, bSprinting);
	DOREPLIFETIME(USurvivalStatsComponent, bExhausted);
}

void USurvivalStatsComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// Mantém a velocidade coerente no servidor e nos clientes (inclusive após replicar bSprinting)
	ApplySprintSpeed();

	// Texto provisório na tela, só para o jogador dono deste personagem
	if (bShowDebugText && GEngine)
	{
		const APawn* Pawn = Cast<APawn>(GetOwner());
		if (Pawn && Pawn->IsLocallyControlled())
		{
			const FString Text = FString::Printf(TEXT("Vida: %.0f / %.0f    Stamina: %.0f / %.0f%s%s"),
				Health, MaxHealth, Stamina, MaxStamina,
				bSprinting ? TEXT("    [CORRENDO]") : (bExhausted ? TEXT("    [CANSADO]") : TEXT("")),
				IsDead() ? TEXT("    [MORTO]") : TEXT(""));
			GEngine->AddOnScreenDebugMessage((uint64)GetUniqueID(), 0.f, IsDead() ? FColor::Red : FColor::Green, Text);
		}
	}

	if (!GetOwner()->HasAuthority() || IsDead())
	{
		return;
	}

	// Só gasta stamina correndo se estiver de fato se movendo
	const bool bMoving = GetOwner()->GetVelocity().SizeSquared2D() > 25.f;

	// Agachou ou esgotou no meio da corrida: para de correr
	if (bSprinting && !CanSprint())
	{
		bSprinting = false;
	}

	if (bSprinting && bMoving)
	{
		TimeSinceStaminaUse = 0.f;
		SetStamina(Stamina - SprintDrainPerSecond * DeltaTime);

		if (Stamina <= 0.f)
		{
			bExhausted = true;
			bSprinting = false;
		}
	}
	else
	{
		TimeSinceStaminaUse += DeltaTime;
		if (TimeSinceStaminaUse >= StaminaRegenDelay && Stamina < MaxStamina)
		{
			SetStamina(Stamina + StaminaRegenPerSecond * DeltaTime);
		}
	}

	if (bExhausted && Stamina >= MinStaminaToSprint)
	{
		bExhausted = false;
	}
}

bool USurvivalStatsComponent::CanSprint() const
{
	const ACharacter* Character = Cast<ACharacter>(GetOwner());
	const bool bCrouched = Character && Character->bIsCrouched;
	return !IsDead() && !bExhausted && !bCrouched && Stamina > 0.f;
}

void USurvivalStatsComponent::SetSprinting(bool bNewSprinting)
{
	if (GetOwner()->HasAuthority())
	{
		bSprinting = bNewSprinting && CanSprint();
	}
	else
	{
		// Previsão local para a corrida não "travar" esperando o servidor
		bSprinting = bNewSprinting && CanSprint();
		ServerSetSprinting(bNewSprinting);
	}
	ApplySprintSpeed();
}

void USurvivalStatsComponent::ServerSetSprinting_Implementation(bool bNewSprinting)
{
	SetSprinting(bNewSprinting);
}

void USurvivalStatsComponent::ApplySprintSpeed()
{
	if (ACharacter* Character = Cast<ACharacter>(GetOwner()))
	{
		if (BaseWalkSpeed > 0.f)
		{
			float Multiplier = 1.f;
			if (bSprinting)
			{
				Multiplier = SprintSpeedMultiplier;
			}
			else if (bExhausted)
			{
				Multiplier = ExhaustedSpeedMultiplier;
			}
			Character->GetCharacterMovement()->MaxWalkSpeed = BaseWalkSpeed * Multiplier;
		}
	}
}

void USurvivalStatsComponent::Heal(float Amount)
{
	if (GetOwner()->HasAuthority() && !IsDead() && Amount > 0.f)
	{
		SetHealth(Health + Amount);
	}
}

bool USurvivalStatsComponent::ConsumeStamina(float Amount)
{
	if (!GetOwner()->HasAuthority() || IsDead() || Stamina < Amount)
	{
		return false;
	}
	TimeSinceStaminaUse = 0.f;
	SetStamina(Stamina - Amount);
	return true;
}

void USurvivalStatsComponent::HandleTakeAnyDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser)
{
	if (Damage > 0.f && !IsDead())
	{
		SetHealth(Health - Damage, InstigatedBy);
	}
}

void USurvivalStatsComponent::SetHealth(float NewHealth, AController* Instigator)
{
	const float OldHealth = Health;
	Health = FMath::Clamp(NewHealth, 0.f, MaxHealth);
	if (Health == OldHealth)
	{
		return;
	}

	OnHealthChanged.Broadcast(Health, MaxHealth);

	if (Health <= 0.f && OldHealth > 0.f)
	{
		bSprinting = false;
		OnDeath.Broadcast(Instigator);
	}
}

void USurvivalStatsComponent::SetStamina(float NewStamina)
{
	const float OldStamina = Stamina;
	Stamina = FMath::Clamp(NewStamina, 0.f, MaxStamina);
	if (Stamina != OldStamina)
	{
		OnStaminaChanged.Broadcast(Stamina, MaxStamina);
	}
}

void USurvivalStatsComponent::OnRep_Health(float OldHealth)
{
	OnHealthChanged.Broadcast(Health, MaxHealth);
	if (Health <= 0.f && OldHealth > 0.f)
	{
		OnDeath.Broadcast(nullptr);
	}
}

void USurvivalStatsComponent::OnRep_Stamina()
{
	OnStaminaChanged.Broadcast(Stamina, MaxStamina);
}
