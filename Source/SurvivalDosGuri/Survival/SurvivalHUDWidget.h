// SurvivalDosGuri - HUD com barras de vida e stamina

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SurvivalHUDWidget.generated.h"

class UProgressBar;
class UTextBlock;

/**
 *  HUD montado 100% em C++ (não precisa criar nada no editor).
 *  A cada frame lê a vida/stamina do personagem que o jogador controla,
 *  então continua funcionando depois de morrer e renascer.
 */
UCLASS()
class USurvivalHUDWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	UPROPERTY(Transient)
	TObjectPtr<UProgressBar> HealthBar;

	UPROPERTY(Transient)
	TObjectPtr<UProgressBar> StaminaBar;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> HealthText;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> StaminaText;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> CenterMessage;

private:
	void BuildLayout();
	UProgressBar* MakeBar(class UVerticalBox* Parent, const FLinearColor& Color, UTextBlock*& OutText);
};
