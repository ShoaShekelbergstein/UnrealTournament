// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#include "CommonUIPrivatePCH.h"
#include "CommonButton.h"

#include "CommonTextBlock.h"
#include "CommonUIContext.h"
#include "CommonActivatablePanel.h"
#include "CommonInputManager.h"
#include "CommonGlobalInputHandler.h"
#include "CommonUISettings.h"
#include "CommonButton.h"
#include "Components/ButtonSlot.h"
#include "Blueprint/WidgetTree.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBox.h"
#include "Framework/Application/SlateApplication.h"

//////////////////////////////////////////////////////////////////////////
// UCommonButtonStyle
//////////////////////////////////////////////////////////////////////////

void UCommonButtonStyle::GetButtonPadding(FMargin& OutButtonPadding) const
{
	OutButtonPadding = ButtonPadding;
}

void UCommonButtonStyle::GetCustomPadding(FMargin& OutCustomPadding) const
{
	OutCustomPadding = CustomPadding;
}

UCommonTextStyle* UCommonButtonStyle::GetNormalTextStyle() const
{
	if (NormalTextStyle)
	{
		if (UCommonTextStyle* TextStyle = Cast<UCommonTextStyle>(NormalTextStyle->ClassDefaultObject))
		{
			return TextStyle;
		}
	}
	return nullptr;
}

UCommonTextStyle* UCommonButtonStyle::GetSelectedTextStyle() const
{
	if (SelectedTextStyle)
	{
		if (UCommonTextStyle* TextStyle = Cast<UCommonTextStyle>(SelectedTextStyle->ClassDefaultObject))
		{
			return TextStyle;
		}
	}
	return nullptr;
}

UCommonTextStyle* UCommonButtonStyle::GetDisabledTextStyle() const
{
	if (DisabledTextStyle)
	{
		if (UCommonTextStyle* TextStyle = Cast<UCommonTextStyle>(DisabledTextStyle->ClassDefaultObject))
		{
			return TextStyle;
		}
	}
	return nullptr;
}

void UCommonButtonStyle::GetMaterialBrush(FSlateBrush& Brush) const
{
	Brush = Material;
}

void UCommonButtonStyle::GetNormalBaseBrush(FSlateBrush& Brush) const
{
	Brush = NormalBase;
}

void UCommonButtonStyle::GetNormalHoveredBrush(FSlateBrush& Brush) const
{
	Brush = NormalHovered;
}

void UCommonButtonStyle::GetNormalPressedBrush(FSlateBrush& Brush) const
{
	Brush = NormalPressed;
}

void UCommonButtonStyle::GetSelectedBaseBrush(FSlateBrush& Brush) const
{
	Brush = SelectedBase;
}

void UCommonButtonStyle::GetSelectedHoveredBrush(FSlateBrush& Brush) const
{
	Brush = SelectedHovered;
}

void UCommonButtonStyle::GetSelectedPressedBrush(FSlateBrush& Brush) const
{
	Brush = SelectedPressed;
}

void UCommonButtonStyle::GetDisabledBrush(FSlateBrush& Brush) const
{
	Brush = Disabled;
}

//////////////////////////////////////////////////////////////////////////
// SCommonButton
//////////////////////////////////////////////////////////////////////////
/** 
 * Lets us disable clicking on a button without disabling hit-testing
 * Needed because NativeOnMouseEnter is not received by disabled widgets, 
 * but that also disables our anchored tooltips.
 */
class SCommonButton : public SButton
{
public:
	SLATE_BEGIN_ARGS(SCommonButton)
		: _Content()
		, _HAlign(HAlign_Fill)
		, _VAlign(VAlign_Fill)
		, _ClickMethod(EButtonClickMethod::DownAndUp)
		, _TouchMethod(EButtonTouchMethod::DownAndUp)
		, _PressMethod(EButtonPressMethod::DownAndUp)
		, _IsFocusable(true)
		, _IsInteractionEnabled(true)
	{}
		SLATE_DEFAULT_SLOT(FArguments, Content)
		SLATE_STYLE_ARGUMENT(FButtonStyle, ButtonStyle)
		SLATE_ARGUMENT(EHorizontalAlignment, HAlign)
		SLATE_ARGUMENT(EVerticalAlignment, VAlign)
		SLATE_EVENT(FOnClicked, OnClicked)
		SLATE_EVENT(FSimpleDelegate, OnPressed)
		SLATE_EVENT(FSimpleDelegate, OnReleased)
		SLATE_ARGUMENT(EButtonClickMethod::Type, ClickMethod)
		SLATE_ARGUMENT(EButtonTouchMethod::Type, TouchMethod)
		SLATE_ARGUMENT(EButtonPressMethod::Type, PressMethod)
		SLATE_ARGUMENT(bool, IsFocusable)

		/** Is interaction enabled? */
		SLATE_ARGUMENT(bool, IsInteractionEnabled)
		SLATE_END_ARGS()

	void Construct(const FArguments& InArgs)
	{
		SButton::Construct(SButton::FArguments()
			.ButtonStyle(InArgs._ButtonStyle)
			.HAlign(InArgs._HAlign)
			.VAlign(InArgs._VAlign)
			.ClickMethod(InArgs._ClickMethod)
			.TouchMethod(InArgs._TouchMethod)
			.PressMethod(InArgs._PressMethod)
			.OnClicked(InArgs._OnClicked)
			.OnPressed(InArgs._OnPressed)
			.OnReleased(InArgs._OnReleased)
			.IsFocusable(InArgs._IsFocusable)
			.Content()
			[
				InArgs._Content.Widget
			]);

		bIsInteractionEnabled = InArgs._IsInteractionEnabled;
	}

	virtual FReply OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override
	{
		return bIsInteractionEnabled ? SButton::OnMouseButtonDown(MyGeometry, MouseEvent) : FReply::Handled();
	}
	
	virtual FReply OnMouseButtonDoubleClick(const FGeometry& InMyGeometry, const FPointerEvent& InMouseEvent) override
	{
		return OnMouseButtonDown(InMyGeometry, InMouseEvent);
	}

	virtual FReply OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override
	{
		FReply Reply = FReply::Handled();
		if (!bIsInteractionEnabled)
		{
			if (HasMouseCapture())
			{
				// It's conceivable that interaction was disabled while this button had mouse capture
				// If that's the case, we want to release it (without acknowledging the click)
				Release();
				Reply.ReleaseMouseCapture();
			}
		}
		else
		{
			Reply = SButton::OnMouseButtonUp(MyGeometry, MouseEvent);
		}
		
		return Reply;
	}
	
	virtual bool IsHovered() const override
	{
		return bIsInteractionEnabled ? SButton::IsHovered() : false;
	}

	virtual bool IsPressed() const override
	{
		return bIsInteractionEnabled ? SButton::IsPressed() : false;
	}

	void SetIsInteractionEnabled(bool bInIsInteractionEnabled)
	{
		bIsInteractionEnabled = bInIsInteractionEnabled;
	}

private:

	/** True if clicking is disabled, to allow for things like double click */
	bool bIsInteractionEnabled;
};


//////////////////////////////////////////////////////////////////////////
// UCommonButton
//////////////////////////////////////////////////////////////////////////

UCommonButtonInternal::UCommonButtonInternal(const FObjectInitializer& Initializer)
	: Super(Initializer)
	, bInteractionEnabled(true)
{}

void UCommonButtonInternal::SetInteractionEnabled(bool bInIsInteractionEnabled)
{
	bInteractionEnabled = bInIsInteractionEnabled;
	if (MyCommonButton.IsValid())
	{
		MyCommonButton->SetIsInteractionEnabled(bInIsInteractionEnabled);
	}
}

bool UCommonButtonInternal::IsHovered() const
{
	if (MyCommonButton.IsValid())
	{
		return MyCommonButton->IsHovered();
	}
	return false;
}

bool UCommonButtonInternal::IsPressed() const
{
	if (MyCommonButton.IsValid())
	{
		return MyCommonButton->IsPressed();
	}
	return false;
}

void UCommonButtonInternal::SetMinDesiredHeight(int32 InMinHeight)
{
	MinHeight = InMinHeight;
	if (MyBox.IsValid())
	{
		MyBox->SetMinDesiredHeight(InMinHeight);
	}
}

void UCommonButtonInternal::SetMinDesiredWidth(int32 InMinWidth)
{
	MinWidth = InMinWidth;
	if (MyBox.IsValid())
	{
		MyBox->SetMinDesiredWidth(InMinWidth);
	}
}

TSharedRef<SWidget> UCommonButtonInternal::RebuildWidget()
{
	MyButton = MyCommonButton = SNew(SCommonButton)
		.OnClicked(BIND_UOBJECT_DELEGATE(FOnClicked, SlateHandleClicked))
		.OnPressed(BIND_UOBJECT_DELEGATE(FSimpleDelegate, SlateHandlePressed))
		.OnReleased(BIND_UOBJECT_DELEGATE(FSimpleDelegate, SlateHandleReleased))
		.ButtonStyle(&WidgetStyle)
		.ClickMethod(ClickMethod)
		.TouchMethod(TouchMethod)
		.IsFocusable(IsFocusable)
		.IsInteractionEnabled(bInteractionEnabled);

	MyBox = SNew(SBox)
		.MinDesiredWidth(MinWidth)
		.MinDesiredHeight(MinHeight)
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Fill)
		[
			MyCommonButton.ToSharedRef()
		];

	if (GetChildrenCount() > 0)
	{
		Cast<UButtonSlot>(GetContentSlot())->BuildSlot(MyCommonButton.ToSharedRef());
	}

	return MyBox.ToSharedRef();
}

void UCommonButtonInternal::ReleaseSlateResources(bool bReleaseChildren)
{
	Super::ReleaseSlateResources(bReleaseChildren);
	MyCommonButton.Reset();
	MyBox.Reset();
}

//////////////////////////////////////////////////////////////////////////
// UCommonButton
//////////////////////////////////////////////////////////////////////////

UCommonButton::UCommonButton(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, MinWidth(0)
	, MinHeight(0)
	, bApplyAlphaOnDisable(true)
	, bSelectable(false)
	, bToggleable(false)
	, bSelected(false)
	, bInteractionEnabled(true)
{
	Style = GetDefault<UCommonUISettings>()->DefaultButtonStyle;
}

void UCommonButton::SynchronizeProperties()
{
	Super::SynchronizeProperties();

	RefreshDimensions();
	BuildStyles();
}

bool UCommonButton::Initialize()
{
	const bool bInitializedThisCall = Super::Initialize();

	if (bInitializedThisCall)
	{
		UCommonButtonInternal* RootButtonRaw = WidgetTree->ConstructWidget<UCommonButtonInternal>(UCommonButtonInternal::StaticClass(), FName(TEXT("InternalRootButton")));
		RootButtonRaw->ClickMethod = ClickMethod;
		RootButtonRaw->IsFocusable = bSupportsKeyboardFocus_DEPRECATED;
		RootButtonRaw->SetInteractionEnabled(bInteractionEnabled);
		RootButton = RootButtonRaw;

		if (WidgetTree->RootWidget)
		{
			UButtonSlot* NewSlot = Cast<UButtonSlot>(RootButtonRaw->AddChild(WidgetTree->RootWidget));
			NewSlot->SetPadding(FMargin());
			NewSlot->SetHorizontalAlignment(EHorizontalAlignment::HAlign_Fill);
			NewSlot->SetVerticalAlignment(EVerticalAlignment::VAlign_Fill);
			WidgetTree->RootWidget = RootButtonRaw;

			RootButton->OnClicked.AddUniqueDynamic(this, &UCommonButton::HandleButtonClicked);
		}

		BuildStyles();
	}

	return bInitializedThisCall;
}

void UCommonButton::NativeConstruct()
{
	Super::NativeConstruct();

	BindTriggeringInputActionToClick();
	
	BuildStyles();
}

void UCommonButton::NativeDestruct()
{
	UnbindTriggeringInputActionToClick();

	Super::NativeDestruct();
}

void UCommonButton::SetIsEnabled(bool bInIsEnabled)
{
	Super::SetIsEnabled(bInIsEnabled);

	if (UButton* ButtonPtr = RootButton.Get())
	{
		ButtonPtr->SetIsEnabled(GetIsEnabled());
	}
}

bool UCommonButton::NativeIsInteractable() const
{
	// If it's enabled, it's "interactable" from a UMG perspective. 
	// For now this is how we generate friction on the analog cursor, which we still want for disabled buttons since they have tooltips.
	return GetIsEnabled();
}

void UCommonButton::BindTriggeringInputActionToClick()
{
	if (TriggeringInputAction.IsNull() || !TriggeredInputAction.IsNull())
	{
		return;
	}
	
	UCommonUIContext* CommonUIContext = GetContext<UCommonUIContext>();
	check(CommonUIContext);
	UCommonInputManager* CommonInputManager = CommonUIContext->GetInputManager();
	check(CommonInputManager);
			
	if (!TriggeringInputAction.IsNull())
	{		
		UCommonGlobalInputHandler* CommonGlobalInputHandler = CommonInputManager->GetGlobalInputHandler();
		check(CommonGlobalInputHandler);

		FCommonActionCommited CommitedEvent;
		CommitedEvent.BindDynamic(this, &ThisClass::HandleTriggeringActionCommited);

		FCommonActionProgressSingle ProgressEvent;
		ProgressEvent.BindDynamic(this, &ThisClass::OnActionProgress);

		FCommonActionCompleteSingle CompleteEvent;
		CompleteEvent.BindDynamic(this, &ThisClass::OnActionComplete);
		
		CommonGlobalInputHandler->RegisterInputAction(this, TriggeringInputAction, CommitedEvent, CompleteEvent, ProgressEvent);
	}
}

void UCommonButton::UnbindTriggeringInputActionToClick()
{
	if (TriggeringInputAction.IsNull() || !TriggeredInputAction.IsNull())
	{
		return;
	}

	UCommonUIContext* CommonUIContext = GetContext<UCommonUIContext>();
	if (CommonUIContext)
	{
		UCommonInputManager* CommonInputManager = CommonUIContext->GetInputManager();
		check(CommonInputManager);

		if (!TriggeringInputAction.IsNull())
		{
			UCommonGlobalInputHandler* CommonGlobalInputHandler = CommonInputManager->GetGlobalInputHandler();
			check(CommonGlobalInputHandler);
			CommonGlobalInputHandler->UnregisterInputAction(this, TriggeringInputAction);
		}
	}	
}

void UCommonButton::HandleTriggeringActionCommited(bool& bPassthrough)
{
	if (IsInteractionEnabled())
	{
		HandleButtonClicked();
	}
}

void UCommonButton::EnableButton()
{
	if (!bInteractionEnabled)
	{
		bInteractionEnabled = true;

		// If this is a selected and not-toggleable button, don't enable root button interaction
		if (!GetSelected() || bToggleable)
		{
			RootButton->SetInteractionEnabled(true);
		}
		
		if (bApplyAlphaOnDisable)
		{
			FLinearColor ButtonColor = RootButton->ColorAndOpacity;
			ButtonColor.A = 1.f;
			RootButton->SetColorAndOpacity(ButtonColor);
		}

		NativeOnEnabled();
	}
}

void UCommonButton::DisableButton()
{
	if (bInteractionEnabled)
	{
		bInteractionEnabled = false;
		RootButton->SetInteractionEnabled(false);

		if (bApplyAlphaOnDisable)
		{
			FLinearColor ButtonColor = RootButton->ColorAndOpacity;
			ButtonColor.A = 0.5f;
			RootButton->SetColorAndOpacity(ButtonColor);
		}

		NativeOnDisabled();
	}
}

void UCommonButton::DisableButtonWithReason(const FText& DisabledReason)
{
	DisableButton();

	DisabledTooltipText = DisabledReason;
}

bool UCommonButton::IsInteractionEnabled() const
{
	return GetIsEnabled() && bInteractionEnabled;
}

bool UCommonButton::IsHovered() const
{
	return RootButton.IsValid() && RootButton->IsHovered();
}

bool UCommonButton::IsPressed() const
{
	return RootButton.IsValid() && RootButton->IsPressed();
}

void UCommonButton::SetIsSelectable(bool bInIsSelectable)
{
	if (bInIsSelectable != bSelectable)
	{
		bSelectable = bInIsSelectable;

		if (bSelected && !bInIsSelectable)
		{
			SetSelectedInternal(false);
		}
	}
}

void UCommonButton::SetIsToggleable(bool bInIsToggleable)
{
	bToggleable = bInIsToggleable;

	// Update interactability.
	if (!GetSelected() || bToggleable)
	{
		RootButton->SetInteractionEnabled(bInteractionEnabled);
	}
	else if (GetSelected() && !bToggleable)
	{
		RootButton->SetInteractionEnabled(false);
	}
}

void UCommonButton::SetIsSelected(bool InSelected, bool bGiveClickFeedback)
{
	if (bSelectable && bSelected != InSelected)
	{
		if (!InSelected && bToggleable)
		{
			SetSelectedInternal(false);
		}
		else if (InSelected)
		{
			// Only allow a sound if we weren't just clicked
			SetSelectedInternal(true, bGiveClickFeedback);
		}
	}
}

void UCommonButton::SetSelectedInternal(bool bInSelected, bool bAllowSound /*= true*/, bool bBroadcast /*= true*/)
{
	bool bDidChange = (bInSelected != bSelected);

	bSelected = bInSelected;

	SetButtonStyle();

	if (bSelected)
	{
		NativeOnSelected(bBroadcast);
		if (!bToggleable)
		{
			// If the button isn't toggleable, then disable interaction with the root button while selected
			// The prevents us getting unnecessary click noises and events
			RootButton->SetInteractionEnabled(false);
		}

		if (bAllowSound)
		{
			// Selection was not triggered by a button click, so play the click sound
			FSlateApplication::Get().PlaySound(NormalStyle.PressedSlateSound);
		}
	}
	else
	{
		// Once deselected, restore the root button interactivity to the desired state
		RootButton->SetInteractionEnabled(bInteractionEnabled);
		
		NativeOnDeselected(bBroadcast);
	}
}

void UCommonButton::RefreshDimensions()
{
	const UCommonButtonStyle* const StyleCDO = GetStyleCDO();
	RootButton->SetMinDesiredWidth(FMath::Max(MinWidth, StyleCDO ? StyleCDO->MinWidth : 0));
	RootButton->SetMinDesiredHeight(FMath::Max(MinHeight, StyleCDO ? StyleCDO->MinHeight : 0));
}

void UCommonButton::NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseEnter(InGeometry, InMouseEvent);

	if (IsInteractionEnabled())
	{
		NativeOnHovered();
	}
}

void UCommonButton::NativeOnMouseLeave(const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseLeave(InMouseEvent);
	
	if (IsInteractionEnabled())
	{
		NativeOnUnhovered();
	}
}

bool UCommonButton::GetSelected() const
{
	return bSelected;
}

void UCommonButton::ClearSelection()
{
	SetSelectedInternal( false, false );
}

void UCommonButton::SetStyle(TSubclassOf<UCommonButtonStyle> InStyle)
{
	if (InStyle && Style != InStyle)
	{
		Style = InStyle;
		BuildStyles();
	}
}

UCommonButtonStyle* UCommonButton::GetStyle() const
{
	return const_cast<UCommonButtonStyle*>(GetStyleCDO());
}

const UCommonButtonStyle* UCommonButton::GetStyleCDO() const
{
	if (Style)
	{
		if (const UCommonButtonStyle* CommonButtonStyle = Cast<UCommonButtonStyle>(Style->ClassDefaultObject))
		{
			return CommonButtonStyle;
		}
	}
	return nullptr;
}

void UCommonButton::GetCurrentButtonPadding(FMargin& OutButtonPadding) const
{
	if (const UCommonButtonStyle* CommonButtonStyle = GetStyleCDO())
	{
		CommonButtonStyle->GetButtonPadding( OutButtonPadding);
	}
}

void UCommonButton::GetCurrentCustomPadding(FMargin& OutCustomPadding) const
{
	if (const UCommonButtonStyle* CommonButtonStyle = GetStyleCDO())
	{
		CommonButtonStyle->GetCustomPadding(OutCustomPadding);
	}
}

UCommonTextStyle* UCommonButton::GetCurrentTextStyle() const
{
	if (const UCommonButtonStyle* CommonButtonStyle = GetStyleCDO())
	{
		if (GetIsEnabled())
		{
			if (bSelected)
			{
				return CommonButtonStyle->GetSelectedTextStyle();
			}
			else
			{
				return CommonButtonStyle->GetNormalTextStyle();
			}
		}
		else
		{
			return CommonButtonStyle->GetDisabledTextStyle();
		}
	}
	return nullptr;
}

TSubclassOf<UCommonTextStyle> UCommonButton::GetCurrentTextStyleClass() const
{
	if (UCommonTextStyle* CurrentTextStyle = GetCurrentTextStyle())
	{
		return CurrentTextStyle->GetClass();
	}
	return nullptr;
}

void UCommonButton::SetMinDimensions(int32 InMinWidth, int32 InMinHeight)
{
	MinWidth = InMinWidth;
	MinHeight = InMinHeight;

	RefreshDimensions();
}

void UCommonButton::SetTriggeredInputAction(const FDataTableRowHandle &InputActionRow, UCommonActivatablePanel* OldPanel)
{
	UCommonUIContext* CommonUIContext = GetContext<UCommonUIContext>();
	check(CommonUIContext);
	UCommonInputManager* CommonInputManager = CommonUIContext->GetInputManager();
	check(CommonInputManager);

	FCommonActionProgressSingle ProgressEvent;
	ProgressEvent.BindDynamic(this, &ThisClass::OnActionProgress);
	FCommonActionCompleteSingle CompleteEvent;
	CompleteEvent.BindDynamic(this, &ThisClass::OnActionComplete);
	
	bool ActionRemoved = OldPanel && OldPanel->StopListeningForExistingHeldAction(InputActionRow, CompleteEvent, ProgressEvent);
	if (!ActionRemoved)
	{
		CommonInputManager->StopListeningForExistingHeldAction(InputActionRow, CompleteEvent, ProgressEvent);
	}
	
	TriggeredInputAction = InputActionRow;

	CommonInputManager->StartListeningForExistingHeldAction(TriggeredInputAction, CompleteEvent, ProgressEvent);
	
	OnTriggeredInputActionChanged(TriggeredInputAction);
}

bool UCommonButton::GetInputAction(FDataTableRowHandle &InputActionRow) const
{
	bool bBothActionsSet = !TriggeringInputAction.IsNull() && !TriggeredInputAction.IsNull();
	bool bNoActionSet = TriggeringInputAction.IsNull() && TriggeredInputAction.IsNull();

	if (bBothActionsSet || bNoActionSet)
	{
		return false;
	}

	if (!TriggeringInputAction.IsNull())
	{
		InputActionRow = TriggeringInputAction;
		return true;
	}
	else
	{
		InputActionRow = TriggeredInputAction;
		return true;
	}
}

void UCommonButton::ExecuteTriggeredInput()
{
	if (!TriggeredInputAction.IsNull())
	{
		UCommonUIContext* CommonUIContext = GetContext<UCommonUIContext>();
		check(CommonUIContext);
		UCommonInputManager* CommonInputManager = CommonUIContext->GetInputManager();
		check(CommonInputManager);
		CommonInputManager->TriggerInputActionHandler(TriggeredInputAction);
	}
}

void UCommonButton::HandleButtonClicked()
{
	SetIsSelected(!bSelected, false);

	NativeOnClicked();

	ExecuteTriggeredInput();
}

void UCommonButton::NativeOnSelected(bool bBroadcast)
{
	OnSelected();

	if (OnSelectedChanged.IsBound() && bBroadcast )
	{
		OnSelectedChanged.Broadcast(this, true);
	}
	NativeOnCurrentTextStyleChanged();
}

void UCommonButton::NativeOnDeselected(bool bBroadcast)
{
	OnDeselected();

	if (OnSelectedChanged.IsBound() && bBroadcast)
	{
		OnSelectedChanged.Broadcast(this, false);
	}
	NativeOnCurrentTextStyleChanged();
}

void UCommonButton::NativeOnHovered()
{
	OnHovered();

	if (OnButtonHovered.IsBound())
	{
		OnButtonHovered.Broadcast(this);
	}
}

void UCommonButton::NativeOnUnhovered()
{
	OnUnhovered();

	if (OnButtonUnhovered.IsBound())
	{
		OnButtonUnhovered.Broadcast(this);
	}
}

void UCommonButton::NativeOnClicked()
{
	OnClicked();

	if (OnButtonClicked.IsBound())
	{
		OnButtonClicked.Broadcast(this);
	}
}

void UCommonButton::NativeOnEnabled()
{
	OnEnabled();
	NativeOnCurrentTextStyleChanged();
}

void UCommonButton::NativeOnDisabled()
{
	OnDisabled();
	NativeOnCurrentTextStyleChanged();
}

void UCommonButton::NativeOnCurrentTextStyleChanged()
{
	OnCurrentTextStyleChanged();
}

void UCommonButton::BuildStyles()
{
	if (const UCommonButtonStyle* CommonButtonStyle = GetStyleCDO())
	{
		const FMargin& ButtonPadding = CommonButtonStyle->ButtonPadding;
		const FSlateBrush& DisabledBrush = CommonButtonStyle->Disabled;

		NormalStyle.Normal = CommonButtonStyle->NormalBase;
		NormalStyle.Hovered = CommonButtonStyle->NormalHovered;
		NormalStyle.Pressed = CommonButtonStyle->NormalPressed;
		NormalStyle.Disabled = DisabledBrush;
		NormalStyle.NormalPadding = ButtonPadding;
		NormalStyle.PressedPadding = ButtonPadding;
		NormalStyle.PressedSlateSound = PressedSlateSoundOverride.GetResourceObject() ? PressedSlateSoundOverride : CommonButtonStyle->PressedSlateSound;
		NormalStyle.HoveredSlateSound = HoveredSlateSoundOverride.GetResourceObject() ? HoveredSlateSoundOverride : CommonButtonStyle->HoveredSlateSound;

		SelectedStyle.Normal = CommonButtonStyle->SelectedBase;
		SelectedStyle.Hovered = CommonButtonStyle->SelectedHovered;
		SelectedStyle.Pressed = CommonButtonStyle->SelectedPressed;
		SelectedStyle.Disabled = DisabledBrush;
		SelectedStyle.NormalPadding = ButtonPadding;
		SelectedStyle.PressedPadding = ButtonPadding;
		SelectedStyle.PressedSlateSound = NormalStyle.PressedSlateSound;
		SelectedStyle.HoveredSlateSound = NormalStyle.HoveredSlateSound;

		SetButtonStyle();
	}
}

void UCommonButton::SetButtonStyle()
{
	if (UButton* ButtonPtr = RootButton.Get())
	{
		ButtonPtr->SetStyle(bSelected ? SelectedStyle : NormalStyle);
	}
}