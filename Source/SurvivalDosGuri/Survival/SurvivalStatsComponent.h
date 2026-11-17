// SurvivalDosGuri - Vida e Stamina replicadas

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SurvivalStatsComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnStatChanged, float, NewValue, float, MaxValue);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDeath, AController*, Killer);

/**
 *  Vida e stamina do jogador.
 */
UCLASS(ClassGroup=(Survival), meta=(BlueprintSpawnableComponent))
class USurvivalStatsComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	USurvivalStatsComponent();

	// ---------- Configuração ----------

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Survival|Health", meta=(ClampMin=1))
	float MaxHealth = 100.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Survival|Stamina", meta=(ClampMin=1))
	float MaxStamina = 100.f;

	/** Espera (s) após perder vida antes de começar a regenerar */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Survival|Health", meta=(ClampMin=0))
	float HealthRegenDelay = 8.f;

	/** Vida recuperada por segundo regen*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Survival|Health", meta=(ClampMin=0))
	float HealthRegenPerSecond = 2.f;

	/** Stamina gasta por segundo correndo */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Survival|Stamina", meta=(ClampMin=0))
	float SprintDrainPerSecond = 20.f;

	/** Stamina recuperada por segundo sem correr */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Survival|Stamina", meta=(ClampMin=0))
	float StaminaRegenPerSecond = 15.f;

	/** Espera (s) após gastar stamina antes de começar a regenerar */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Survival|Stamina", meta=(ClampMin=0))
	float StaminaRegenDelay = 1.f;

	/** Stamina mínima para voltar a correr depois de esgotar */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Survival|Stamina", meta=(ClampMin=0))
	float MinStaminaToSprint = 15.f;

	/** Mostra vida/stamina no canto da tela (provisório, até termos o HUD) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Survival|Debug")
	bool bShowDebugText = false;

	// ---------- Eventos (HUD) ----------

	UPROPERTY(BlueprintAssignable, Category="Survival")
	FOnStatChanged OnHealthChanged;

	UPROPERTY(BlueprintAssignable, Category="Survival")
	FOnStatChanged OnStaminaChanged;

	UPROPERTY(BlueprintAssignable, Category="Survival")
	FOnDeath OnDeath;

	// ---------- Consulta ----------

	UFUNCTION(BlueprintPure, Category="Survival")
	float GetHealth() const { return Health; }

	UFUNCTION(BlueprintPure, Category="Survival")
	float GetStamina() const { return Stamina; }

	UFUNCTION(BlueprintPure, Category="Survival")
	bool IsDead() const { return Health <= 0.f; }

	UFUNCTION(BlueprintPure, Category="Survival")
	bool IsSprinting() const { return bSprinting; }

	/** Esgotou a stamina e ainda não recuperou o mínimo */
	UFUNCTION(BlueprintPure, Category="Survival")
	bool IsExhausted() const { return bExhausted; }

	/** Pode correr agora? (stamina suficiente e vivo) */
	UFUNCTION(BlueprintPure, Category="Survival")
	bool CanSprint() const;

	// ---------- Ações (só têm efeito no servidor) ----------

	/** Cura. Chame no servidor. */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category="Survival")
	void Heal(float Amount);

	/** Gasta stamina de uma vez (pulo, golpe...). Retorna false se não tinha o suficiente. */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category="Survival")
	bool ConsumeStamina(float Amount);

	/** Liga/desliga a corrida. Pode ser chamado pelo cliente dono: vai por RPC ao servidor. */
	UFUNCTION(BlueprintCallable, Category="Survival")
	void SetSprinting(bool bNewSprinting);

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(ReplicatedUsing=OnRep_Health, VisibleInstanceOnly, Category="Survival")
	float Health = 0.f;

	UPROPERTY(ReplicatedUsing=OnRep_Stamina, VisibleInstanceOnly, Category="Survival")
	float Stamina = 0.f;

	UPROPERTY(Replicated, VisibleInstanceOnly, Category="Survival")
	bool bSprinting = false;

	/** Esgotou a stamina: precisa recuperar até MinStaminaToSprint para correr de novo */
	UPROPERTY(Replicated)
	bool bExhausted = false;

	UFUNCTION()
	void OnRep_Health(float OldHealth);

	UFUNCTION()
	void OnRep_Stamina();

	UFUNCTION(Server, Reliable)
	void ServerSetSprinting(bool bNewSprinting);

	/** Recebe o dano padrão da Unreal (UGameplayStatics::ApplyDamage etc.) */
	UFUNCTION()
	void HandleTakeAnyDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser);

	void SetHealth(float NewHealth, AController* Instigator = nullptr);
	void SetStamina(float NewStamina);

	/** Ajusta a velocidade do CharacterMovement conforme a corrida */
	void ApplySprintSpeed();

	/** Velocidade de corrida = velocidade normal * este fator */
	UPROPERTY(EditDefaultsOnly, Category="Survival|Stamina", meta=(ClampMin=1))
	float SprintSpeedMultiplier = 1.3f;

	/** Velocidade de recuperação de vigor quanto abaixado */
	UPROPERTY(EditDefaultsOnly, Category="Survival|Stamina", meta=(ClampMin=1))
	float StaminaRegenMultiplierWhenCrouched = 1.5f;

	/** Sem stamina (esgotado): velocidade = velocidade normal * este fator */
	UPROPERTY(EditDefaultsOnly, Category="Survival|Stamina", meta=(ClampMin=0.1, ClampMax=1))
	float ExhaustedSpeedMultiplier = 0.6f;

private:
	float BaseWalkSpeed = 0.f;
	float TimeSinceStaminaUse = 0.f;
	float TimeSinceDamage = 0.f;
};
