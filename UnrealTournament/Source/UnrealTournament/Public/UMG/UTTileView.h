// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "UTListView.h"
#include "Widgets/Views/STileView.h"
#include "CommonTileView.h"

#include "UTTileView.generated.h"

UCLASS(DisplayName = "UT Tile View")
class UUTTileView : public UUTListView
{
	GENERATED_BODY()

public:
	UUTTileView();

	virtual void ReleaseSlateResources( bool bReleaseChildren ) override;
	virtual void BeginDestroy() override;

	/** Sets the width of every tile in the list */
	UFUNCTION(BlueprintCallable, Category = TileView)
	void SetItemWidth(float NewWidth);

protected:
	virtual TSharedRef<SListView<UObject*>> RebuildListWidget() override;

	float GetTotalItemWidth() const;

	UPROPERTY(EditInstanceOnly, Category = ItemLayout, meta = (DesignerRebuild = true))
	EItemAlignment ItemAlignment;

	UPROPERTY(EditInstanceOnly, Category = ItemLayout, meta = (DesignerRebuild = true))
	float ItemWidth;

private:
	TSharedPtr<STileView<UObject*>> MyTileView;
};