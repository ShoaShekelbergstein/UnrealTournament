// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Components/WidgetSwitcher.h"
#include "CommonActivatablePanel.h"
#include "Animation/CurveHandle.h"
#include "CommonWidgetSwitcher.generated.h"

class SCommonAnimatedSwitcher;

UENUM(BlueprintType)
enum class ECommonSwitcherTransition : uint8
{
	/** Fade transition only with no movement */
	FadeOnly,
	/** Increasing the active index goes right, decreasing goes left */
	Horizontal,
	/** Increasing the active index goes up, decreasing goes down */
	Vertical,
	/** Increasing the active index zooms in, decreasing zooms out */
	Zoom
};

UENUM(BlueprintType)
enum class ETransitionCurve : uint8
{
	/** Linear interpolation, with no easing */
	Linear,
	/** Quadratic ease in */
	QuadIn,
	/** Quadratic ease out */
	QuadOut,
	/** Quadratic ease in, quadratic ease out */
	QuadInOut,
	/** Cubic ease in */
	CubicIn,
	/** Cubic ease out */
	CubicOut,
	/** Cubic ease in, cubic ease out */
	CubicInOut,
};

static FORCEINLINE ECurveEaseFunction::Type TransitionCurveToCurveEaseFunction(ETransitionCurve CurveType)
{
	switch (CurveType)
	{
		default:
		case ETransitionCurve::Linear: return ECurveEaseFunction::Linear;
		case ETransitionCurve::QuadIn: return ECurveEaseFunction::QuadIn;
		case ETransitionCurve::QuadOut: return ECurveEaseFunction::QuadOut;
		case ETransitionCurve::QuadInOut: return ECurveEaseFunction::QuadInOut;
		case ETransitionCurve::CubicIn: return ECurveEaseFunction::CubicIn;
		case ETransitionCurve::CubicOut: return ECurveEaseFunction::CubicOut;
		case ETransitionCurve::CubicInOut: return ECurveEaseFunction::CubicInOut;
	}
}


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnActiveWidgetDeactivated, UCommonActivatablePanel*, DeactivatedWidget);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnActiveWidgetChanged, UWidget*, ActiveWidget, int32, ActiveWidgetIndex);

/** 
 * A widget switcher with a few extra bells a whistles.
 * - Automatically activates and deactivates any contained UCommonActivatableWidgets
 * - Has built-in transition animations for switching between active indices
 */
UCLASS()
class UCommonWidgetSwitcher : public UWidgetSwitcher
{
	GENERATED_UCLASS_BODY()

public:
	virtual void ReleaseSlateResources(bool bReleaseChildren) override;
	virtual void SetActiveWidgetIndex(int32 Index) override;
	virtual void SetActiveWidget(UWidget* Widget) override;

	/** Fires when the currently active widget is deactivated */
	UPROPERTY(BlueprintAssignable, Category = "Common Widget Switcher")
	FOnActiveWidgetDeactivated OnActiveWidgetDeactivated;

	UPROPERTY(BlueprintAssignable, Category = "Common Widget Switcher")
	FOnActiveWidgetChanged OnActiveWidgetChanged;

	/**
	 *	Helper to activate an Activatable Panel if it is the active
	 *	widget on the widget switcher 
	 */
	UFUNCTION(BlueprintCallable, Category = "Common Widget Switcher")
	void ActivateActiveWidget();

	/**
	 *	Helper to deactivate an Activatable Panel if it is the active
	 *	widget on the widget switcher
	 */
	UFUNCTION(BlueprintCallable, Category = "Common Widget Switcher")
	void DeactivateActiveWidget();

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;

	/** The type of transition to play between widgets */
	UPROPERTY(EditAnywhere, Category = "Transition")
	ECommonSwitcherTransition TransitionType;

	/** The curve function type to apply to the transition animation */
	UPROPERTY(EditAnywhere, Category = "Transition")
	ETransitionCurve TransitionCurveType;

	/** The total duration of a single transition between widgets */
	UPROPERTY(EditAnywhere, Category = "Transition")
	float TransitionDuration;

	FOnWidgetActivationChangedDynamic ActiveWidgetDeactivatedDelegate;
	TSharedPtr<SCommonAnimatedSwitcher> MyAnimatedSwitcher;

private:
	UFUNCTION() void HandleActiveWidgetDeactivated(UCommonActivatablePanel* DeactivatedPanel);
};
