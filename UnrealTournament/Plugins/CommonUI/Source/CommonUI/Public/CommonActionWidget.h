// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CommonUITypes.h"

#include "CommonActionWidget.generated.h"

class SBox;

UCLASS(BlueprintType, Blueprintable)
class UCommonActionWidget: public UWidget
{
	GENERATED_UCLASS_BODY()

public:
	
	/** Begin UWidget */
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void ReleaseSlateResources(bool bReleaseChildren) override;
	virtual void SynchronizeProperties() override;
	/** End UWidet */
	
	UFUNCTION(BlueprintCallable, Category = CommonActionWidget)
	FSlateBrush GetIcon() const;

	UFUNCTION(BlueprintCallable, Category = CommonActionWidget)
	FText GetDisplayText() const;

	UFUNCTION(BlueprintCallable, Category = CommonActionWidget)
	void SetInputAction(FDataTableRowHandle InputActionRow);

	UFUNCTION(BlueprintCallable, Category = CommonActionWidget)
	bool IsHeldAction() const;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInputMethodChanged, bool, bUsingGamepad);
	UPROPERTY(BlueprintAssignable, Category = CommonActionWidget)
	FOnInputMethodChanged OnInputMethodChanged;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = CommonActionWidget, meta = (RowType = CommonInputActionData))
	FDataTableRowHandle InputActionDataRow;

protected:
	virtual void OnWidgetRebuilt() override;
	
	ECommonInputType GetCurrentInputType() const;
	
	void UpdateActionWidget();

	void ListenToInputMethodChanged(bool bListen = true);

	void HandleInputMethodChanged(bool bUsingGamepad);
	
	TSharedPtr<SBox> MyKeyBox;

	FSlateBrush Icon;
};