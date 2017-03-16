// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CommonLoadGuard.h"
#include "CommonUITypes.h"
#include "ICommonUIModule.h"
#include "Components/Image.h"
#include "CommonLazyImage.generated.h"

class UCommonLoadGuard;
class UCommonMcpItemDefinition;

/**
 * A special Image widget that can show unloaded images and takes care of the loading for you!
 */
UCLASS()
class UCommonLazyImage : public UImage
{
	GENERATED_UCLASS_BODY()

public:
	/** Set the brush from a lazy texture asset pointer - will load the texture as needed. */
	UFUNCTION(BlueprintCallable, Category = LazyImage)
	void SetBrushFromLazyTexture(const TAssetPtr<UTexture2D>& LazyTexture, bool bMatchSize = false);

	/** Set the brush from a lazy material asset pointer - will load the material as needed. */
	UFUNCTION(BlueprintCallable, Category = LazyImage)
	void SetBrushFromLazyMaterial(const TAssetPtr<UMaterialInterface>& LazyMaterial);
	
	/** Set the brush from a string asset ref only - expects the referenced asset to be a texture, material, or item definition. */
	UFUNCTION(BlueprintCallable, Category = LazyImage)
	void SetBrushFromLazyDisplayAsset(const TAssetPtr<UObject>& LazyObject, bool bMatchTextureSize = false);

	UFUNCTION(BlueprintCallable, Category = LazyImage)
	bool IsLoading() const;

	void CancelLoading();

	UPROPERTY(BlueprintAssignable, Category = LazyImage)
	FOnLoadingStateChanged OnLoadingStateChanged;

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void OnWidgetRebuilt() override;
	virtual void ReleaseSlateResources(bool bReleaseChildren) override;
	virtual void SynchronizeProperties() override;

#if WITH_EDITOR
	virtual const FText GetPaletteCategory() override;
#endif	

#if WITH_EDITORONLY_DATA
	UPROPERTY(EditAnywhere, Category = LoadGuard)
	bool bShowLoading;
#endif

	UFUNCTION(BlueprintNativeEvent, Category = LazyImage)
	void ShowDefaultImage();

private:
	// Simply forwards the changes from the load guard
	UFUNCTION() void ForwardLoadingStateChanged(bool bIsLoading);

	TAssetPtr<UObject> LazyAsset;
	bool bMatchSizeIfTexture;

	UPROPERTY(Transient)
	UCommonLoadGuard* LoadGuard;
};