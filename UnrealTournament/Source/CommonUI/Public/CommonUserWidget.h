// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CommonUITypes.h"
#include "IBlueprintContextModule.h"
#include "BlueprintContextBase.h"
#include "BlueprintContextBase.h"
#include "IBlueprintContextModule.h"
#include "CommonUserWidget.generated.h"

UCLASS(ClassGroup = UI, meta = (Category = "Common UI"))
class COMMONUI_API UCommonUserWidget : public UUserWidget
{
	GENERATED_UCLASS_BODY()

	/**
	 * Called before the Construct Function.
	 * This function is called both in game and the designer! Only do cosmetic and completely locally dependent code in this.
	 * You cannot access contexts, or game instance, or any other game code.
	 * If in doubt please ask, you can cause crashes that can lose work!
	 */
	UFUNCTION(BlueprintImplementableEvent, BlueprintCosmetic, Category = "User Interface")
	void PreConstruct(bool IsDesignTime);

protected:
	/**
	*  Helper function to get the game instance
	*
	*  @return a pointer to the owning game instance
	*/
	template <class TGameInstance = UGameInstance>
	TGameInstance* GetGameInstance() const
	{
		if (UWorld* World = GetWorld())
		{
			return Cast<TGameInstance>(World->GetGameInstance());
		}

		return nullptr;
	}

	template <class ContextType>
	ContextType* GetContext() const
	{
		return Cast<ContextType>(UBlueprintContextLibrary::GetContext(GetOwningLocalPlayer(), ContextType::StaticClass()));
	}

	/** Set this to true if you don't want any pointer (mouse and touch) input to bubble past this widget */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CommonUserWidget")
	bool bConsumePointerInput;

	virtual void NativePreConstruct();
	virtual void OnWidgetRebuilt() override;

	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseWheel(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseButtonDoubleClick(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnTouchGesture(const FGeometry& InGeometry, const FPointerEvent& InGestureEvent) override;
	virtual FReply NativeOnTouchStarted(const FGeometry& InGeometry, const FPointerEvent& InGestureEvent) override;
	virtual FReply NativeOnTouchMoved(const FGeometry& InGeometry, const FPointerEvent& InGestureEvent) override;
	virtual FReply NativeOnTouchEnded(const FGeometry& InGeometry, const FPointerEvent& InGestureEvent) override;
};