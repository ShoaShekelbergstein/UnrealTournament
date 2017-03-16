// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CommonListItem.h"
#include "CommonObjectListItem.generated.h"

/** Implement for list items that represent UObject data */
UINTERFACE()
class COMMONUI_API UCommonObjectListItem : public UCommonListItem
{
	GENERATED_UINTERFACE_BODY()
};

class COMMONUI_API ICommonObjectListItem : public ICommonListItem
{
	GENERATED_IINTERFACE_BODY()

public:
	UFUNCTION( BlueprintNativeEvent, BlueprintCallable, Category = ObjectListItem )
	UObject* GetData() const;

	UFUNCTION( BlueprintNativeEvent, BlueprintCallable, Category = ObjectListItem )
	void SetData( UObject* InData );

	UFUNCTION( BlueprintNativeEvent, BlueprintCallable, Category = ObjectListItem )
	void Reset();
};


// for ease of copy paste

/*

// ICommonObjectListItem interface
virtual UObject* GetData_Implementation() const override;
virtual void SetData_Implementation( UObject* InData ) override;
virtual void Reset_Implementation() override;
// ~ICommonObjectListItem

// ICommonListItem interface
virtual void SetSelected_Implementation( bool bInSelected ) override {}
virtual void SetIndexInList_Implementation( int32 InIndexInList ) override {}
virtual bool IsItemExpanded_Implementation() const override { return false; }
virtual void ToggleExpansion_Implementation() override {}
virtual int32 GetIndentLevel_Implementation() const override { return INDEX_NONE; }
virtual int32 DoesItemHaveChildren_Implementation() const override { return INDEX_NONE; }
virtual ESelectionMode::Type GetSelectionMode_Implementation() const override { return ESelectionMode::None; }
virtual void Private_OnExpanderArrowShiftClicked_Implementation() override {}
virtual void RegisterOnClicked_Implementation( const FOnItemClicked& Callback ) override {}
// ~ICommonListItem

*/