// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#pragma once

// From Core:
#include "CoreTypes.h"
#include "CoreFwd.h"
#include "Containers/ContainersFwd.h"
#include "UObject/UObjectHierarchyFwd.h"
#include "GenericPlatform/GenericPlatformMisc.h"
#include "HAL/PlatformCrt.h"
#include "Misc/AssertionMacros.h"
#include "HAL/PlatformMisc.h"
#include "Templates/IsPointer.h"
#include "Misc/VarArgs.h"
#include "Misc/OutputDevice.h"
#include "Logging/LogVerbosity.h"
#include "GenericPlatform/GenericPlatformAtomics.h"
#include "Misc/Exec.h"
#include "Templates/EnableIf.h"
#include "Templates/AndOrNot.h"
#include "HAL/UnrealMemory.h"
#include "HAL/PlatformMemory.h"
#include "GenericPlatform/GenericPlatformMemory.h"
#include "HAL/MemoryBase.h"
#include "HAL/PlatformAtomics.h"
#include "Templates/AreTypesEqual.h"
#include "Templates/IsTriviallyCopyConstructible.h"
#include "Templates/IsPODType.h"
#include "Templates/IsArithmetic.h"
#include "Templates/UnrealTypeTraits.h"
#include "Templates/RemoveCV.h"
#include "Templates/AlignOf.h"
#include "Templates/ChooseClass.h"
#include "Templates/IntegralConstant.h"
#include "Templates/IsClass.h"
#include "HAL/PlatformMath.h"
#include "GenericPlatform/GenericPlatformMath.h"
#include "HAL/PlatformProperties.h"
#include "GenericPlatform/GenericPlatformProperties.h"
#include "Misc/Char.h"
#include "GenericPlatform/GenericPlatformStricmp.h"
#include "GenericPlatform/GenericPlatformString.h"
#include "Templates/TypeCompatibleBytes.h"
#include "Templates/UnrealTemplate.h"
#include "Templates/RemoveReference.h"
#include "Traits/IsContiguousContainer.h"
#include "Serialization/Archive.h"
#include "Templates/IsEnumClass.h"
#include "Misc/Compression.h"
#include "Misc/EngineVersionBase.h"
#include "Internationalization/TextNamespaceFwd.h"
#include "Math/UnrealMathUtility.h"
#include "Containers/ContainerAllocationPolicies.h"
#include "Templates/MemoryOps.h"
#include "Templates/IsTriviallyCopyAssignable.h"
#include "Templates/IsTriviallyDestructible.h"
#include "Math/NumericLimits.h"
#include "Containers/Array.h"
#include "Templates/Less.h"
#include "Templates/Sorting.h"
#include "Misc/CString.h"
#include "HAL/PlatformString.h"
#include "Misc/Crc.h"
#include "Containers/UnrealString.h"
#include "Misc/Timespan.h"
#include "HAL/CriticalSection.h"
#include "Containers/StringConv.h"
#include "UObject/NameTypes.h"
#include "UObject/UnrealNames.h"
#include "Logging/LogCategory.h"
#include "Logging/LogMacros.h"
#include "HAL/PlatformTLS.h"
#include "GenericPlatform/GenericPlatformTLS.h"
#include "Templates/Function.h"
#include "Templates/Decay.h"
#include "Templates/Invoke.h"
#include "Templates/PointerIsConvertibleFromTo.h"
#include "CoreGlobals.h"
#include "Templates/AlignmentTemplates.h"
#include "Templates/TypeHash.h"
#include "Containers/Set.h"
#include "Misc/StructBuilder.h"
#include "Containers/SparseArray.h"
#include "Containers/ScriptArray.h"
#include "Containers/BitArray.h"
#include "Containers/Map.h"
#include "Containers/Algo/Reverse.h"
#include "Templates/SharedPointer.h"
#include "Delegates/IDelegateInstance.h"
#include "Delegates/DelegateSettings.h"
#include "Delegates/Delegate.h"
#include "UObject/WeakObjectPtrTemplates.h"
#include "UObject/AutoPointer.h"
#include "Delegates/MulticastDelegateBase.h"
#include "Delegates/DelegateBase.h"
#include "Delegates/IntegerSequence.h"
#include "Delegates/Tuple.h"
#include "Templates/TypeWrapper.h"
#include "UObject/ScriptDelegates.h"
#include "Misc/Parse.h"
#include "Misc/Optional.h"
#include "Containers/EnumAsByte.h"
#include "Internationalization/CulturePointer.h"
#include "Internationalization/Text.h"
#include "Internationalization/TextLocalizationManager.h"
#include "Templates/UniquePtr.h"
#include "Templates/IsArray.h"
#include "Templates/RemoveExtent.h"
#include "Math/Color.h"
#include "GenericPlatform/GenericPlatformTime.h"
#include "HAL/PlatformTime.h"
#include "Misc/ScopeLock.h"
#include "Misc/DateTime.h"
#include "HAL/ThreadSingleton.h"
#include "HAL/TlsAutoCleanup.h"
#include "Internationalization/Internationalization.h"
#include "Templates/UniqueObj.h"
#include "Math/IntPoint.h"
#include "GenericPlatform/GenericPlatformProcess.h"
#include "HAL/PlatformProcess.h"
#include "HAL/ThreadSafeCounter.h"
#include "Misc/Paths.h"
#include "Misc/EnumClassFlags.h"
#include "Math/Vector2D.h"
#include "GenericPlatform/GenericPlatformFile.h"
#include "Math/IntVector.h"
#include "Misc/Guid.h"
#include "Misc/CommandLine.h"
#include "Misc/SlowTask.h"
#include "Misc/SlowTaskStack.h"
#include "Misc/ByteSwap.h"
#include "Misc/FeedbackContext.h"
#include "Misc/NoopCounter.h"
#include "Containers/LockFreeList.h"
#include "Math/Vector.h"
#include "Stats/Stats.h"
#include "Containers/ChunkedArray.h"
#include "Containers/IndirectArray.h"
#include "Math/VectorRegister.h"
#include "Misc/CoreMisc.h"
#include "Math/Rotator.h"
#include "Containers/Queue.h"
#include "Misc/AutomationTest.h"
#include "HAL/FileManager.h"
#include "HAL/Event.h"
#include "GenericPlatform/GenericPlatformAffinity.h"
#include "Misc/App.h"
#include "HAL/IConsoleManager.h"
#include "UObject/ObjectVersion.h"
#include "Misc/ConfigCacheIni.h"
#include "HAL/Runnable.h"
#include "Math/Vector4.h"
#include "HAL/PlatformAffinity.h"
#include "HAL/RunnableThread.h"
#include "Misc/OutputDeviceRedirector.h"
#include "Math/Matrix.h"
#include "Math/Plane.h"
#include "Math/Axis.h"
#include "Misc/IQueuedWork.h"
#include "Misc/QueuedThreadPool.h"
#include "Math/Quat.h"
#include "Internationalization/Culture.h"
#include "Math/Transform.h"
#include "Math/IntRect.h"
#include "GenericPlatform/GenericWindow.h"
#include "Misc/MemStack.h"
#include "Containers/LockFreeFixedSizeAllocator.h"
#include "GenericPlatform/GenericPlatformCompression.h"
#include "Modules/ModuleInterface.h"
#include "Templates/RefCounting.h"
#include "Async/AsyncWork.h"
#include "Templates/ValueOrError.h"
