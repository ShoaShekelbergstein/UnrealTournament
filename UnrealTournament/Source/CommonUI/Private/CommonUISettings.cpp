
#include "CommonUIPrivatePCH.h"
#include "CommonUISettings.h"
#include "CommonTextBlock.h"
#include "CommonButton.h"

#define FIXUP_CLASS_REFERENCE_FROM_STRING_REFERENCE(ClassType, MemberName) MemberName = MemberName##_StringRef.TryLoadClass<ClassType>();
#define FIXUP_OBJECT_REFERENCE_FROM_STRING_REFERENCE(ObjectType, MemberName) MemberName = Cast<ObjectType>(MemberName##_StringRef.ResolveObject());
	
void UCommonUISettings::PostInitProperties()
{
	Super::PostInitProperties();

	FIXUP_CLASS_REFERENCE_FROM_STRING_REFERENCE(UCommonTextStyle, DefaultTextStyle);
	FIXUP_CLASS_REFERENCE_FROM_STRING_REFERENCE(UCommonButtonStyle, DefaultButtonStyle)
}

#undef FIXUP_OBJECT_REFERENCE_FROM_STRING_REFERENCE
#undef FIXUP_CLASS_REFERENCE_FROM_STRING_REFERENCE
