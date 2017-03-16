// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#include "CommonUIPrivatePCH.h"
#include "CommonWidgetGroup.h"

UCommonWidgetGroup::UCommonWidgetGroup()
{
}

void UCommonWidgetGroup::AddWidget( UWidget* InWidget )
{
	if ( InWidget->IsA( GetWidgetType() ) )
	{
		OnWidgetAdded( InWidget );
	}
}

void UCommonWidgetGroup::RemoveWidget( UWidget* InWidget )
{
	if ( InWidget->IsA( GetWidgetType() ) )
	{
		OnWidgetRemoved( InWidget );
	}
}

void UCommonWidgetGroup::RemoveAll()
{
	OnRemoveAll();
}
