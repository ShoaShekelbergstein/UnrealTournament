// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#include "CommonUIPrivatePCH.h"
#include "CommonUIContext.h"

#include "CommonInputManager.h"

UCommonUIContext::UCommonUIContext()
	:
#if !PLATFORM_DESKTOP
	bIsUsingGamepad(true)
#else
	bIsUsingGamepad(false)
#endif
{


}

void UCommonUIContext::Initialize()
{
	CommonInputManager = NewObject<UCommonInputManager>(this);
}

void UCommonUIContext::PreDestructContext(UWorld* World)
{
	CommonInputManager->MarkPendingKill();
}

void UCommonUIContext::SetUsingGamepad(bool bUsingGamepad)
{
	bIsUsingGamepad = bUsingGamepad;
	OnInputMethodChangedNative.Broadcast(bUsingGamepad);
	OnInputMethodChanged.Broadcast(bUsingGamepad);
}
