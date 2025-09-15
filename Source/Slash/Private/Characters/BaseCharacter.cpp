#include "Characters/BaseCharacter.h"
#include "Components/BoxComponent.h"
#include "Components/AttributeComponent.h"
#include "Components/CapsuleComponent.h"
#include "Item/Weapons/Weapon.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"

ABaseCharacter::ABaseCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	Attributes = CreateDefaultSubobject<UAttributeComponent>(TEXT("Attributes"));

}




void ABaseCharacter::BeginPlay()
{
	Super::BeginPlay();

	

}

void ABaseCharacter::GetHit_Implementation(const FVector& ImpactPoint, AActor* Hitter)
{
	if (Attributes && Attributes->IsAlive()) {
		DirectionalHitReact(ImpactPoint);
	}
	else {
		Die();
	}
	PlayHitSound(ImpactPoint);
	SpawnHitParticles(ImpactPoint);
}

void ABaseCharacter::Attack() {

}

void ABaseCharacter::Die() {

}	

void ABaseCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ABaseCharacter::PlayMontageSection(UAnimMontage* Montage, const FName& SectionName)
{	
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && Montage) {
		AnimInstance->Montage_Play(AttackMontage);
		AnimInstance->Montage_JumpToSection(SectionName, AttackMontage);
	}
}

int32 ABaseCharacter::PlayRandomMontageSection(UAnimMontage* Montage, const TArray<FName>& SectionNames)
{
	if (SectionNames.Num() <= 0) return -1;
	const int32 Selection = FMath::RandRange(0, SectionNames.Num() - 1);
	PlayMontageSection(Montage, SectionNames[Selection]);
	return Selection;
}


int32 ABaseCharacter::PlayAttackMontage() {
	return PlayRandomMontageSection(AttackMontage, AttackMontageSections);
}
int32 ABaseCharacter::PlayDeathMontage()
{
	return PlayRandomMontageSection(DeathMontage, DeathMontageSections);
}
void ABaseCharacter::DisableCapsule()
{
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}
void ABaseCharacter::PlayHitSound(const FVector& ImpactPoint)
{
	if (HitSound) {
		UGameplayStatics::PlaySoundAtLocation(this, HitSound, ImpactPoint);
	}
}

void ABaseCharacter::SpawnHitParticles(const FVector& ImpactPoint)
{
	if (HitParticles) {
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), HitParticles, ImpactPoint);
	}

}

void ABaseCharacter::HandleDamage(float DamageAmount)
{
	if (Attributes) {
		Attributes->ReceiveDamage(DamageAmount);
	}
}

bool ABaseCharacter::CanAttack()
{
	return false;
}

bool ABaseCharacter::IsAlive()
{
	return Attributes && Attributes->IsAlive();
}

void ABaseCharacter::AttackEnd() {

}
void ABaseCharacter::PlayHitReactMontage(const FName& SectionName) {
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && HitReactMontage) {
		AnimInstance->Montage_Play(HitReactMontage);
		AnimInstance->Montage_JumpToSection(SectionName, HitReactMontage);
	}
}
void ABaseCharacter::DirectionalHitReact(const FVector& ImpactPoint)
{
	const FVector Forward = GetActorForwardVector();
	const FVector ImapactLowered(ImpactPoint.X, ImpactPoint.Y, GetActorLocation().Z);
	const FVector ToHit = (ImapactLowered - GetActorLocation()).GetSafeNormal();

	const double CosTheta = FVector::DotProduct(Forward, ToHit);
	double Theta = FMath::Acos(CosTheta);
	Theta = FMath::RadiansToDegrees(Theta);

	const FVector CrossProduct = FVector::CrossProduct(Forward, ToHit);
	UKismetSystemLibrary::DrawDebugArrow(this, GetActorLocation(), GetActorLocation() + CrossProduct * 60.f, 5.f, FColor::Green, 5.f, 2.f);

	if (CrossProduct.Z < 0) {
		Theta *= -1.f;
	}

	FName SectionName = FName("FromBack");
	if (Theta >= -45.f && Theta < 45.f) {
		PlayHitReactMontage(FName("FromFront"));
	}
	else if (Theta >= 45.f && Theta < 135.f) {
		PlayHitReactMontage(FName("FromRight"));
	}
	else if (Theta >= -135.f && Theta < -45.f) {
		PlayHitReactMontage(FName("FromLeft"));
	}

	PlayHitReactMontage(SectionName);

	if (GEngine) {
		FString Message = FString::Printf(TEXT("Theta: %f"), Theta);
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, Message);
	}
	UKismetSystemLibrary::DrawDebugArrow(this, GetActorLocation(), GetActorLocation() + Forward * 60.f, 5.f, FColor::Green, 5.f, 2.f);
	UKismetSystemLibrary::DrawDebugArrow(this, GetActorLocation(), GetActorLocation() + ToHit * 60.f, 5.f, FColor::Orange, 5.f, 2.f);
}
void ABaseCharacter::SetWeaponCollisionEnabled(ECollisionEnabled::Type CollisionEnabled) {
	if (EquippedWeapon && EquippedWeapon->GetWeaponBox()) {
		EquippedWeapon->GetWeaponBox()->SetCollisionEnabled(CollisionEnabled);
		EquippedWeapon->IgnoredActors.Empty(); 
	}
}


