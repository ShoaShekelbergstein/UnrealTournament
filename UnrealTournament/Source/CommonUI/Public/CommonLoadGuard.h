// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CommonUITypes.h"
#include "ICommonUIModule.h"
#include "Engine/StreamableManager.h"
#include "Components/ContentWidget.h"
#include "CommonLoadGuard.generated.h"

class UCommonTextStyle;
class UCommonTextBlock;
class SBox;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLoadingStateChanged, bool, bIsLoading);

/**
 * The load guard is a simple container to use whenever a widget may need to load before appearing.
 */
UCLASS(Config = CommonUI, DefaultConfig)
class COMMONUI_API UCommonLoadGuard : public UContentWidget
{
	GENERATED_UCLASS_BODY()

	DECLARE_DYNAMIC_DELEGATE_OneParam(FOnAssetLoaded, UObject*, Object);

public:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void ReleaseSlateResources(bool bReleaseChildren) override;
	virtual void SynchronizeProperties() override;

	virtual UClass* GetSlotClass() const override;
	virtual void OnSlotAdded(UPanelSlot* Slot) override;
	virtual void OnSlotRemoved(UPanelSlot* Slot) override;

	void SetContent(const TSharedRef<SWidget>& Content);

	UFUNCTION(BlueprintCallable, Category = LoadGuard)
	void SetLoadingText(const FText& InLoadingText);
	
	UFUNCTION(BlueprintCallable, Category = LoadGuard)
	void SetIsLoading(bool bInIsLoading);

	UFUNCTION(BlueprintCallable, Category = LoadGuard)
	bool IsLoading() const;

	UFUNCTION(BlueprintCallable, Category = LoadGuard, meta = (DisplayName = "Guard and Load Asset"))
	void BP_GuardAndLoadAsset(const TAssetPtr<UObject>& InLazyAsset, const FOnAssetLoaded& OnAssetLoaded);

	/** 
	 * Displays the loading spinner until the asset is loaded
	 * Will pass a casted pointer to the asset to the loaded object - could be nullptr if you provide an incompatible type.
	 */
	template <typename ObjectType>
	void GuardAndLoadAsset(const TAssetPtr<UObject>& InLazyAsset, TFunction<void(ObjectType*)> OnAssetLoaded)
	{
		LazyAsset = InLazyAsset;

		if (InLazyAsset.IsValid())
		{
			OnAssetLoaded(Cast<ObjectType>(InLazyAsset.Get()));
			SetIsLoadingInternal(false);
		}
		else if (!InLazyAsset.IsNull())
		{
			SetIsLoadingInternal(true);

			TWeakObjectPtr<UCommonLoadGuard> WeakThis = this;
			const ICommonUIModule& CommonUIModule = ICommonUIModule::Get();
			FStreamableManager* const StreamableManager = CommonUIModule.GetStreamableManager();
			StreamableManager->RequestAsyncLoad(InLazyAsset.ToStringReference(),
				[WeakThis, OnAssetLoaded]()
				{
					if (WeakThis.IsValid() && WeakThis->LazyAsset.IsValid() && WeakThis->bIsLoading)
					{
						OnAssetLoaded(Cast<ObjectType>(WeakThis->LazyAsset.Get()));
						WeakThis->SetIsLoadingInternal(false);
					}
				},
				CommonUIModule.GetLazyLoadPriority());
		}
	}

	UPROPERTY(BlueprintAssignable, Category = LoadGuard)
	FOnLoadingStateChanged OnLoadingStateChanged;

protected:
#if WITH_EDITOR
	virtual const FText GetPaletteCategory() override;
#endif	

#if WITH_EDITORONLY_DATA
	UPROPERTY(EditAnywhere, Category = LoadGuard)
	bool bShowLoading;
#endif

private:
	void SetIsLoadingInternal(bool bInIsLoading, bool bForce = true);
	void RefreshText();

	TAssetPtr<UObject> LazyAsset;

	/** The horizontal alignment of the loading throbber & message */
	UPROPERTY(EditAnywhere, Category = LoadGuardThrobber)
	TEnumAsByte<EHorizontalAlignment> ThrobberAlignment;

	/** The horizontal alignment of the loading throbber & message */
	UPROPERTY(EditAnywhere, Category = LoadGuardThrobber)
	FMargin ThrobberPadding;

	/** Displays as "Loading {Name}..." */
	UPROPERTY(EditAnywhere, Category = LoadGuardText)
	FText LoadingText;

	UPROPERTY(Config)
	FStringClassReference TextStyleClass;

	/** References the button style asset that defines a style in multiple sizes */
	UPROPERTY(EditAnywhere, Category = LoadGuardText)
	TSubclassOf<UCommonTextStyle> TextStyle;

	UPROPERTY(Transient)
	UCommonTextBlock* Text_LoadingText;

	bool bIsLoading;

	TSharedPtr<class SCommonLoadGuard> MyLoadGuard;

	TSharedPtr<SOverlay> MyGuardOverlay;
	TSharedPtr<SBox> MyGuardBox;
	TSharedPtr<SBox> MyContentBox;
};