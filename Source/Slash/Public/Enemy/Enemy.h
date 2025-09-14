// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Characters/BaseCharacter.h"
#include "Sound/SoundWave.h"
#include "Particles/ParticleSystem.h"
#include "Enemy.generated.h"

class UAttributeComponent;
class UHealthBarComponent;
class UPawnSensingComponent;


UCLASS()
class SLASH_API AEnemy : public ABaseCharacter
{
	GENERATED_BODY()

public:
	AEnemy();
	/** <AACtor> */
	virtual void Tick(float DeltaTime) override;
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser);
	virtual void Destroyed() override;
	/** <AACtor> */

	/** <IHitInterface>*/
	virtual void GetHit_Implementation(const FVector& ImpactPoint) override;
	/** <IHitInterface>*/





protected:
	/** <AActor> */
	virtual void BeginPlay() override;

	/** <AActor> */

	/** <ABaseCharacter> */
	void Die();
	virtual void Attack() override;
	virtual bool CanAttack() override;
	virtual void AttackEnd() override;
	virtual void HandleDamage(float DamageAmount) override;
	/** <ABaseCharacter> */

	UPROPERTY(BlueprintReadOnly)
	EEnemyState EnemyState = EEnemyState::EES_Patrolling;






	virtual int32 PlayDeathMontage() override;
	virtual void DirectionalHitReact(const FVector& ImpactPoint) override;
	bool InTargetRange(AActor* Target, double Radius);
	void MoveToTarget(AActor* Target);


private:

	/** AI Behavior*/
	void InitializeEnemy();
	void CheckPatrolTarget();
	void CheckCombatTarget();
	void PatrolTimerFinished();

	void ShowHealthBar();
	void HideHealthBar();
	void LoseInterest();
	void StartPatrolling();
	void ChaseTarget();
	bool IsInsideAttackRadius();
	bool IsOutsideAttackRadius();
	bool IsOutsideCombatRadius();
	bool IsChasing();
	bool IsAttacking();
	bool IsDead() const;
	bool IsEngaged() const;
	void StartAttackTimer();

	void ClearPatrolTimer();
	void ClearAttackTimer();
	AActor* ChoosePatrolTarget();
	void SpawnDefaultWeapon();



	UFUNCTION()
	void PawnSeen(APawn* SeenPawn);

	UPROPERTY(VisibleAnywhere, Category = Attribute, meta = (AllowPrivateAccess = "true"))
	UHealthBarComponent* HealthBarWidget;

	UPROPERTY(VisibleAnywhere)
	UPawnSensingComponent* PawnSensing;

	UPROPERTY(EditAnywhere)
	TSubclassOf<class AWeapon> WeaponClass;

	UPROPERTY(EditAnywhere)
	double CombatRadius = 500.f;

	UPROPERTY(EditAnywhere)
	double AttackRadius = 150.f;

	UPROPERTY()
	class AAIController* EnemyController;

	UPROPERTY(EditInstanceOnly, Category = "AI Navigation", BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	AActor* PatrolTarget;

	UPROPERTY(EditInstanceOnly, Category = "AI Navigation")
	TArray<AActor*> PatrolTargets;

	UPROPERTY(EditAnywhere)
	double PatrolRadius = 200.f;

	FTimerHandle PatrolTimer;

	UPROPERTY(EditAnywhere, Category = "AI Navigation")
	float PatrolWaitMin = 5.f;

	UPROPERTY(EditAnywhere, Category = "AI Navigation")
	float PatrolWaitMax = 10.f;

	UPROPERTY(EditAnywhere, Category = "Combat")
	float PatrollingSpeed = 125.f;

	FTimerHandle AttackTimer;

	UPROPERTY(EditAnywhere, Category = "Combat")
	float AttackMin = 0.5f;

	UPROPERTY(EditAnywhere, Category = "Combat")
	float AttackMax = 1.f;

	UPROPERTY(EditAnywhere, Category = "Combat")
	float ChasingSpeed = 400.f;

	UPROPERTY(EditAnywhere, Category = "Combat")
	float DeathLifeSpan = 3.f;
};
