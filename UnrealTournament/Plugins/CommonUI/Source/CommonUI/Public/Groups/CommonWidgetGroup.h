// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Components/Widget.h"
#include "CommonWidgetGroup.generated.h"

UCLASS( BlueprintType )
class COMMONUI_API UCommonWidgetGroup : public UObject
{
	GENERATED_BODY()
public:
	UCommonWidgetGroup();

	virtual TSubclassOf<UWidget> GetWidgetType() const { return UWidget::StaticClass(); }

	UFUNCTION( BlueprintCallable, Category = Group )
	void AddWidget( UWidget* InWidget );

	UFUNCTION( BlueprintCallable, Category = Group )
	void RemoveWidget( UWidget* InWidget );

	UFUNCTION( BlueprintCallable, Category = Group )
	void RemoveAll();

	template <class WidgetType, class... WidgetTypes>
	void AddWidgets( WidgetType&& Widget, WidgetTypes&&... Widgets )
	{
		AddWidgets( Forward<WidgetType>( Widget ) );
		AddWidgets( Forward<WidgetTypes>( Widgets )... );
	}

	template <class WidgetType>
	void AddWidgets( WidgetType&& Widget )
	{
		AddWidget( Widget );
	}

protected:
	virtual void OnWidgetAdded( UWidget* NewWidget ) PURE_VIRTUAL( UCommonWidgetGroup::OnWidgetAdded, );
	virtual void OnWidgetRemoved( UWidget* OldWidget ) PURE_VIRTUAL( UCommonWidgetGroup::OnWidgetRemoved, );
	virtual void OnRemoveAll() PURE_VIRTUAL( UCommonWidgetGroup::OnRemoveAll, );
};