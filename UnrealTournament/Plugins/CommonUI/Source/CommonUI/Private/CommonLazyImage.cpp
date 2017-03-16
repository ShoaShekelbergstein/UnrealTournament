// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#include "CommonUIPrivatePCH.h"
#include "CommonLazyImage.h"
#include "CommonTextBlock.h"
#include "CommonLoadGuard.h"
#include "CommonWidgetPaletteCategories.h"
#include "Engine/Texture2D.h"

#include "Materials/MaterialInterface.h"

UCommonLazyImage::UCommonLazyImage(const FObjectInitializer& Initializer)
	: Super(Initializer)
	, LoadGuard(nullptr)
{
#if WITH_EDITORONLY_DATA
	bShowLoading = false;
#endif
}

bool UCommonLazyImage::IsLoading() const
{
	return LoadGuard ? LoadGuard->IsLoading() : false;
}

void UCommonLazyImage::CancelLoading()
{
	LazyAsset.Reset();
	if ( LoadGuard )
	{
		LoadGuard->SetIsLoading(true);
	}
}

TSharedRef<SWidget> UCommonLazyImage::RebuildWidget()
{
	LoadGuard = NewObject<UCommonLoadGuard>(this);
	// For images, just show the spinner
	LoadGuard->SetLoadingText(FText());

	// Very important to TakeWidget() BEFORE actually setting the content
	TSharedRef<SWidget> LoadGuardWidget = LoadGuard->TakeWidget();

	// The load guard contains the actual image
	LoadGuard->SetContent(Super::RebuildWidget());
	
	if (!IsDesignTime())
	{
		LoadGuard->OnLoadingStateChanged.AddUniqueDynamic(this, &UCommonLazyImage::ForwardLoadingStateChanged);
	}

	return LoadGuardWidget;
}

void UCommonLazyImage::OnWidgetRebuilt()
{
	Super::OnWidgetRebuilt();

	if (!LazyAsset.IsNull())
	{
		SetBrushFromLazyDisplayAsset(LazyAsset);
	}
}

void UCommonLazyImage::SynchronizeProperties()
{
	Super::SynchronizeProperties();

#if WITH_EDITORONLY_DATA
	if (LoadGuard)
	{
		LoadGuard->SetIsLoading(bShowLoading);
	}
#endif
}

#if WITH_EDITOR
const FText UCommonLazyImage::GetPaletteCategory()
{
	return CommonWidgetPaletteCategories::Default;
}
#endif

void UCommonLazyImage::ShowDefaultImage_Implementation()
{
	SetBrushFromTexture(nullptr);
}

void UCommonLazyImage::ForwardLoadingStateChanged(bool bIsLoading)
{
	OnLoadingStateChanged.Broadcast(bIsLoading);
}

void UCommonLazyImage::ReleaseSlateResources(bool bReleaseChildren)
{
	Super::ReleaseSlateResources(bReleaseChildren);

	LazyAsset.Reset();
	if (LoadGuard)
	{
		LoadGuard->ReleaseSlateResources(bReleaseChildren);
		LoadGuard = nullptr;
	}
}

void UCommonLazyImage::SetBrushFromLazyTexture(const TAssetPtr<UTexture2D>& LazyTexture, bool bMatchSize)
{
	if (!LazyTexture.IsNull())
	{
		LazyAsset = LazyTexture;
		bMatchSizeIfTexture = bMatchSize;
		SetBrushFromTexture(nullptr);

		if (LoadGuard)
		{
			LoadGuard->GuardAndLoadAsset<UTexture2D>(LazyTexture, 
				[this] (UTexture2D* Texture)
				{
					SetBrushFromTexture(Texture, bMatchSizeIfTexture);
				});
		}
		else
		{
			UE_LOG(LogCommonUI, Verbose, TEXT("Tried to set [%s] from a lazy texture before the load guard is valid"), *GetName());
		}
	}
	else
	{
		ShowDefaultImage();
	}
}

void UCommonLazyImage::SetBrushFromLazyMaterial(const TAssetPtr<UMaterialInterface>& LazyMaterial)
{
	if (!LazyMaterial.IsNull())
	{
		LazyAsset = LazyMaterial;
		bMatchSizeIfTexture = false;
		SetBrushFromTexture(nullptr);
		
		if (LoadGuard)
		{
			LoadGuard->GuardAndLoadAsset<UMaterialInterface>(LazyMaterial,
				[this] (UMaterialInterface* Material)
				{
					SetBrushFromMaterial(Material);
				});
		}
		else
		{
			UE_LOG(LogCommonUI, Warning, TEXT("Tried to set [%s] from a lazy material before the load guard is valid"), *GetName());
		}
	}
	else
	{
		ShowDefaultImage();
	}
}

void UCommonLazyImage::SetBrushFromLazyDisplayAsset(const TAssetPtr<UObject>& LazyObject, bool bMatchTextureSize)
{
	if (!LazyObject.IsNull())
	{
		LazyAsset = LazyObject;
		
		bMatchSizeIfTexture = bMatchTextureSize;
		SetBrushFromTexture(nullptr);

		if (LoadGuard)
		{
			LoadGuard->GuardAndLoadAsset<UObject>(LazyAsset,
				[this] (UObject* Object)
				{
					if (UTexture2D* AsTexture = Cast<UTexture2D>(Object))
					{
						SetBrushFromTexture(AsTexture, bMatchSizeIfTexture);
					}
					else if (UMaterialInterface* AsMaterial = Cast<UMaterialInterface>(Object))
					{
						SetBrushFromMaterial(AsMaterial);
					}
				});
		}
		else
		{
			UE_LOG(LogCommonUI, Warning, TEXT("Tried to set [%s] from a display asset before the load guard is valid"), *GetName());
		}
	}
	else
	{
		ShowDefaultImage();
	}
}