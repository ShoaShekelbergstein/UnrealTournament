// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#include "CommonUIPrivatePCH.h"
#include "CommonObjectListItem.h"

UCommonObjectListItem::UCommonObjectListItem(const FObjectInitializer& Initializer)
	: Super(Initializer)
{
}

UObject* ICommonObjectListItem::GetData_Implementation() const
{
	return nullptr;
}

void ICommonObjectListItem::SetData_Implementation( UObject* InData )
{
	// stub
}

void ICommonObjectListItem::Reset_Implementation()
{
	// stub
}
