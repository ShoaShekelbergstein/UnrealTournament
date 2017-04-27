// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Runtime/Core/Public/Features/IModularFeature.h"

class UTVoiceChatFeature : public IModularFeature
{
public:
	virtual void Connect() = 0;
	virtual void Disconnect() = 0;

	virtual void LoginUsingToken(const FString& PlayerName, const FString& Token) = 0;
	virtual void Logout(const FString& PlayerName) = 0;
	virtual void JoinChannelUsingToken(const FString& PlayerName, const FString& Channel, const FString& Token) = 0;
	virtual void LeaveChannel(const FString& PlayerName, const FString& Channel) = 0;

	virtual void SetPlaybackVolume(float InVolume) = 0;
	virtual void SetRecordVolume(float InVolume) = 0;

	virtual void SetAudioInputDeviceMuted(bool bIsMuted) = 0;

	virtual void MutePlayer(const FString& PlayerName) = 0;
	virtual void UnMutePlayer(const FString& PlayerName) = 0;
	virtual bool IsPlayerMuted(const FString& PlayerName) = 0;
};