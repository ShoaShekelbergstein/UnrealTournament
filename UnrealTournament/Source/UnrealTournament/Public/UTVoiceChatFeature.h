// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Runtime/Core/Public/Features/IModularFeature.h"

class UTVoiceChatFeature : public IModularFeature
{
public:
	virtual void LoginUsingToken(const FString& PlayerName, const FString& Token) = 0;
	virtual void Disconnect(const FString& PlayerName) = 0;
	virtual void JoinChannelUsingToken(const FString& PlayerName, const FString& Channel, const FString& Token) = 0;
	virtual void LeaveChannel(const FString& PlayerName, const FString& Channel) = 0;
};