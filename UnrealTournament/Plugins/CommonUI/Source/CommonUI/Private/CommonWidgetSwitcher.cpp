// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#include "CommonUIPrivatePCH.h"
#include "CommonWidgetSwitcher.h"
#include "CommonInputManager.h"
#include "CommonUIContext.h"
#include "Components/WidgetSwitcherSlot.h"
#include "Animation/CurveSequence.h"

//////////////////////////////////////////////////////////////////////////
// SCommonAnimatedSwitcher
//////////////////////////////////////////////////////////////////////////

class SCommonAnimatedSwitcher : public SWidgetSwitcher
{
public:
	SLATE_BEGIN_ARGS(SCommonAnimatedSwitcher)
		: _InitialIndex(0)
		, _TransitionType(ECommonSwitcherTransition::FadeOnly)
		, _TransitionCurveType(ECurveEaseFunction::CubicInOut)
		, _TransitionDuration(0.4f)
		{
			_Visibility = EVisibility::SelfHitTestInvisible;
		}
		
		SLATE_ARGUMENT(int32, InitialIndex)

		SLATE_ARGUMENT(ECommonSwitcherTransition, TransitionType)
		SLATE_ARGUMENT(ECurveEaseFunction::Type, TransitionCurveType)
		SLATE_ARGUMENT(float, TransitionDuration)

	SLATE_END_ARGS()

public:
	void Construct(const FArguments& InArgs)
	{
		SWidgetSwitcher::Construct(SWidgetSwitcher::FArguments().WidgetIndex(InArgs._InitialIndex));
		
		bTransitioningOut = false;
		PendingActiveIndex = 0;

		TransitionType = InArgs._TransitionType;

		TransitionSequence.AddCurve(0.f, InArgs._TransitionDuration * 0.5f, InArgs._TransitionCurveType);
	}

	virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyClippingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override
	{
		FWidgetStyle CompoundedWidgetStyle = FWidgetStyle(InWidgetStyle);
		
		// Set the alpha during transitions
		if (TransitionSequence.IsPlaying())
		{
			float Alpha = TransitionSequence.GetLerp();

			if ((bTransitioningOut && !TransitionSequence.IsInReverse()) ||
				(!bTransitioningOut && TransitionSequence.IsInReverse()))
			{
				Alpha = 1 - Alpha;
			}
			
			CompoundedWidgetStyle.BlendColorAndOpacityTint(FLinearColor(1.f, 1.f, 1.f, Alpha));
		}
		
		return SWidgetSwitcher::OnPaint(Args, AllottedGeometry, MyClippingRect, OutDrawElements, LayerId, CompoundedWidgetStyle, bParentEnabled);
	}

	void TransitionToIndex(int32 NewWidgetIndex)
	{
		if (TransitionSequence.GetCurve(0).DurationSeconds <= 0.f)
		{
			// Transition time set to 0 - just set it instantly
			SetActiveWidgetIndex(NewWidgetIndex);
			return;
		}
		
		// Cache the index we want to reach
		PendingActiveIndex = NewWidgetIndex;

		const int32 CurrentIndex = GetActiveWidgetIndex();
		const bool bNewGoalHigher = NewWidgetIndex > CurrentIndex;
		const bool bNewGoalLower = NewWidgetIndex < CurrentIndex;

		if (TransitionSequence.IsPlaying())
		{
			// Already a transition in progress - see if we need to reverse it
			const bool bNeedsReverse = (TransitionSequence.IsInReverse() && bNewGoalHigher) ||		// Currently headed to a lower index, now need to go to a higher one
										(!TransitionSequence.IsInReverse() && bNewGoalLower) ||		// Currently headed to a higher index, now need to go to a lower one
										(bTransitioningOut && NewWidgetIndex == CurrentIndex);			// Return to the index we're just now leaving
			if (bNeedsReverse)
			{
				bTransitioningOut = !bTransitioningOut;
				TransitionSequence.Reverse();
			}
		}
		else if (bNewGoalHigher || bNewGoalLower)
		{
			if (bNewGoalHigher)
			{
				TransitionSequence.Play(AsShared());
			}
			else
			{
				TransitionSequence.PlayReverse(AsShared());
			}

			SetVisibility(EVisibility::HitTestInvisible);
			bTransitioningOut = true;
			RegisterActiveTimer(0.f, FWidgetActiveTimerDelegate::CreateSP(this, &SCommonAnimatedSwitcher::UpdateTransition));
		}
	}

private:
	EActiveTimerReturnType UpdateTransition(double InCurrentTime, float InDeltaTime)
	{
		if (TransitionType == ECommonSwitcherTransition::Zoom)
		{
			static const float MaxScaleModifier = 0.25f;
			
			SetRenderTransform(FSlateRenderTransform(1 + (MaxScaleModifier * GetTransitionProgress())));
		}
		else if (TransitionType != ECommonSwitcherTransition::FadeOnly)
		{
			static const float MaxTranslation = 200.f;

			const float Offset = MaxTranslation * GetTransitionProgress();
			
			FVector2D Translation = FVector2D::ZeroVector;
			if (TransitionType == ECommonSwitcherTransition::Horizontal)
			{
				Translation.X = -Offset;
			}
			else
			{
				Translation.Y = Offset;
			}

			SetRenderTransform(Translation);
		}

		if (!TransitionSequence.IsPlaying() && GetActiveWidgetIndex() != PendingActiveIndex)
		{
			if (bTransitioningOut)
			{
				// Finished transitioning out - update the active index and play again from the start
				SetActiveWidgetIndex(PendingActiveIndex);
			}

			bTransitioningOut = !bTransitioningOut;

			if (TransitionSequence.IsInReverse())
			{
				TransitionSequence.PlayReverse(AsShared());
			}
			else
			{
				TransitionSequence.Play(AsShared());
			}
		}

		// If the sequence still isn't playing, the transition is complete
		if (!TransitionSequence.IsPlaying())
		{
			SetVisibility(EVisibility::SelfHitTestInvisible);
			return EActiveTimerReturnType::Stop;
		}
		
		return EActiveTimerReturnType::Continue;
	}

	float GetTransitionProgress() const 
	{ 
		float Progress = TransitionSequence.GetLerp();

		if ((bTransitioningOut && TransitionSequence.IsInReverse()) ||
			(!bTransitioningOut && TransitionSequence.IsForward()))
		{
			Progress += -1.f;
		}
		
		return Progress;
	}

	/** The type of transition anim to play */
	ECommonSwitcherTransition TransitionType;

	// Anim sequence for the transition; plays twice per transition
	FCurveSequence TransitionSequence;

	/** The pending active widget index, set when the initial transition out completes */
	int32 PendingActiveIndex;

	/** If true, we are transitioning content out and need to play the sequence again to transition it in */
	bool bTransitioningOut;
};

//////////////////////////////////////////////////////////////////////////
// UCommonActivatableWidgetSwitcher
//////////////////////////////////////////////////////////////////////////

UCommonWidgetSwitcher::UCommonWidgetSwitcher(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, TransitionType(ECommonSwitcherTransition::FadeOnly)
	, TransitionCurveType(ETransitionCurve::CubicInOut)
	, TransitionDuration(0.4f)
{
}

void UCommonWidgetSwitcher::ReleaseSlateResources(bool bReleaseChildren)
{
	Super::ReleaseSlateResources(bReleaseChildren);

	MyAnimatedSwitcher.Reset();
}

void UCommonWidgetSwitcher::SetActiveWidgetIndex(int32 Index)
{
#if WITH_EDITOR
	if (IsDesignTime())
	{
		Super::SetActiveWidgetIndex(Index);
		return;
	}
#endif

	// Deactivate the previously displayed child
	if (UCommonActivatablePanel* WidgetToDeactivate = Cast<UCommonActivatablePanel>(GetWidgetAtIndex(ActiveWidgetIndex)))
	{
		WidgetToDeactivate->OnWidgetDeactivated.RemoveAll(this);
		ULocalPlayer* LocalPlayer = WidgetToDeactivate->GetOwningLocalPlayer();
		UCommonUIContext* CommonUIContext = Cast<UCommonUIContext>(UBlueprintContextLibrary::GetContext(LocalPlayer, UCommonUIContext::StaticClass()));
		UCommonInputManager* CommonInputManager = CommonUIContext ? CommonUIContext->GetInputManager() :nullptr;
		if (CommonInputManager)
		{
			CommonInputManager->PopActivatablePanel(WidgetToDeactivate);
		}
	}

	ActiveWidgetIndex = Index;
	if (MyAnimatedSwitcher.IsValid())
	{
		// Ensure the index is clamped to a valid range.
		int32 SafeIndex = FMath::Clamp(ActiveWidgetIndex, 0, FMath::Max(0, Slots.Num() - 1));
		MyAnimatedSwitcher->TransitionToIndex(SafeIndex);
	}

	// Activate the newly displayed child
	if (UCommonActivatablePanel* WidgetToActivate = Cast<UCommonActivatablePanel>(GetWidgetAtIndex(ActiveWidgetIndex)))
	{
		ULocalPlayer* LocalPlayer = WidgetToActivate->GetOwningLocalPlayer();
		UCommonUIContext* CommonUIContext = Cast<UCommonUIContext>(UBlueprintContextLibrary::GetContext(LocalPlayer, UCommonUIContext::StaticClass()));
		UCommonInputManager* CommonInputManager = CommonUIContext ? CommonUIContext->GetInputManager() : nullptr;
		if (CommonInputManager)
		{
			CommonInputManager->PushActivatablePanel(WidgetToActivate);
		}

		WidgetToActivate->OnWidgetDeactivated.RemoveAll(this);
		WidgetToActivate->OnWidgetDeactivated.AddDynamic(this, &UCommonWidgetSwitcher::HandleActiveWidgetDeactivated);
	}

	if (Slots.IsValidIndex(Index))
	{
		OnActiveWidgetChanged.Broadcast(GetWidgetAtIndex(Index), Index);
	}
}

void UCommonWidgetSwitcher::SetActiveWidget(UWidget* Widget)
{
#if WITH_EDITOR
	if (IsDesignTime())
	{
		Super::SetActiveWidget(Widget);
		return;
	}
#endif

	const int32 ChildIndex = GetChildIndex(Widget);
	if (ChildIndex >= 0 && ChildIndex < Slots.Num() && ChildIndex != ActiveWidgetIndex)
	{
		// Deactivate the previously displayed child
		if (UCommonActivatablePanel* WidgetToDeactivate = Cast<UCommonActivatablePanel>(GetWidgetAtIndex(ActiveWidgetIndex)))
		{
			WidgetToDeactivate->OnWidgetDeactivated.RemoveAll(this);
			ULocalPlayer* LocalPlayer = WidgetToDeactivate->GetOwningLocalPlayer();
			UCommonUIContext* CommonUIContext = Cast<UCommonUIContext>(UBlueprintContextLibrary::GetContext(LocalPlayer, UCommonUIContext::StaticClass()));
			UCommonInputManager* CommonInputManager = CommonUIContext ? CommonUIContext->GetInputManager() : nullptr;
			if (CommonInputManager)
			{
				CommonInputManager->PopActivatablePanel(WidgetToDeactivate);
			}
		}

		ActiveWidgetIndex = GetChildIndex(Widget);
		if (MyAnimatedSwitcher.IsValid())
		{
			// Ensure the index is clamped to a valid range.
			int32 SafeIndex = FMath::Clamp(ActiveWidgetIndex, 0, FMath::Max(0, Slots.Num() - 1));
			MyAnimatedSwitcher->TransitionToIndex(SafeIndex);
		}

		// Activate the newly displayed child
		if (UCommonActivatablePanel* WidgetToActivate = Cast<UCommonActivatablePanel>(Widget))
		{
			ULocalPlayer* LocalPlayer = WidgetToActivate->GetOwningLocalPlayer();
			UCommonUIContext* CommonUIContext = Cast<UCommonUIContext>(UBlueprintContextLibrary::GetContext(LocalPlayer, UCommonUIContext::StaticClass()));
			UCommonInputManager* CommonInputManager = CommonUIContext ? CommonUIContext->GetInputManager() : nullptr;
			if (CommonInputManager)
			{
				CommonInputManager->PushActivatablePanel(WidgetToActivate);
			}

			WidgetToActivate->OnWidgetDeactivated.RemoveAll(this);
			WidgetToActivate->OnWidgetDeactivated.AddDynamic(this, &UCommonWidgetSwitcher::HandleActiveWidgetDeactivated);
		}

		OnActiveWidgetChanged.Broadcast(Widget, ChildIndex);
	}
}

TSharedRef<SWidget> UCommonWidgetSwitcher::RebuildWidget()
{
	MyWidgetSwitcher = MyAnimatedSwitcher = SNew(SCommonAnimatedSwitcher)
		.InitialIndex(ActiveWidgetIndex)
		.TransitionCurveType(TransitionCurveToCurveEaseFunction(TransitionCurveType))
		.TransitionDuration(TransitionDuration)
		.TransitionType(TransitionType);

	for (UPanelSlot* CurrentSlot : Slots)
	{
		if (UWidgetSwitcherSlot* TypedSlot = Cast<UWidgetSwitcherSlot>(CurrentSlot))
		{
			TypedSlot->Parent = this;
			TypedSlot->BuildSlot(MyWidgetSwitcher.ToSharedRef());
		}
	}

	return BuildDesignTimeWidget(MyAnimatedSwitcher.ToSharedRef());
}

void UCommonWidgetSwitcher::HandleActiveWidgetDeactivated(UCommonActivatablePanel* DeactivatedPanel)
{
	if (DeactivatedPanel)
	{
		OnActiveWidgetDeactivated.Broadcast(DeactivatedPanel);
	}
}

void UCommonWidgetSwitcher::ActivateActiveWidget()
{
	if (UCommonActivatablePanel* WidgetToActivate = Cast<UCommonActivatablePanel>(GetWidgetAtIndex(ActiveWidgetIndex)))
	{
		ULocalPlayer* LocalPlayer = WidgetToActivate->GetOwningLocalPlayer();
		UCommonUIContext* CommonUIContext = Cast<UCommonUIContext>(UBlueprintContextLibrary::GetContext(LocalPlayer, UCommonUIContext::StaticClass()));
		UCommonInputManager* CommonInputManager = CommonUIContext ? CommonUIContext->GetInputManager() : nullptr;
		CommonInputManager->PushActivatablePanel(WidgetToActivate);
	}
}

void UCommonWidgetSwitcher::DeactivateActiveWidget()
{
	if (UCommonActivatablePanel* WidgetToDeactivate = Cast<UCommonActivatablePanel>(GetWidgetAtIndex(ActiveWidgetIndex)))
	{
		WidgetToDeactivate->PopPanel();
	}
}