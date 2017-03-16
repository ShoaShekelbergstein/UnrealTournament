// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.
#pragma once
#include "ModuleManager.h"

struct FStreamableManager;

/**
 * Interface for the purchase flow module.
 */
class ICommonUIModule
	: public IModuleInterface
{
public:

	/**
	 * Singleton-like access to this module's interface.  This is just for convenience!
	 * Beware of calling this during the shutdown phase, though.  Your module might have been unloaded already.
	 *
	 * @return Returns singleton instance, loading the module on demand if needed
	 */
	static inline ICommonUIModule& Get()
	{
		return FModuleManager::LoadModuleChecked<ICommonUIModule>("CommonUI");
	}

	/**
	 * Checks to see if this module is loaded and ready.  It is only valid to call Get() if IsAvailable() returns true.
	 *
	 * @return True if the module is loaded and ready to use
	 */
	static inline bool IsAvailable()
	{
		return FModuleManager::Get().IsModuleLoaded("CommonUI");
	}

	// Streamable Management
	typedef TFunction<FStreamableManager* ()> FStreamableManagerGetter;
	virtual void SetStreamableManagerGetter(const FStreamableManagerGetter& Getter) = 0;
	virtual FStreamableManager* GetStreamableManager() const = 0;

	// Lazy Load Priority
	virtual TAsyncLoadPriority GetLazyLoadPriority() const = 0;
};
