// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CommonUITypes.h"

#include "CommonActionHandlerInterface.generated.h"

/** Action Delegates */

/** 
 * Action committed delegate tells the handler that an action is ready to handle. Return value
 * is used to determine if the action was handled or ignored.
 */
DECLARE_DYNAMIC_DELEGATE_OneParam(FCommonActionCommited, bool&, bPassThrough);

/**
 * Action complete delegate will tell a listener if a held action completed. The single delegate
 * will be used for binding with a listener that the multicast delegate calls.
 */
DECLARE_DYNAMIC_DELEGATE(FCommonActionCompleteSingle);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FCommonActionComplete);

/**
 * Action progress delegate will tell a listener about the progress of an action being held. The 
 * single delegate will be used for binding with a listener that the multicast delegate calls.
 */
DECLARE_DYNAMIC_DELEGATE_OneParam(FCommonActionProgressSingle, float, HeldPercent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCommonActionProgress, float, HeldPercent);

/**
 *  Action Handler Data used to trigger any callbacks by an object implementing the 
 *	ICommonActionHandlerInterface. The callback can be the implementor of 
 *	ICommonActionHandlerInterface or a user widget.
 */

struct FCommonInputActionHandlerData
{
	FCommonActionProgress OnActionProgress;
	FCommonActionCommited OnActionCommited;
	FCommonActionComplete OnActionComplete;

	FCommonInputActionHandlerData()
	{
	}

	FCommonInputActionHandlerData(const FCommonActionProgressSingle& ActivationProgressDelgate, const FCommonActionCommited& ActivationCommitedDelegate)
		: OnActionCommited(ActivationCommitedDelegate)
	{
		OnActionProgress.Add(ActivationProgressDelgate);
	}
};

/**
 *  Action Handler Interface is primarily used to take key input and do something with it
 *	in the implementation of the interface or another user widget.
 */

UINTERFACE()
class UCommonActionHandlerInterface : public UInterface
{
	GENERATED_UINTERFACE_BODY()
};


class ICommonActionHandlerInterface
{
	GENERATED_IINTERFACE_BODY()

public:
	/** The common input manager calls this for any object implementing this interface */
	virtual bool HandleInput(FKey KeyPressed, EInputEvent EventType, bool& bPassThrough) = 0;
	/** When an action progress happens for a held key, this will be called */
	virtual void UpdateCurrentlyHeldAction(float InDeltaTime) = 0;
};