// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Characters/CharacterTypes.h"
#include "GameFramework/Character.h"
#include "Characters/BaseCharacter.h"
#include "SlashCharacter.generated.h"


class USpringArmComponent;
class UCameraComponent;
class UGroomComponent;
class AItem;
class AWeapon;
class UAnimMontage;



UCLASS()
class SLASH_API ASlashCharacter : public ABaseCharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ASlashCharacter();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;



protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	void MoveForward(float Value);
	void MoveRight(float Value);
	void Turn(float Value);
	void LookUp(float Value);
	void EKeyPressed();
	void Attack();

	// Animation Montage
	void playAttackMontage();
	void playEquipMontage(const FName& SectionName);

	UFUNCTION(BlueprintCallable)
	void AttackEnd();
	bool CanAttack() const;
	bool CanDisarm() const;
	bool CanArm() const;

	UFUNCTION(BlueprintCallable)
	void Disarm();
	UFUNCTION(BlueprintCallable)
	void Arm();

	UFUNCTION(BlueprintCallable)
	void FinishEquipping();


private:
	
	ECharacterState CharacterState = ECharacterState::ECS_Unequipped;

	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	EActionState ActionState = EActionState::EAS_Unoccupied;

	UPROPERTY(VisibleAnywhere);
	USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere);
	UCameraComponent* ViewCamera;

	UPROPERTY(VisibleAnywhere, Category = Hair);
	UGroomComponent* Hair;

	UPROPERTY(VisibleAnywhere, Category = Hair);
	UGroomComponent* Eyebrows;

	UPROPERTY(VisibleInstanceOnly);
	AItem* OverlappingItem;



	UPROPERTY(EditDefaultsOnly, Category = Montage)
	UAnimMontage* AttackMontage;

	UPROPERTY(EditDefaultsOnly, Category = Montage)
	UAnimMontage* EquipMontage;

public:
	FORCEINLINE void SetOverlappingItem(AItem* Item) { OverlappingItem = Item; }

	FORCEINLINE ECharacterState GetCharacterState() const { return CharacterState; }

	
};
