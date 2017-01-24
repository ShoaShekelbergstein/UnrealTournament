#include "UnrealTournament.h"
#include "UTMonsterAI.h"

void AUTMonsterAI::CheckWeaponFiring(bool bFromWeapon)
{
	if (bOneShotAttacks && bFromWeapon)
	{
		if (GetUTChar() != nullptr)
		{
			GetUTChar()->StopFiring();
			GetWorldTimerManager().SetTimer(CheckWeaponFiringTimerHandle, this, &AUTMonsterAI::CheckWeaponFiringTimed, 1.2f - 0.09f * FMath::Min<float>(10.0f, Skill + Personality.ReactionTime), true);
		}
	}
	// if invis, don't attack until close, under attack, or have flag
	else if ( bFromWeapon || GetUTChar() == nullptr || !GetUTChar()->IsInvisible() || GetWorld()->TimeSeconds - GetUTChar()->LastTakeHitTime < 3.0f ||
		GetUTChar()->GetCarriedObject() != nullptr || GetTarget() == nullptr || (GetTarget()->GetActorLocation() - GetUTChar()->GetActorLocation()).Size() < 2000.0f )
	{
		Super::CheckWeaponFiring(bFromWeapon);
	}
}