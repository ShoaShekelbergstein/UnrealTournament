// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#include "CommonUIPrivatePCH.h"
#include "CommonButtonGroup.h"

UCommonButtonGroup::UCommonButtonGroup()
	: bSelectionRequired(false)
	, SelectedButtonIndex(INDEX_NONE)
{
}

void UCommonButtonGroup::SetSelectionRequired(bool bRequireSelection)
{
	if (bSelectionRequired != bRequireSelection)
	{
		bSelectionRequired = bRequireSelection;
		
		if (bSelectionRequired && 
			Buttons.Num() > 0 && 
			SelectedButtonIndex < 0)
		{
			// Selection is now required and nothing is selected, so select the first button
			SelectButtonAtIndex(0);
		}
	}
}

void UCommonButtonGroup::DeselectAll()
{
	ensureMsgf(!bSelectionRequired, TEXT("BaseButton Groups that require selection should not deselect all."));

	for (auto& Button : Buttons)
	{
		if (Button.IsValid() && Button->GetSelected())
		{
			Button->SetSelectedInternal(false);
		}
	}

	SelectedButtonIndex = INDEX_NONE;
	OnSelectionCleared.Broadcast();
}

void UCommonButtonGroup::SelectNextButton(bool bAllowWrap /*= true*/)
{
	SelectNextButtonRecursive(SelectedButtonIndex, bAllowWrap);
}

void UCommonButtonGroup::SelectPreviousButton(bool bAllowWrap /*= true*/)
{
	SelectPrevButtonRecursive(SelectedButtonIndex, bAllowWrap);
}

void UCommonButtonGroup::SelectButtonAtIndex(int32 ButtonIndex)
{
	if (ButtonIndex < 0 || ButtonIndex >= Buttons.Num())
	{
		DeselectAll();
	}
	else if (ButtonIndex != SelectedButtonIndex)
	{
		UCommonButton* ButtonToSelect = GetButtonAtIndex(ButtonIndex);
		if (ButtonToSelect && !ButtonToSelect->GetSelected() && ButtonToSelect->GetIsEnabled())
		{
			SelectedButtonIndex = ButtonIndex;
			ButtonToSelect->SetSelectedInternal(true);
		}
	}
}

int32 UCommonButtonGroup::GetSelectedButtonIndex()
{
	for (int32 ButtonIndex = 0; ButtonIndex < Buttons.Num(); ButtonIndex++)
	{
		UCommonButton* Button = GetButtonAtIndex(ButtonIndex);
		if (Button && Button->GetSelected())
		{
			return ButtonIndex;
		}
	}

	return INDEX_NONE;
}

void UCommonButtonGroup::ForEach(TFunctionRef<void(UCommonButton&, int32)> Functor)
{
	for (int32 ButtonIndex = 0; ButtonIndex < Buttons.Num(); ButtonIndex++)
	{
		UCommonButton* Button = GetButtonAtIndex(ButtonIndex);
		if (Button)
		{
			Functor(*Button, ButtonIndex);
		}
		else
		{
			// We're at a valid button index, but no button was returned
			// Therefore the button reference was stale and removed - so back up the index accordingly
			--ButtonIndex;
		}
	}
}

void UCommonButtonGroup::OnWidgetAdded(UWidget* NewWidget)
{
	if (UCommonButton* Button = Cast<UCommonButton>(NewWidget))
	{
		Button->OnSelectedChanged.AddUniqueDynamic(this, &UCommonButtonGroup::OnSelectionStateChanged);

		Buttons.Emplace(Button);

		// If selection in the group is required and this is the first button added, make sure it's selected (quietly)
		if (bSelectionRequired && Buttons.Num() == 1 && !Button->GetSelected())
		{
			Button->SetSelectedInternal(true, false);
		}
	}
}

void UCommonButtonGroup::OnWidgetRemoved( UWidget* OldWidget )
{
	if (UCommonButton* Button = Cast<UCommonButton>(OldWidget))
	{
		Button->OnSelectedChanged.RemoveDynamic(this, &UCommonButtonGroup::OnSelectionStateChanged);

		Buttons.RemoveAll( [Button]( TWeakObjectPtr<UCommonButton> Entry ) { return Entry == Button || !Entry.IsValid(); } );
	}
}

void UCommonButtonGroup::OnRemoveAll()
{
	while ( Buttons.Num() > 0 )
	{
		if (UCommonButton* Button = Buttons.Pop().Get())
		{
			RemoveWidget(Button);
		}
	}
}

void UCommonButtonGroup::OnSelectionStateChanged( UCommonButton* BaseButton, bool bIsSelected )
{
	if (bIsSelected)
	{
		SelectedButtonIndex = INDEX_NONE;
		for (int32 ButtonIndex = 0; ButtonIndex < Buttons.Num(); ButtonIndex++)
		{
			UCommonButton* Button = GetButtonAtIndex(ButtonIndex);
			if (Button)
			{
				if (Button == BaseButton)
				{
					SelectedButtonIndex = ButtonIndex;
				}
				else if (Button->GetSelected())
				{
					// Make sure any other selected buttons are deselected
					Button->SetSelectedInternal(false);
				}
			}
			else
			{
				// We're at a valid button index, but no button was returned
				// Therefore the button reference was stale and removed - so back up the index accordingly
				--ButtonIndex;
			}
		}

		OnSelectedButtonChanged.Broadcast(BaseButton, SelectedButtonIndex);
	}
	else
	{
		for ( auto& WeakButton : Buttons )
		{
			if ( WeakButton.IsValid() && WeakButton->GetSelected() )
			{
				return; // early out here and prevent the delegate below from being triggered
			}
		}

		OnSelectionCleared.Broadcast();
	}
}

void UCommonButtonGroup::SelectNextButtonRecursive(int32 SelectionIndex, bool bAllowWrap)
{
	if (SelectionIndex < (Buttons.Num() - 1))
	{
		++SelectionIndex;
	}
	else if (bAllowWrap)
	{
		SelectionIndex = 0;
	}

	if (Buttons.IsValidIndex(SelectionIndex))
	{
		UCommonButton* NextButton = GetButtonAtIndex(SelectionIndex);
		if (NextButton && NextButton->IsInteractionEnabled())
		{
			SelectButtonAtIndex(SelectionIndex);
		}
		else
		{
			SelectNextButtonRecursive(SelectionIndex, bAllowWrap);
		}
	}
}

void UCommonButtonGroup::SelectPrevButtonRecursive(int32 SelectionIndex, bool bAllowWrap)
{
	if (SelectionIndex > 0)
	{
		--SelectionIndex;
	}
	else if (bAllowWrap)
	{
		SelectionIndex = Buttons.Num() - 1;
	}

	if (Buttons.IsValidIndex(SelectionIndex))
	{
		UCommonButton* PrevButton = GetButtonAtIndex(SelectionIndex);
		if (PrevButton && PrevButton->IsInteractionEnabled())
		{
			SelectButtonAtIndex(SelectionIndex);
		}
		else
		{
			SelectPrevButtonRecursive(SelectionIndex, bAllowWrap);
		}
	}
}

UCommonButton* UCommonButtonGroup::GetButtonAtIndex(int32 Index)
{
	if (Buttons.IsValidIndex(Index))
	{
		if (Buttons[Index].IsValid())
		{
			return Buttons[Index].Get();
		}
		else
		{
			// Remove the stale button
			Buttons.RemoveAt(Index);
		}
	}

	return nullptr;
}
