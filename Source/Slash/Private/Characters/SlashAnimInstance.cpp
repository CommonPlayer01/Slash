// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/SlashAnimInstance.h"
#include "Characters/SlashCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"

void USlashAnimInstance::NativeInitializeAnimation() {
	Super::NativeInitializeAnimation();
	SlashCharacter =  Cast<ASlashCharacter>(TryGetPawnOwner());
	if(SlashCharacter) {
		SlashCharacterMovement = SlashCharacter->GetCharacterMovement();
	}
	else {
		SlashCharacterMovement = nullptr;
	}
}

void USlashAnimInstance::NativeUpdateAnimation(float DeltaTime){
	Super::NativeUpdateAnimation(DeltaTime);
	if(SlashCharacterMovement) {
		//GroundSpeed = SlashCharacterMovement->Velocity.Size();
		GroundSpeed = UKismetMathLibrary::VSizeXY(SlashCharacterMovement->Velocity);
		isFalling = SlashCharacterMovement->IsFalling();
		CharacterState = SlashCharacter->GetCharacterState();
	}
	else {
		GroundSpeed = 0.0f;
	}

}