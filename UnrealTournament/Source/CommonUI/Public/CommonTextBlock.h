// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CommonUITypes.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Components/TextBlock.h"
#include "CommonTextBlock.generated.h"

/* 
 * ---- All properties must be EditDefaultsOnly, BlueprintReadOnly !!! -----
 * We return the CDO to blueprints, so we cannot allow any changes (blueprint doesn't support const variables)
 */
UCLASS(Abstract, Blueprintable, ClassGroup = UI, meta = (Category = "Common UI"))
class COMMONUI_API UCommonTextStyle : public UObject
{
	GENERATED_BODY()

public:
	UCommonTextStyle();

	/** The font to apply at each size */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Font")
	FSlateFontInfo Font;

	/** The color of the text */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Color")
	FLinearColor Color;

	/** Whether or not the style uses a drop shadow */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Shadow")
	bool bUsesDropShadow;

	/** The offset of the drop shadow at each size */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Shadow", meta = (EditCondition = "bUsesDropShadow"))
	FVector2D ShadowOffset;

	/** The drop shadow color */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Shadow", meta = (EditCondition = "bUsesDropShadow"))
	FLinearColor ShadowColor;

	/** The amount of blank space left around the edges of text area at each size */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Properties")
	FMargin Margin;

	/** The amount to scale each lines height by at each size */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Properties")
	float LineHeightPercentage;

	UFUNCTION(BlueprintCallable, Category = "Common Text Style|Getters")
	void GetFont(FSlateFontInfo& OutFont) const;

	UFUNCTION(BlueprintCallable, Category = "Common Text Style|Getters")
	void GetColor(FLinearColor& OutColor) const;

	UFUNCTION(BlueprintCallable, Category = "Common Text Style|Getters")
	void GetMargin(FMargin& OutMargin) const;

	UFUNCTION(BlueprintCallable, Category = "Common Text Style|Getters")
	float GetLineHeightPercentage() const;

	UFUNCTION(BlueprintCallable, Category = "Common Text Style|Getters")
	void GetShadowOffset(FVector2D& OutShadowOffset) const;

	UFUNCTION(BlueprintCallable, Category = "Common Text Style|Getters")
	void GetShadowColor(FLinearColor& OutColor) const;

	void ToTextBlockStyle(FTextBlockStyle& OutTextBlockStyle);
};

/* 
 * ---- All properties must be EditDefaultsOnly, BlueprintReadOnly !!! -----
 * We return the CDO to blueprints, so we cannot allow any changes (blueprint doesn't support const variables)
 */
UCLASS(Abstract, Blueprintable, ClassGroup = UI, meta = (Category = "Common UI"))
class COMMONUI_API UCommonTextScrollStyle : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Properties")
	float Speed;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Properties")
	float StartDelay;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Properties")
	float EndDelay;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Properties")
	float FadeInDelay;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Properties")
	float FadeOutDelay;
};

UCLASS(Config = CommonUI, DefaultConfig, ClassGroup = UI, meta = (Category = "Common UI", DisplayName = "Common Text"))
class COMMONUI_API UCommonTextBlock : public UTextBlock
{
	GENERATED_UCLASS_BODY()
public:
	UFUNCTION(BlueprintCallable, Category = "Common Text")
	void SetWrapTextWidth(int32 InWrapTextAt);

	UFUNCTION(BlueprintCallable, Category = "Common Text")
	void SetStyle(TSubclassOf<UCommonTextStyle> InStyle);

	UFUNCTION(BlueprintCallable, Category = "Common Text")
	void SetScrollStyle(TSubclassOf<UCommonTextScrollStyle> InScrollStyle);

	UFUNCTION(BlueprintCallable, Category = "Common Text")
	void SetProperties(TSubclassOf<UCommonTextStyle> InStyle, TSubclassOf<UCommonTextScrollStyle> InScrollStyle = nullptr);

	/** References the text style to use */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Common Text", meta = (ExposeOnSpawn = true))
	TSubclassOf<UCommonTextStyle> Style;

	/** References the scroll style asset to use, no refernce disables scrolling*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Common Text", meta = (ExposeOnSpawn = true))
	TSubclassOf<UCommonTextScrollStyle> ScrollStyle;

protected:
	// UWidget interface
	virtual void SynchronizeProperties() override;
	// End of UWidget interface

	virtual TSharedRef<SWidget> RebuildWidget() override;

private:
	const UCommonTextStyle* GetStyleCDO() const;
	const UCommonTextScrollStyle* GetScrollStyleCDO() const;

	enum
	{
		EFadeIn = 0,
		EStart,
		EStartWait,
		EScrolling,
		EStop,
		EStopWait,
		EFadeOut,
	} ActiveState;

	bool OnTick( float Delta );

	float TimeElapsed;
	float ScrollOffset;
	float FontAlpha;
	bool bPlaying;

	TSharedPtr<SScrollBox> ScrollBox;
	TSharedPtr<SScrollBar> ScrollBar;

	FDelegateHandle TickHandle;
};