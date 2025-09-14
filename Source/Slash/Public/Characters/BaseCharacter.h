// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Interface/HitInterface.h"
#include "BaseCharacter.generated.h"


class AWeapon;
class UAnimMontage;
class UAttributeComponent;

UCLASS()
class SLASH_API ABaseCharacter : public ACharacter, public IHitInterface
{
	GENERATED_BODY()

public:
	ABaseCharacter();
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable)
	void SetWeaponCollisionEnabled(ECollisionEnabled::Type CollisionEnabled);


protected:
	virtual void BeginPlay() override;
	virtual void Attack();
	virtual void Die();

	/*
	*	Play Montage Functions
	*/
	virtual void PlayAttackMontage();
	void PlayHitReactMontage(const FName& SectionName);
	virtual void DirectionalHitReact(const FVector& ImpactPoint);
	void PlayHitSound(const FVector& ImpactPoint);
	void SpawnHitParticles(const FVector& ImpactPoint);
	virtual void HandleDamage(float DamageAmount);

	virtual bool CanAttack();
	bool IsAlive();

	UFUNCTION(BlueprintCallable)
	virtual void AttackEnd();

	UPROPERTY(VisibleAnywhere, Category = Weapon);
	AWeapon* EquippedWeapon;

	/*
		Animation Montages
	*/
	UPROPERTY(EditDefaultsOnly, Category = Montages)
	UAnimMontage* AttackMontage;

	UPROPERTY(EditDefaultsOnly, Category = Montages)
	UAnimMontage* HitReactMontage;

	/*
		Components
	*/
	UPROPERTY(VisibleAnywhere, Category = Attribute)
	UAttributeComponent* Attributes;



public:	

private:
	UPROPERTY(EditAnywhere, Category = Sounds)
	USoundWave* HitSound;

	UPROPERTY(EditAnywhere, Category = VisualsEffects)
	UParticleSystem* HitParticles;

};
