// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Characters/BaseCharacter.h"
#include "Sound/SoundWave.h"
#include "Particles/ParticleSystem.h"
#include "Characters/CharacterTypes.h"
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
	virtual void Tick(float DeltaTime) override;
	void CheckPatrolTarget();
	void CheckCombatTarget();

	bool IsAttacking();

	bool IsInsideAttackRadius();

	void ChaseTarget();
	void StartPatrolling();

	bool IsOutsideAttackRadius();
	bool IsChasing();
	bool IsAlive();
	bool IsDead() const;
	bool IsOutsideCombatRadius();
	bool IsEngaged() const;

	void ClearPatrolTimer();
	void ClearAttackTimer();

	virtual bool CanAttack() override;

	/*
		Combat
	*/
	void StartAttackTimer();

	FTimerHandle AttackTimer;

	UPROPERTY(EditAnywhere, Category = "Combat")
	float AttackMin = 0.5f;

	UPROPERTY(EditAnywhere, Category = "Combat")
	float AttackMax = 1.f;



	virtual void GetHit_Implementation(const FVector& ImpactPoint) override;

	void ShowHealthBar();
	void HideHealthBar();
	void LoseInterest();

	virtual void DirectionalHitReact(const FVector& ImpactPoint) override;

	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser);

	virtual void HandleDamage(float DamageAmount) override;

	virtual void Destroyed() override;
protected:
	virtual void BeginPlay() override;

	void Die();
	bool InTargetRange(AActor* Target, double Radius);
	void MoveToTarget(AActor* Target);
	AActor* ChoosePatrolTarget();
	virtual void Attack() override;
	virtual void PlayAttackMontage() override;

	UFUNCTION()
	void PawnSeen(APawn* SeenPawn);

	void PlayHitReactMontage(const FName& SectionName);

	UPROPERTY(BluePrintReadOnly)
	EDeathPose DeathPose;

	UPROPERTY(BlueprintReadOnly)
	EEnemyState EnemyState = EEnemyState::EES_Patrolling;

	UPROPERTY(EditAnywhere, Category = "Combat")
	float PatrollingSpeed = 125.f;

	UPROPERTY(EditAnywhere, Category = "Combat")
	float ChasingSpeed = 400.f;

private:


	UPROPERTY(VisibleAnywhere, Category = Attribute, meta = (AllowPrivateAccess = "true"))
	UHealthBarComponent* HealthBarWidget;

	UPROPERTY(VisibleAnywhere)
	UPawnSensingComponent* PawnSensing;

	UPROPERTY(EditAnywhere)
	TSubclassOf<class AWeapon> WeaponClass;


	UPROPERTY(EditDefaultsOnly, Category = Montages, meta = (AllowPrivateAccess = "true"))
	UAnimMontage* DeathMontage;



	UPROPERTY()
	AActor* CombatTarget;

	UPROPERTY(EditAnywhere)
	double combatRadius = 500.f;

	UPROPERTY(EditAnywhere)
	double attackRadius = 150.f;

	/*
		Navigation
	*/
	UPROPERTY()
	class AAIController* EnemyController;

	UPROPERTY(EditInstanceOnly, Category = "AI Navigation", BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	AActor* PatrolTarget;

	UPROPERTY(EditInstanceOnly, Category ="AI Navigation")
	TArray<AActor*> PatrolTargets;

	UPROPERTY(EditAnywhere)
	double PatrolRadius = 200.f;

	FTimerHandle PatrolTimer;
	void PatrolTimerFinished();

	UPROPERTY(EditAnywhere, Category = "AI Navigation")
	float WaitMin = 5.f;

	UPROPERTY(EditAnywhere, Category = "AI Navigation")
	float WaitMax = 10.f;




};
