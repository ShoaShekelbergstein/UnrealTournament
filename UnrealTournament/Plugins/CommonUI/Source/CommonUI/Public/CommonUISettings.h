#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Misc/StringClassReference.h"
#include "Templates/SubclassOf.h"
#include "CommonUISettings.generated.h"

class UCommonTextStyle;
class UCommonButtonStyle;

UCLASS(config = Game, defaultconfig)
class COMMONUI_API UCommonUISettings : public UObject
{
	GENERATED_BODY()

public:
	virtual void PostInitProperties() override;

	UPROPERTY(config, EditAnywhere, Category = "Text", meta = (MetaClass = "CommonTextStyle"))
	FStringClassReference DefaultTextStyle_StringRef;

	UPROPERTY()
	TSubclassOf<UCommonTextStyle> DefaultTextStyle;

	UPROPERTY(config, EditAnywhere, Category = "Buttons", meta = (MetaClass = "CommonButtonStyle"))
	FStringClassReference DefaultButtonStyle_StringRef;

	UPROPERTY()
	TSubclassOf<UCommonButtonStyle> DefaultButtonStyle;
};