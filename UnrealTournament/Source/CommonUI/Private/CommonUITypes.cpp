// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#include "CommonUIPrivatePCH.h"
#include "CommonUITypes.h"

void FCommonInputActionData::OnPostDataImport(const UDataTable* InDataTable, const FName InRowName, TArray<FString>& OutCollectedImportProblems)
{
#if WITH_EDITOR
	const FString LocKey = FString::Printf(TEXT("%s_%s"), *InRowName.ToString(), GET_MEMBER_NAME_STRING_CHECKED(FCommonInputActionData, DisplayName));
	DisplayName = FText::ChangeKey(InDataTable ? InDataTable->GetName() : TEXT(""), LocKey, DisplayName);
#endif // WITH_EDITOR
}

UUserWidget* CommonUIUtils::GetOwningUserWidget(const UWidget* Widget)
{
	UObject* Outer = Widget ? Widget->GetOuter() : nullptr;
	UUserWidget* UserWidget = Outer ? Cast<UUserWidget>(Outer->GetOuter()) : nullptr;
	return UserWidget;
}

const FCommonInputActionData* GetInputActionData(const FDataTableRowHandle& InputActionRowHandle)
{
	return InputActionRowHandle.GetRow<FCommonInputActionData>(TEXT("UCommonActivatableManager::GetInputActionData couldn't find the row passed in, check data table if its missing."));
}

const FCommonInputVirtualKeyData* GetInputVirtualKeyData(const FDataTableRowHandle& InputKeyRowHandle)
{
	return InputKeyRowHandle.GetRow<FCommonInputVirtualKeyData>(TEXT("UCommonActivatableManager::GetInputKeyData couldn't find the row passed in, check data table if its missing."));
}

bool IsKeyBoundToInputActionData(const FKey& Key, const FDataTableRowHandle& InputActionDataRow)
{
	const FCommonInputActionData* InputActionData = GetInputActionData(InputActionDataRow);
	if (ensure(InputActionData))
	{
		const FCommonInputVirtualKeyData* VirtualKeyData = GetInputVirtualKeyData(InputActionData->VirtualKeyData);
		if (ensure(VirtualKeyData))
		{
			for (const FCommonInputKeyData& InputKeyData : VirtualKeyData->Platforms)
			{
				if (InputKeyData.InputKey.IsValid() && Key == InputKeyData.InputKey)
				{
					return true;
				}
			}
		}
	}
	
	return false;
}

FSlateBrush GetInputActionIcon(const FDataTableRowHandle& InputActionRowHandle, ECommonInputType InputType)
{
	if (ensure(InputType != ECommonInputType::Count) && !InputActionRowHandle.IsNull())
	{
		const FCommonInputActionData* InputActionData = GetInputActionData(InputActionRowHandle);
		if (ensure(InputActionData))
		{
			const FCommonInputVirtualKeyData* VirtualKeyData = GetInputVirtualKeyData(InputActionData->VirtualKeyData);
			if (ensure(VirtualKeyData))
			{
				return VirtualKeyData->Platforms[static_cast<uint8>(InputType)].InputIcon;
			}
		}
	}
	return FSlateBrush();
}
