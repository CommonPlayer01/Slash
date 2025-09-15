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
	if (PawnSensing) PawnSensing->OnSeePawn.AddDynamic(this, &AEnemy::PawnSeen);
	InitializeEnemy();
	Tags.Add(FName("Enemy"));


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

void AEnemy::InitializeEnemy()
{
	EnemyController = Cast<AAIController>(GetController());
	MoveToTarget(PatrolTarget);
	HideHealthBar();
	SpawnDefaultWeapon();
}

void AEnemy::SpawnDefaultWeapon()
{
	UWorld* World = GetWorld();
	if (World && WeaponClass) {
		AWeapon* DefaultWeapon = World->SpawnActor<AWeapon>(WeaponClass);
		DefaultWeapon->Equip(GetMesh(), FName("RightHandSocket"), this, this);
		EquippedWeapon = DefaultWeapon;
	}
}

void AEnemy::Die()
{	
	EnemyState = EEnemyState::EES_Dead;
	ClearPatrolTimer();
	ClearAttackTimer();
	PlayDeathMontage();
	HideHealthBar();
	DisableCapsule();
	SetLifeSpan(DeathLifeSpan);
	GetCharacterMovement()->bOrientRotationToMovement = false;
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
	MoveRequest.SetAcceptanceRadius(130.f);
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


 void AEnemy::PawnSeen(APawn* SeenPawn)
 {	
	 const bool bShouldChaseTarget =
		 EnemyState == EEnemyState::EES_Patrolling &&
		 SeenPawn->ActorHasTag(FName("EngageableTarget"));
	 
	 if (bShouldChaseTarget) {
		 CombatTarget = SeenPawn;
		 ClearPatrolTimer();
		 ChaseTarget();
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
			const float WaitTime = FMath::RandRange(PatrolWaitMin, PatrolWaitMax);
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
	return InTargetRange(CombatTarget, AttackRadius);
}

bool AEnemy::IsOutsideAttackRadius()
{
	return !InTargetRange(CombatTarget, AttackRadius);
}

bool AEnemy::IsChasing()
{
	return EnemyState != EEnemyState::EES_Chasing;
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
	return !InTargetRange(CombatTarget, CombatRadius);
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

void AEnemy::Attack()
{
	EnemyState = EEnemyState::EES_Engaged;
	Super::Attack();
	PlayAttackMontage();
}

void AEnemy::AttackEnd()
{
	EnemyState = EEnemyState::EES_NoState;
	CheckCombatTarget();
}

bool AEnemy::CanAttack()
{
	return IsInsideAttackRadius() &&
		!IsAttacking() &&
		!IsEngaged() &&
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



void AEnemy::GetHit_Implementation(const FVector& ImpactPoint, AActor* Hitter)
{
	// DRAW_SPHERE_COLOR(ImpactPoint, FColor::Orange);

	// DirectionalHitReact(ImpactPoint);
	Super::GetHit_Implementation(ImpactPoint, Hitter);

	ShowHealthBar();



}

void AEnemy::ShowHealthBar()
{
	if (HealthBarWidget) {
		HealthBarWidget->SetVisibility(true);
	}
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
