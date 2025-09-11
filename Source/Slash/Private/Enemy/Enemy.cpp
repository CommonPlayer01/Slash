// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/Enemy.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "Slash/DebugMacros.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Components/AttributeComponent.h"
#include "HUD/HealthBarComponent.h"
#include "AIController.h"
#include "NavigationData.h"
#include "Navigation/PathFollowingComponent.h"


// Sets default values
AEnemy::AEnemy()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	GetMesh()->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetMesh()->SetGenerateOverlapEvents(true);

	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);

	Attributes = CreateDefaultSubobject<UAttributeComponent>(TEXT("Attributes"));
	HealthBarWidget = CreateDefaultSubobject<UHealthBarComponent>(TEXT("HealthBar"));
	HealthBarWidget->SetupAttachment(GetRootComponent());
	

}

// Called when the game starts or when spawned
void AEnemy::BeginPlay()
{
	Super::BeginPlay();
	if(HealthBarWidget) {
		HealthBarWidget->SetVisibility(false);
	}

	EnemyController = Cast<AAIController>(GetController());
	FAIMoveRequest MoveRequest;
	MoveRequest.SetGoalActor(PatrolTarget);
	MoveRequest.SetAcceptanceRadius(15.f);
	FNavPathSharedPtr NavPath;
	EnemyController->MoveTo(MoveRequest, &NavPath);
	TArray<FNavPathPoint>& PathPoints = NavPath->GetPathPoints();

	for(auto& Point : PathPoints) {
		const FVector& PointLocation = Point.Location;
		DrawDebugSphere(GetWorld(), PointLocation, 12.f, 12, FColor::Red, false, 10.f);
	}
}

void AEnemy::Die()
{
	//TODO: Play death animation
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if(AnimInstance && DeathMontage) {
		AnimInstance->Montage_Play(DeathMontage);
		AnimInstance->Montage_JumpToSection("Death1", DeathMontage);
		DeathPose = EDeathPose::EDP_Death1;
	}
	if(HealthBarWidget) {
		HealthBarWidget->SetVisibility(false);
	}
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SetLifeSpan(3.f);
}
 bool AEnemy::InTargetRange(AActor *Target, double Radius)
{	
	const double DistanceToTarget = (Target->GetActorLocation() - GetActorLocation()).Size();
	DRAW_SPHERE_SingleFrame(GetActorLocation());
	DRAW_SPHERE_SingleFrame(Target->GetActorLocation());
	return DistanceToTarget <= Radius;
}
void AEnemy::PlayHitReactMontage(const FName& SectionName)
{
    UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if(AnimInstance && HitReactMontage) {
		AnimInstance->Montage_Play(HitReactMontage);
		AnimInstance->Montage_JumpToSection(SectionName, HitReactMontage);
	}
}
// Called every frame
void AEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if(CombatTarget){
		if(!InTargetRange(CombatTarget, combatRadius)) {
			CombatTarget = nullptr;
			if(HealthBarWidget) {
				HealthBarWidget->SetVisibility(false);
			}
		}
	}
	if(PatrolTarget && EnemyController){
		if(InTargetRange(PatrolTarget, PatrolRadius)) {
			TArray<AActor*> ValidTargets;
			for(AActor* Target : PatrolTargets) {
				if(Target != PatrolTarget) {
					ValidTargets.AddUnique(Target);
				}
			}
			const int32 NumPatrolTargets = ValidTargets.Num();

			if(NumPatrolTargets > 0) {
				const int32 TargetSelection = FMath::RandRange(0, NumPatrolTargets - 1);
				AActor* Target = ValidTargets[TargetSelection];
				PatrolTarget = Target;

				FAIMoveRequest MoveRequest;
				MoveRequest.SetGoalActor(PatrolTarget);
				MoveRequest.SetAcceptanceRadius(15.f);
				EnemyController->MoveTo(MoveRequest);
			}
		}
	}

}

// Called to bind functionality to input
void AEnemy::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void AEnemy::GetHit_Implementation(const FVector& ImpactPoint)
{
	// DRAW_SPHERE_COLOR(ImpactPoint, FColor::Orange);

	// DirectionalHitReact(ImpactPoint);

	if(HealthBarWidget) {
		HealthBarWidget->SetVisibility(true);
	}
	if(Attributes && Attributes->IsAlive()) {
		DirectionalHitReact(ImpactPoint);
	}else{
		Die();
	}

	if(HitSound) {
		UGameplayStatics::PlaySoundAtLocation(this, HitSound, ImpactPoint);
	}
	if(HitParticles) {
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), HitParticles, ImpactPoint);
	}
}

void AEnemy::DirectionalHitReact(const FVector& ImpactPoint)
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

	FName SectionName = FName();
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

float AEnemy::TakeDamage(float DamageAmount, FDamageEvent const &DamageEvent, AController *EventInstigator, AActor *DamageCauser)
{	
	if(Attributes && HealthBarWidget){
		Attributes->ReceiveDamage(DamageAmount);
		HealthBarWidget->SetHealthPercent(Attributes->GetHealthPercentage());
	}
	CombatTarget = EventInstigator->GetPawn();
    return DamageAmount;
}
