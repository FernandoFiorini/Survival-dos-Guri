// SurvivalDosGuri - HUD com barras de vida e stamina

#include "Survival/SurvivalHUDWidget.h"
#include "Survival/SurvivalStatsComponent.h"
#include "Blueprint/WidgetTree.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Components/ProgressBar.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "GameFramework/Character.h"

namespace SurvivalHUD
{
	const FLinearColor HealthColor(0.75f, 0.08f, 0.08f);
	const FLinearColor CriticalHealthColor(1.0000f, 0.1274f, 0.0030f, 1.0f);
	const FLinearColor StaminaColor(0.0000f, 0.5149f, 0.0000f, 1.0f);
	const FLinearColor ExhaustedColor(0.3050f, 0.0000f, 1.0000f, 1.0f);
	constexpr float BarWidth = 320.f;
	constexpr float BarHeight = 30.f;

}

TSharedRef<SWidget> USurvivalHUDWidget::RebuildWidget()
{
	if (WidgetTree && !WidgetTree->RootWidget)
	{
		BuildLayout();
	}
	return Super::RebuildWidget();
}

void USurvivalHUDWidget::BuildLayout()
{
	UCanvasPanel* Root = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("Root"));
	WidgetTree->RootWidget = Root;

	// Posição das Barras de vida e stamina
	UVerticalBox* Bars = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("Bars"));
	if (UCanvasPanelSlot* BarsSlot = Root->AddChildToCanvas(Bars))
	{
		BarsSlot->SetAnchors(FAnchors(1.f, 1.f));
		BarsSlot->SetAlignment(FVector2D(1.f, 1.f));
		BarsSlot->SetPosition(FVector2D(-30.f, -30.f));
		BarsSlot->SetAutoSize(true);
	}

	UTextBlock* Text = nullptr;
	HealthBar = MakeBar(Bars, SurvivalHUD::HealthColor, Text);
	HealthText = Text;
	StaminaBar = MakeBar(Bars, SurvivalHUD::StaminaColor, Text);
	StaminaText = Text;

	// Mensagem no centro da tela (morte)
	CenterMessage = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("CenterMessage"));
	FSlateFontInfo BigFont = CenterMessage->GetFont();
	BigFont.Size = 42;
	CenterMessage->SetFont(BigFont);
	CenterMessage->SetColorAndOpacity(FSlateColor(FLinearColor(0.9f, 0.1f, 0.1f)));
	CenterMessage->SetJustification(ETextJustify::Center);
	CenterMessage->SetVisibility(ESlateVisibility::Collapsed);
	if (UCanvasPanelSlot* MsgSlot = Root->AddChildToCanvas(CenterMessage))
	{
		MsgSlot->SetAnchors(FAnchors(0.5f, 0.4f));
		MsgSlot->SetAlignment(FVector2D(0.5f, 0.5f));
		MsgSlot->SetAutoSize(true);
	}
}

UProgressBar* USurvivalHUDWidget::MakeBar(UVerticalBox* Parent, const FLinearColor& Color, UTextBlock*& OutText)
{
	USizeBox* Size = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass());
	Size->SetWidthOverride(SurvivalHUD::BarWidth);
	Size->SetHeightOverride(SurvivalHUD::BarHeight);
	if (UVerticalBoxSlot* VSlot = Parent->AddChildToVerticalBox(Size))
	{
		VSlot->SetPadding(FMargin(0.f, 3.f));
	}

	// Barra com o número por cima
	UOverlay* Overlay = WidgetTree->ConstructWidget<UOverlay>(UOverlay::StaticClass());
	Size->AddChild(Overlay);

	UProgressBar* Bar = WidgetTree->ConstructWidget<UProgressBar>(UProgressBar::StaticClass());
	Bar->SetFillColorAndOpacity(Color);
	Bar->SetPercent(1.f);
	if (UOverlaySlot* BarSlot = Overlay->AddChildToOverlay(Bar))
	{
		BarSlot->SetHorizontalAlignment(HAlign_Fill);
		BarSlot->SetVerticalAlignment(VAlign_Fill);
	}

	OutText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
	FSlateFontInfo SmallFont = OutText->GetFont();
	SmallFont.Size = 11;
	OutText->SetFont(SmallFont);
	OutText->SetShadowOffset(FVector2D(1.f, 1.f));
	OutText->SetShadowColorAndOpacity(FLinearColor(0.f, 0.f, 0.f, 0.8f));
	if (UOverlaySlot* TextSlot = Overlay->AddChildToOverlay(OutText))
	{
		TextSlot->SetHorizontalAlignment(HAlign_Center);
		TextSlot->SetVerticalAlignment(VAlign_Center);
	}
	return Bar;
}

void USurvivalHUDWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	const APawn* Pawn = GetOwningPlayerPawn();
	const USurvivalStatsComponent* Stats = Pawn ? Pawn->FindComponentByClass<USurvivalStatsComponent>() : nullptr;
	const ACharacter* Char = Cast<ACharacter>(Pawn); //Var personagem

	if (!Stats || !HealthBar)
	{
		return;
	}

	//Stats
	const bool personagemCorrendo = Stats->IsSprinting();
	const bool personagemCansado = Stats->IsExhausted();
	const bool criticalHealth = Stats->GetHealth() <= Stats->MaxHealth * 0.30f;
	const bool personagemMorto = Stats->IsDead();
	const bool personagemAgachado = Char && Char->bIsCrouched;
	//const bool personagemPulando = Char && Char->bIsJum;

	StaminaBar->SetPercent(Stats->GetStamina() / Stats->MaxStamina);
	StaminaBar->SetFillColorAndOpacity(Stats->IsExhausted() ? SurvivalHUD::ExhaustedColor : SurvivalHUD::StaminaColor);
	
	HealthBar->SetPercent(Stats->GetHealth() / Stats->MaxHealth);

	//CriticalHealthColor
	HealthBar->SetFillColorAndOpacity(criticalHealth ? SurvivalHUD::CriticalHealthColor : SurvivalHUD::HealthColor);
	HealthText->SetText(FText::FromString(FString::Printf(TEXT("Vida  %.0f / %.0f"), Stats->GetHealth(), Stats->MaxHealth)));
	
	StaminaText->SetText(FText::FromString(FString::Printf(TEXT("Stamina  %.0f / %.0f%s"),
		Stats->GetStamina(), Stats->MaxStamina, personagemCansado ? TEXT("  (cansado)") : (personagemCorrendo ? TEXT("  (correndo)") : personagemAgachado ? TEXT("  (agachado)") : TEXT("")))));

	if (personagemMorto)
	{
		CenterMessage->SetText(FText::FromString(TEXT("Você virou comida de zumbi\nrenascendo...")));
		CenterMessage->SetVisibility(ESlateVisibility::HitTestInvisible);
	}
	else
	{
		CenterMessage->SetVisibility(ESlateVisibility::Collapsed);
	}
}
