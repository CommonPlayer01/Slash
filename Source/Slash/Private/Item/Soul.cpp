// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/Soul.h"
#include "Interface/PickupInterface.h"

void ASoul::Tick(float DeltaTime)
{
}

void ASoul::BeginPlay()
{
	Super::BeginPlay();

}

void ASoul::OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{	
	const FString OtherActorName = OtherActor->GetName();
	const FString OtherComponentName = OtherComp->GetName();

	FString ComponentName = OverlappedComponent->GetName();
	if (GEngine) {

		FString Message = FString::Printf(TEXT("soul %s Component Begin Overlap with %s %s"), *ComponentName, *OtherActorName, *OtherComponentName);
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, Message);
		//UE_LOG(LogTemp, Warning, TEXT("Item Sphere Begin Overlap with %s"), *ComponentName);
	}

	IPickupInterface* PickupInterface = Cast<IPickupInterface>(OtherActor);
	if (PickupInterface)
	{
		PickupInterface->AddSouls(this);
		//SpawnPickupSystem();
		//SpawnPickupSound();
		Destroy();
	}
}
