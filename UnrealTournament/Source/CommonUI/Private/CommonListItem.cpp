// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#include "CommonUIPrivatePCH.h"
#include "CommonListItem.h"

UCommonListItem::UCommonListItem( const FObjectInitializer& Initializer )
    : Super( Initializer )
{
}

void ICommonListItem::SetSelected_Implementation( bool bSelected )
{
	// stub
}

void ICommonListItem::SetIndexInList_Implementation( int32 InIndexInList )
{
	// stub
}

bool ICommonListItem::IsItemExpanded_Implementation() const
{
	return false;
}

void ICommonListItem::ToggleExpansion_Implementation()
{
	// stub
}

int32 ICommonListItem::GetIndentLevel_Implementation() const
{
	return INDEX_NONE;
}

int32 ICommonListItem::DoesItemHaveChildren_Implementation() const
{
	return INDEX_NONE;
}

//ESelectionMode::Type ICommonListItem::GetSelectionMode_Implementation() const
//{
//	return ESelectionMode::None;
//}

void ICommonListItem::Private_OnExpanderArrowShiftClicked_Implementation()
{
	// stub
}
void ICommonListItem::RegisterOnClicked_Implementation(const FOnItemClicked& Callback)
{
	// stub
}