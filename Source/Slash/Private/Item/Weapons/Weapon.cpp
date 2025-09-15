// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/Weapons/Weapon.h"
#include "Characters/SlashCharacter.h"
#include "Components/SphereComponent.h"
#include "Components/BoxComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Interface/HitInterface.h"
#include "NiagaraComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Weapon.h"

AWeapon::AWeapon() {
	WeaponBox = CreateDefaultSubobject<UBoxComponent>(TEXT("WeaponBox"));
	WeaponBox->SetupAttachment(GetRootComponent());

	WeaponBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	//WeaponBox->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	WeaponBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);
	WeaponBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);

	BoxTraceStart = CreateDefaultSubobject<USceneComponent>(TEXT("Box Trace Start"));
	BoxTraceStart->SetupAttachment(GetRootComponent());
	BoxTraceEnd = CreateDefaultSubobject<USceneComponent>(TEXT("Box Trace End"));
	BoxTraceEnd->SetupAttachment(GetRootComponent());
}

void AWeapon::Equip(USceneComponent* InParent, FName InSocketName, AActor* NewOwner, APawn* NewInstigator) {
	SetOwner(NewOwner);
	SetInstigator(NewInstigator);
	AttachMeshToSocket(InParent, InSocketName);
	ItemState = EItemState::EIS_Equipped;
	
	// Sphere->SetGenerateOverlapEvents(false);
	// Sphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
	DisableSphereCollision();
	DeactivateEmbers();
	
}

void AWeapon::DeactivateEmbers() {
	if(EmbersEffect){
		EmbersEffect->Deactivate();
	}
}


void AWeapon::DisableSphereCollision() {
	if(Sphere){
		Sphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
}

void AWeapon::AttachMeshToSocket(USceneComponent* InParent, const FName& InSocketName)
{
	FAttachmentTransformRules TransformRules(EAttachmentRule::SnapToTarget, true);
	ItemMesh->AttachToComponent(InParent, TransformRules, InSocketName);
}



void AWeapon::BeginPlay() {
	Super::BeginPlay();

	WeaponBox->OnComponentBeginOverlap.AddDynamic(this, &AWeapon::OnBoxBeginOverlap);
}

void AWeapon::OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) {
	Super::OnSphereBeginOverlap(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);

	ASlashCharacter* SlashCharacter = Cast<ASlashCharacter>(OtherActor);
	
	FString ComponentName = OverlappedComponent->GetName();
	
	const FString OtherComponentName = OtherComp->GetName();
	


	if (SlashCharacter) {

		FString Message = FString::Printf(TEXT("Weapon %s Component Begin Overlap with %s %s"), *ComponentName, *SlashCharacter->GetName(), *OtherComponentName);
		//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, Message);
	}

}


void AWeapon::BoxTrace(FHitResult& BoxHit) {
	const FVector Start = BoxTraceStart->GetComponentLocation();
	const FVector End = BoxTraceEnd->GetComponentLocation();

	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(this);
	ActorsToIgnore.Add(GetOwner());

	for (AActor* Actor : IgnoreActors)
	{
		ActorsToIgnore.AddUnique(Actor);
	}

	UKismetSystemLibrary::BoxTraceSingle(
		this,
		Start,
		End,
		BoxTraceExtent,
		BoxTraceStart->GetComponentRotation(),
		ETraceTypeQuery::TraceTypeQuery1,
		false,
		ActorsToIgnore,
		bShowBoxDebug ? EDrawDebugTrace::ForDuration : EDrawDebugTrace::None,
		BoxHit,
		true
	);
	IgnoreActors.AddUnique(BoxHit.GetActor());
}

void AWeapon::OnBoxBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) {
	//Debug
	FString Message = FString::Printf(TEXT("Weapon Box Overlapping With %s %s"), *OtherActor->GetName(), *OtherComp->GetName());
	//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, Message);
	if (GetOwner()->ActorHasTag(TEXT("Enemy")) && OtherActor->ActorHasTag(TEXT("Enemy"))) {
		return;
	}

	FHitResult BoxHit;
	BoxTrace(BoxHit);
	if (BoxHit.GetActor()) {
		UGameplayStatics::ApplyDamage(BoxHit.GetActor(), Damage, GetInstigator()->GetController(), this, UDamageType::StaticClass());
		ExecuteGetHit();
		CreateFields(BoxHit.ImpactPoint);
	}
}
void AWeapon::ExecuteGetHit(FHitResult& BoxHit) {
	IHitInterface* HitInterface = Cast<IHitInterface>(BoxHit.GetActor());
	if (HitInterface)
	{
		HitInterface->Execute_GetHit(BoxHit.GetActor(), BoxHit.ImpactPoint, GetOwner());
	}
}
void AWeapon::OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex) {
	Super::OnSphereEndOverlap(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex);
}