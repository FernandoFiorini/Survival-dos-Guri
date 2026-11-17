// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "SurvivalDosGuriCharacter.generated.h"

class UInputComponent;
class USkeletalMeshComponent;
class UCameraComponent;
class UInputAction;
class USurvivalStatsComponent;
struct FInputActionValue;

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

/**
 *  A basic first person character
 */
UCLASS(abstract)
class ASurvivalDosGuriCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Pawn mesh: first person view (arms; seen only by self) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* FirstPersonMesh;

	/** First person camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FirstPersonCameraComponent;

	/** Vida e stamina (replicadas) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	USurvivalStatsComponent* SurvivalStats;

protected:

	/** Jump Input Action */
	UPROPERTY(EditAnywhere, Category ="Input")
	UInputAction* JumpAction;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, Category ="Input")
	UInputAction* MoveAction;

	/** Look Input Action */
	UPROPERTY(EditAnywhere, Category ="Input")
	class UInputAction* LookAction;

	/** Mouse Look Input Action */
	UPROPERTY(EditAnywhere, Category ="Input")
	class UInputAction* MouseLookAction;

	/** Sprint Input Action (segurar para correr) */
	UPROPERTY(EditAnywhere, Category ="Input")
	UInputAction* SurvivalSprintAction;
	
public:
	ASurvivalDosGuriCharacter();

protected:

	/** Called from Input Actions for movement input */
	void MoveInput(const FInputActionValue& Value);

	/** Called from Input Actions for looking input */
	void LookInput(const FInputActionValue& Value);

	/** Handles aim inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoAim(float Yaw, float Pitch);

	/** Handles move inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoMove(float Right, float Forward);

	/** Handles jump start inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoJumpStart();

	/** Handles jump end inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoJumpEnd();

	/** Começa a correr */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoSprintStart();

	/** Para de correr */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoSprintEnd();

	/** Tecla usada para correr (o código cria o mapeamento sozinho) */
	UPROPERTY(EditAnywhere, Category ="Input")
	FKey SprintKey;

	/** Agachar (criada pelo código; tecla em CrouchKey) */
	UPROPERTY(Transient)
	UInputAction* SurvivalCrouchAction;

	/** Tecla para agachar/levantar */
	UPROPERTY(EditAnywhere, Category ="Input")
	FKey CrouchKey;

	/** Agacha ou levanta */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoCrouchToggle();

	/** Stamina gasta a cada pulo */
	UPROPERTY(EditAnywhere, Category="Survival", meta=(ClampMin=0))
	float JumpStaminaCost = 10.f;

	/** Quanto a câmera desce ao agachar (cm) */
	UPROPERTY(EditAnywhere, Category="Survival", meta=(ClampMin=0))
	float CrouchCameraDrop = 50.f;

	/** Segundos até renascer depois de morrer */
	UPROPERTY(EditAnywhere, Category="Survival", meta=(ClampMin=0))
	float RespawnDelay = 5.f;

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual bool CanJumpInternal_Implementation() const override;
	virtual void OnJumped_Implementation() override;

	/** Chamado no servidor e nos clientes quando a vida chega a 0 */
	UFUNCTION()
	void HandleDeath(AController* Killer);

	/** Servidor: cria um personagem novo para o jogador */
	void Respawn();

	bool IsDead() const;

	FTimerHandle RespawnTimer;
	FVector FirstPersonMeshBaseLocation = FVector::ZeroVector;
	bool bDeathHandled = false;

	/** Mapeamento de teclas criado em tempo de execução para a corrida */
	UPROPERTY(Transient)
	class UInputMappingContext* SurvivalMappingContext;

	/** Quando o jogador local assume o personagem: registra a tecla de corrida */
	virtual void PawnClientRestart() override;

public:
	/** TESTE: digite no console (tecla ') "DebugDamage 25" para levar 25 de dano */
	UFUNCTION(Exec)
	void DebugDamage(float Amount = 25.f);

	/** TESTE: digite no console "DebugHeal 25" para curar 25 */
	UFUNCTION(Exec)
	void DebugHeal(float Amount = 25.f);

protected:
	UFUNCTION(Server, Reliable)
	void ServerDebugDamage(float Amount);

	UFUNCTION(Server, Reliable)
	void ServerDebugHeal(float Amount);

protected:

	/** Set up input action bindings */
	virtual void SetupPlayerInputComponent(UInputComponent* InputComponent) override;
	

public:

	/** Returns the first person mesh **/
	USkeletalMeshComponent* GetFirstPersonMesh() const { return FirstPersonMesh; }

	/** Returns first person camera component **/
	UCameraComponent* GetFirstPersonCameraComponent() const { return FirstPersonCameraComponent; }

	/** Returns the survival stats component **/
	USurvivalStatsComponent* GetSurvivalStats() const { return SurvivalStats; }

};

