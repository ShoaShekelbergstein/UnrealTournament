#pragma once

#include "BlueprintContextBase.h"

#include "CommonUIContext.generated.h"

class UCommonInputManager;

UCLASS()
class COMMONUI_API UCommonUIContext : public UBlueprintContextBase
{
	GENERATED_BODY()

public:

	UCommonUIContext();

	// Begin UBlueprintContextBase
	virtual void Initialize() override;
	virtual void PreDestructContext(UWorld* World) override;

	UFUNCTION(BlueprintCallable, Category = CommonUIContext)
	UCommonInputManager* GetInputManager() const
	{
		ensure(CommonInputManager);
		return CommonInputManager;
	}

	/** Gamepad changed related API */
	void SetUsingGamepad(bool bUsingGamepad);

	UFUNCTION(BlueprintCallable, Category = CommonUIContext)
	bool IsUsingGamepad() const { return bIsUsingGamepad; }

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FInputMethodChangedDelegate, bool, bUsingGamepad);
	DECLARE_EVENT_OneParam(UCommonUIContext, FInputMethodChangedEvent, bool);

	UPROPERTY(BlueprintAssignable, Category = CommonUIContext)
	FInputMethodChangedDelegate OnInputMethodChanged;
	FInputMethodChangedEvent OnInputMethodChangedNative;	
private:
	
	UPROPERTY(Transient)
	UCommonInputManager* CommonInputManager;

	UPROPERTY(Transient)
	bool bIsUsingGamepad;
};