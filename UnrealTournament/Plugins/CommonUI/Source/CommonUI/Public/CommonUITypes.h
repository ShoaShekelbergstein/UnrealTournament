// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "IBlueprintContextModule.h"
#include "Engine/DataTable.h"
#include "Blueprint/UserWidget.h"
#include "CommonUITypes.generated.h"

UENUM(BlueprintType)
enum class ECommonInputType : uint8
{
	MouseAndKeyboard,
	XboxOneController,
	PS4Controller,
	Count
};

USTRUCT(BlueprintType)
struct FCommonInputKeyData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CommonInput")
	FKey InputKey;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CommonInput")
	FSlateBrush InputIcon;
};

USTRUCT(BlueprintType)
struct FCommonInputVirtualKeyData : public FTableRowBase
{
	GENERATED_BODY()

	/** Array defining the platforms specific keys
	 * MouseAndKeyboard  = 0
	 * XboxOneController = 1
	 * PS4Controller     = 2 
	 */
	UPROPERTY(EditAnywhere, Category = "CommonInput")
	FCommonInputKeyData Platforms[(uint8)ECommonInputType::Count];
};

USTRUCT(BlueprintType)
struct FCommonInputActionData : public FTableRowBase
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CommonInput")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CommonInput", meta = (RowType = CommonInputVirtualKeyData))
	FDataTableRowHandle VirtualKeyData;

	/** Enables hold time if true */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CommonInput")
	bool bActionRequiresHold;

	/** The hold time in seconds */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CommonInput", meta = (EditCondition = "bActionRequiresHold"))
	float HoldTime;

	virtual void OnPostDataImport(const UDataTable* InDataTable, const FName InRowName, TArray<FString>& OutCollectedImportProblems) override;
};

class CommonUIUtils
{
public:
	// Get owning UserWidget for the given widget
	COMMONUI_API static UUserWidget* GetOwningUserWidget(const UWidget* Widget);

	// Get Context of specified type from a UWidget
	template <class ContextType>
	static ContextType* GetContext(const UWidget* Widget)
	{
		const UUserWidget* UserWidget = Cast<const UUserWidget>(Widget);
		if (!UserWidget)
		{
			UserWidget = GetOwningUserWidget(Widget);
		}
		if (UserWidget)
		{
			return Cast<ContextType>(UBlueprintContextLibrary::GetContext(UserWidget->GetOwningLocalPlayer(), ContextType::StaticClass()));
		}
		return nullptr;
	}
};

const FCommonInputActionData* GetInputActionData(const FDataTableRowHandle& InputActionRowHandle);
const FCommonInputVirtualKeyData* GetInputVirtualKeyData(const FDataTableRowHandle& InputKeyRowHandle);
bool IsKeyBoundToInputActionData(const FKey& Key, const FDataTableRowHandle& InputActionDataRow);
FSlateBrush GetInputActionIcon(const FDataTableRowHandle& InputActionRowHandle, ECommonInputType InputType);

DECLARE_DYNAMIC_DELEGATE_OneParam(FOnItemClicked, UUserWidget*, Widget);
DECLARE_DYNAMIC_DELEGATE_TwoParams(FOnItemSelected, UUserWidget*, Widget, bool, Selected);
