// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/SlashCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GroomComponent.h"
#include "Item/Item.h"
#include "Item/Weapons/Weapon.h"
#include "Animation/AnimMontage.h"

// Sets default values
ASlashCharacter::ASlashCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f); // Rotate at this rate

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetRootComponent());
	CameraBoom->TargetArmLength = 300.0f; // The camera follows at this distance behind the character

	ViewCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("ViewCamera"));
	ViewCamera->SetupAttachment(CameraBoom);

	Hair = CreateDefaultSubobject<UGroomComponent>(TEXT("Hair"));
	Hair->SetupAttachment(GetMesh());
	Hair->AttachmentName = FString("head");

	Eyebrows = CreateDefaultSubobject<UGroomComponent>(TEXT("Eyebrows"));
	Eyebrows->SetupAttachment(GetMesh());
	Eyebrows->AttachmentName = FString("head");

	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;
	bUseControllerRotationPitch = false;
}

// Called when the game starts or when spawned
void ASlashCharacter::BeginPlay()
{
	Super::BeginPlay();
	Tags.Add(FName("SlashCharacter"));
}

// Called every frame
void ASlashCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void ASlashCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis(FName("MoveForward"), this, &ASlashCharacter::MoveForward);
	PlayerInputComponent->BindAxis(FName("MoveRight"), this, &ASlashCharacter::MoveRight);
	PlayerInputComponent->BindAxis(FName("Turn"), this, &ASlashCharacter::Turn);
	PlayerInputComponent->BindAxis(FName("LookUp"), this, &ASlashCharacter::LookUp);

	PlayerInputComponent->BindAction(FName("Jump"), IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction(FName("Equip"), IE_Pressed, this, &ASlashCharacter::EKeyPressed);
	PlayerInputComponent->BindAction(FName("Attack"), IE_Pressed, this, &ASlashCharacter::Attack);

}

void ASlashCharacter::SetWeaponCollisionEnabled(ECollisionEnabled::Type CollisionEnabled) {
	if(EquippedWeapon && EquippedWeapon->GetWeaponBox()) {
		EquippedWeapon->GetWeaponBox()->SetCollisionEnabled(CollisionEnabled);
		EquippedWeapon->IgnoredActors.Empty(); // Clear the array after each attack
	}
}


void ASlashCharacter::MoveForward(float Value) {
	if (ActionState != EActionState::EAS_Unoccupied) return;
	if (Controller && Value != 0.0f)
	{
		//// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);
		//// get forward vector
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

		//const FVector Direction = GetActorForwardVector();
		AddMovementInput(Direction, Value);
	}
}

void ASlashCharacter::MoveRight(float Value) {
	if (ActionState != EActionState::EAS_Unoccupied) return;
	if (Controller && Value != 0.0f)
	{
		//const FVector Direction = GetActorRightVector();
		//AddMovementInput(Direction, Value); 
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0.f, Rotation.Yaw, 0.f);
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		AddMovementInput(Direction, Value); 

	}
}
void ASlashCharacter::Turn(float Value) {
	AddControllerYawInput(Value);

}
void ASlashCharacter::LookUp(float Value){
	AddControllerPitchInput(Value);
}

void ASlashCharacter::EKeyPressed() {

	AWeapon* OverlappingWeapon = Cast<AWeapon>(OverlappingItem);
	if(OverlappingWeapon && !EquippedWeapon) {
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("pickup()"));
		OverlappingWeapon->Equip(GetMesh(), FName("RightHandSocket"), this, this);

		CharacterState = ECharacterState::ECS_EquippedOneHandedWeapon;
		OverlappingItem = nullptr;
		EquippedWeapon = OverlappingWeapon;
	}
	else {
		if(CanDisarm()) {
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("Item Name: CanDisarm()"));
			playEquipMontage(FName("Unequip"));
			CharacterState = ECharacterState::ECS_Unequipped;
			ActionState = EActionState::EAS_EquippingWeapon;
		}else if (CanArm()) {
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("Item Name: CanArm()"));
			playEquipMontage(FName("Equip"));
			CharacterState = ECharacterState::ECS_EquippedOneHandedWeapon;
			ActionState = EActionState::EAS_EquippingWeapon;
		}
	}
}

void ASlashCharacter::playAttackMontage(){
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if(AnimInstance && AttackMontage) {
		AnimInstance->Montage_Play(AttackMontage);
		int32 Selection = FMath::RandRange(0, 1);
		FName SelectionName = FName();
		switch (Selection) {
			case 0:
				SelectionName = FName("Attack1");
				break;
			case 1:
				SelectionName = FName("Attack2");
				break;
			default:
				break;
		}
		AnimInstance->Montage_JumpToSection(SelectionName, AttackMontage);
	}
}


void ASlashCharacter::Attack() {
	/*if (CharacterState == ECharacterState::ECS_EquippedOneHandedWeapon) {
		CharacterState = ECharacterState::ECS_Attacking;
	}*/
	if (CanAttack()) {
		playAttackMontage();
		ActionState = EActionState::EAS_Attacking;
	}
}

void ASlashCharacter::AttackEnd() {
	ActionState = EActionState::EAS_Unoccupied;
}

bool ASlashCharacter::CanAttack() const{
	return ActionState == EActionState::EAS_Unoccupied && CharacterState != ECharacterState::ECS_Unequipped;
}


void ASlashCharacter::playEquipMontage(const FName& SectionName) {
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if(AnimInstance && EquipMontage) {
		AnimInstance->Montage_Play(EquipMontage);
		AnimInstance->Montage_JumpToSection(SectionName, EquipMontage);
	}
}

bool ASlashCharacter::CanDisarm() const {
	return ActionState == EActionState::EAS_Unoccupied && CharacterState != ECharacterState::ECS_Unequipped;
}
bool ASlashCharacter::CanArm() const {
	return ActionState == EActionState::EAS_Unoccupied && CharacterState == ECharacterState::ECS_Unequipped && EquippedWeapon;
}
void ASlashCharacter::Disarm() {
	if(EquippedWeapon) {
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("Item Name: Disarm()"));

		EquippedWeapon->AttachMeshToSocket(GetMesh(), FName("SpineSocket"));
	}
}
void ASlashCharacter::Arm() {
	if(EquippedWeapon) {
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("Item Name: Arm()"));

		EquippedWeapon->AttachMeshToSocket(GetMesh(), FName("RightHandSocket"));
	}
	
}

void ASlashCharacter::FinishEquipping() {
	ActionState = EActionState::EAS_Unoccupied;
}