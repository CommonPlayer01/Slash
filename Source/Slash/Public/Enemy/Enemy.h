// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Interface/HitInterface.h"
#include "Sound/SoundWave.h"
#include "Particles/ParticleSystem.h"
#include "Characters/CharacterTypes.h"
#include "Enemy.generated.h"

class UAnimMontage;
class UAttributeComponent;
class UHealthBarComponent;


UCLASS()
class SLASH_API AEnemy : public ACharacter, public IHitInterface
{
	GENERATED_BODY()

public:
	AEnemy();
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void GetHit_Implementation(const FVector& ImpactPoint) override;

	void DirectionalHitReact(const FVector& ImpactPoint);

	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser);


protected:
	virtual void BeginPlay() override;

	void Die();

	void PlayHitReactMontage(const FName& SectionName);

	UPROPERTY(BluePrintReadOnly)
	EDeathPose DeathPose = EDeathPose::EDP_Alive;
	

private:
	UPROPERTY(VisibleAnywhere, Category = Attribute, meta = (AllowPrivateAccess = "true"))
	UAttributeComponent* Attributes;

	UPROPERTY(VisibleAnywhere, Category = Attribute, meta = (AllowPrivateAccess = "true"))
	UHealthBarComponent* HealthBarWidget;

	UPROPERTY(EditDefaultsOnly, Category = Montages, meta = (AllowPrivateAccess = "true"))
	UAnimMontage* HitReactMontage;

	UPROPERTY(EditDefaultsOnly, Category = Montages, meta = (AllowPrivateAccess = "true"))
	UAnimMontage* DeathMontage;

	UPROPERTY(EditAnywhere, Category = Sounds)
	USoundWave* HitSound;

	UPROPERTY(EditAnywhere, Category = VisualsEffects)
	UParticleSystem* HitParticles;

	UPROPERTY()
	AActor* CombatTarget;

	UPROPERTY(EditAnywhere)
	double combatRadius = 500.f;

	/*
		Navigation
	*/

	UPROPERTY()
	class AAIController* EnemyController;

	UPROPERTY(EditInstanceOnly, Category ="AI Navigation")
	AActor* PatrolTarget;

	UPROPERTY(EditInstanceOnly, Category ="AI Navigation")
	TArray<AActor*> PatrolTargets;

};
