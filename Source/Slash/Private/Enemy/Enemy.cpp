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
#include "GameFramework/CharacterMovementComponent.h"
#include "Perception/PawnSensingComponent.h"
#include "Item/Weapons/Weapon.h"

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


	HealthBarWidget = CreateDefaultSubobject<UHealthBarComponent>(TEXT("HealthBar"));
	HealthBarWidget->SetupAttachment(GetRootComponent());
	
	PawnSensing = CreateDefaultSubobject<UPawnSensingComponent>(TEXT("PawnSensing"));
	PawnSensing->SightRadius = 4000.f;
	PawnSensing->SetPeripheralVisionAngle(45.f);

}

void AEnemy::PatrolTimerFinished()
{
	MoveToTarget(PatrolTarget);
}

// Called when the game starts or when spawned
void AEnemy::BeginPlay()
{
	Super::BeginPlay();
	HideHealthBar();

	EnemyController = Cast<AAIController>(GetController());

	GetWorldTimerManager().SetTimer(PatrolTimer, this, &AEnemy::PatrolTimerFinished, 5.0f);

	if (PawnSensing) {
		PawnSensing->OnSeePawn.AddDynamic(this, &AEnemy::PawnSeen);
	}

	UWorld* World = GetWorld();
	if (World && WeaponClass) {
		AWeapon* DefaultWeapon = World->SpawnActor<AWeapon>(WeaponClass);
		DefaultWeapon->Equip(GetMesh(), FName("RightHandSocket"), this, this);
		EquippedWeapon = DefaultWeapon;
	}

	//FAIMoveRequest MoveRequest;
	//MoveRequest.SetGoalActor(PatrolTarget);
	//MoveRequest.SetAcceptanceRadius(15.f);
	//FNavPathSharedPtr NavPath;
	//EnemyController->MoveTo(MoveRequest, &NavPath);
	//TArray<FNavPathPoint>& PathPoints = NavPath->GetPathPoints();

	//for(auto& Point : PathPoints) {
	//	const FVector& PointLocation = Point.Location;
	//	DrawDebugSphere(GetWorld(), PointLocation, 12.f, 12, FColor::Red, false, 10.f);
	//}
}

void AEnemy::Die()
{	
	PlayDeathMontage();
	HideHealthBar();
	DisableCapsule();
	SetLifeSpan(DeathLifeSpan);
}
 bool AEnemy::InTargetRange(AActor *Target, double Radius)
{	
	const double DistanceToTarget = (Target->GetActorLocation() - GetActorLocation()).Size();
	DRAW_SPHERE_SingleFrame(GetActorLocation());
	DRAW_SPHERE_SingleFrame(Target->GetActorLocation());
	return DistanceToTarget <= Radius;
}
 void AEnemy::MoveToTarget(AActor* Target)
 {
	if (EnemyController == nullptr || Target == nullptr) return;
	FAIMoveRequest MoveRequest;
	MoveRequest.SetGoalActor(PatrolTarget);
	MoveRequest.SetAcceptanceRadius(60.f);
	EnemyController->MoveTo(Target);
 }
 AActor* AEnemy::ChoosePatrolTarget()
 {
	 if (InTargetRange(PatrolTarget, PatrolRadius)) {
		 TArray<AActor*> ValidTargets;
		 for (AActor* Target : PatrolTargets) {
			 if (Target != PatrolTarget) {
				 ValidTargets.AddUnique(Target);
			 }
		 }
		 const int32 NumPatrolTargets = ValidTargets.Num();

		 if (NumPatrolTargets > 0) {
			 const int32 TargetSelection = FMath::RandRange(0, NumPatrolTargets - 1);
			 return  ValidTargets[TargetSelection];
		 }
		 

	 }
	 return nullptr;
}
 void AEnemy::Attack()
 {
	 Super::Attack();
	 PlayAttackMontage();
	 //ActionState = EActionState::EAS_Attacking;
 }

 void AEnemy::PawnSeen(APawn* SeenPawn)
 {	
	 const bool bShouldChaseTarget =
		 EnemyState == EEnemyState::EES_Patrolling &&
		 SeenPawn->ActorHasTag("SlashCharacter");
	 
	 if (bShouldChaseTarget) {
		 CombatTarget = SeenPawn;
		 ClearPatrolTimer();
		 ChaseTarget();
	 }
 }
void AEnemy::PlayHitReactMontage(const FName& SectionName)
{
    UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if(AnimInstance && HitReactMontage) {
		AnimInstance->Montage_Play(HitReactMontage);
		AnimInstance->Montage_JumpToSection(SectionName, HitReactMontage);
	}
}

int32 AEnemy::PlayDeathMontage()
{	
	Super::PlayDeathMontage();
	const int32 Selection = Super::PlayDeathMontage();
	TEnumAsByte<EDeathPose> Pose(Selection);
	if (Pose < EDeathPose::EDP_Max) {
		DeathPose = Pose;
	}
	return Selection;
}

// Called every frame
void AEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (IsDead()) {
		//ClearPatrolTimer();
		//ClearAttackTimer();
		return;
	}
	if (EnemyState > EEnemyState::EES_Patrolling) {
		CheckCombatTarget();
	}
	else {
		CheckPatrolTarget();
	}
}

void AEnemy::CheckPatrolTarget()
{
	if (PatrolTarget && EnemyController) {
		if (InTargetRange(PatrolTarget, PatrolRadius)) {
			PatrolTarget = ChoosePatrolTarget();
			const float WaitTime = FMath::RandRange(WaitMin, WaitMax);
			GetWorldTimerManager().SetTimer(PatrolTimer, this, &AEnemy::PatrolTimerFinished, WaitTime);
		}
	}
}

void AEnemy::CheckCombatTarget()
{
	if (CombatTarget) {
		if (IsOutsideCombatRadius()) {
			LoseInterest();
			ClearAttackTimer();
			if(!IsEngaged()) StartPatrolling();
		}
		else if (IsOutsideAttackRadius() && IsChasing()) {
			ClearAttackTimer();
			if (!IsEngaged()) ChaseTarget();
		}
		else if (CanAttack()) {
			ClearAttackTimer();
			StartAttackTimer();
		}
	}
}

bool AEnemy::IsAttacking()
{
	return EnemyState == EEnemyState::EES_Attacking;
}

bool AEnemy::IsInsideAttackRadius()
{
	return InTargetRange(CombatTarget, attackRadius);
}

bool AEnemy::IsOutsideAttackRadius()
{
	return !InTargetRange(CombatTarget, attackRadius);
}

bool AEnemy::IsChasing()
{
	return EnemyState != EEnemyState::EES_Chasing;
}

bool AEnemy::IsAlive()
{
	return Attributes && Attributes->IsAlive();
}

bool AEnemy::IsDead() const
{
	return EnemyState == EEnemyState::EES_Dead;
}

void AEnemy::ChaseTarget()
{
	EnemyState = EEnemyState::EES_Chasing;
	GetCharacterMovement()->MaxWalkSpeed = ChasingSpeed;
	MoveToTarget(CombatTarget);
}

bool AEnemy::IsOutsideCombatRadius()
{
	return !InTargetRange(CombatTarget, combatRadius);
}

bool AEnemy::IsEngaged() const
{
	return EnemyState == EEnemyState::EES_Engaged;
}

void AEnemy::ClearPatrolTimer()
{
	GetWorldTimerManager().ClearTimer(PatrolTimer);
}

void AEnemy::ClearAttackTimer()
{
	GetWorldTimerManager().ClearTimer(AttackTimer);
}

bool AEnemy::CanAttack()
{
	return IsInsideAttackRadius() &&
		!IsAttacking() &&
		!IsDead();
}

void AEnemy::StartAttackTimer()
{
	EnemyState = EEnemyState::EES_Attacking;
	const float AttackTime = FMath::RandRange(AttackMin, AttackMax);
	GetWorldTimerManager().SetTimer(AttackTimer, this, &AEnemy::Attack, AttackTime);
}



void AEnemy::StartPatrolling()
{
	EnemyState = EEnemyState::EES_Patrolling;
	GetCharacterMovement()->MaxWalkSpeed = PatrollingSpeed;
	MoveToTarget(PatrolTarget);
}

void AEnemy::HideHealthBar()
{
	if (HealthBarWidget) {
		HealthBarWidget->SetVisibility(false);
	}
}

void AEnemy::LoseInterest()
{
	CombatTarget = nullptr;
	HideHealthBar();
}



void AEnemy::GetHit_Implementation(const FVector& ImpactPoint)
{
	// DRAW_SPHERE_COLOR(ImpactPoint, FColor::Orange);

	// DirectionalHitReact(ImpactPoint);

	ShowHealthBar();
	if(Attributes && Attributes->IsAlive()) {
		DirectionalHitReact(ImpactPoint);
	}else{
		Die();
	}
	PlayHitSound(ImpactPoint);
	SpawnHitParticles(ImpactPoint);

}

void AEnemy::ShowHealthBar()
{
	if (HealthBarWidget) {
		HealthBarWidget->SetVisibility(true);
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
	HandleDamage(DamageAmount);
	if (IsAlive()) {
		CombatTarget = EventInstigator->GetPawn();
		ChaseTarget();
	}
	else {
		EnemyState = EEnemyState::EES_Dead;
	}
	
    return DamageAmount;
}

void AEnemy::HandleDamage(float DamageAmount)
{
	Super::HandleDamage(DamageAmount);
	if (Attributes && HealthBarWidget) {
		HealthBarWidget->SetHealthPercent(Attributes->GetHealthPercent());
	}
}

void AEnemy::Destroyed()
{
	if (EquippedWeapon) {
		EquippedWeapon->Destroy();
	}
}
