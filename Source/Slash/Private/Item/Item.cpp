// Fill out your copyright notice in the Description page of Project Settings.





#include "Item/Item.h"
#include "Slash/Slash.h"
#include "Slash/DebugMacros.h"
#include "Components/SphereComponent.h"
#include "Characters/SlashCharacter.h"
#include "Niagaracomponent.h"

// Sets default values
AItem::AItem()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	ItemMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ItemMesh"));
	RootComponent = ItemMesh;
	
	Sphere = CreateDefaultSubobject<USphereComponent>(TEXT("Sphere"));
	Sphere->SetupAttachment(RootComponent);

	EmbersEffect = CreateDefaultSubobject<UNiagaraComponent>(TEXT("Embers"));
	EmbersEffect->SetupAttachment(RootComponent);
}

// Called when the game starts or when spawned
void AItem::BeginPlay()
{
	Super::BeginPlay();
	UE_LOG(LogTemp, Warning, TEXT("Begin play called!"));

	Sphere->OnComponentBeginOverlap.AddDynamic(this, &AItem::OnSphereBeginOverlap);
	Sphere->OnComponentEndOverlap.AddDynamic(this, &AItem::OnSphereEndOverlap);

	////打印日志
	//if (GEngine) {
	//	FString Name = GetName();
	//	FString Message = FString::Printf(TEXT("Item %s has been spawned!"), *Name);
	//	GEngine->AddOnScreenDebugMessage(1, 60.f, FColor::Cyan, Message);
	//	UE_LOG(LogTemp, Warning, TEXT("Item Name: %s"), *Name);
	//}
	////画一个球体
	//FVector Location = GetActorLocation();
	//DRAW_SPHERE(Location);
	//DrawDebugSphere(GetWorld(), GetActorLocation(), 50.f, 24, FColor::Green, false, 30.f);

	//画一根线
	//UWorld* World = GetWorld();
	//FVector Location = GetActorLocation();
	//FVector Forward = GetActorForwardVector();
	//if(World){
	//	//DrawDebugLine(World, Location, Location + Forward * 100.f, FColor::Red, true, -1.0f, 0, 1.0f);
	//	//DrawDebugPoint(World, Location + Forward * 100.f, 15.0f, FColor::Cyan, true);
	//	
	//}

	//DRAW_LINE(Location, Location +  Forward * 100.f);
	//DRAW_POINT(Location + Forward * 100.f);
	//DRAW_VECTOR(Location, Location + Forward * 100.f);
}
 
// Called every frame
void AItem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	float MovementRate = 50.f; // units per second

	RunningTime += DeltaTime;
	
	if(ItemState == EItemState::EIS_Hovering) {
		float DeltaZ = TransformedSin();
		AddActorWorldOffset(FVector(0.f, 0.f, DeltaZ));
	}
	
	//
	//DRAW_SPHERE_SingleFrame(GetActorLocation());
	//DRAW_VECTOR_SingleFrame(GetActorLocation(), GetActorLocation() + GetActorForwardVector() * 100.f); 

}

//DECLARE_DYNAMIC_MULTICAST_SPARSE_DELEGATE_SixParams(FComponentBeginOverlapSignature, UPrimitiveComponent, OnComponentBeginOverlap, UPrimitiveComponent*, OverlappedComponent, AActor*, OtherActor, UPrimitiveComponent*, OtherComp, int32, OtherBodyIndex, bool, bFromSweep, const FHitResult&, SweepResult);


float AItem::TransformedSin() {

	return Amplitude * FMath::Sin(RunningTime * TimeConstant);
}
float AItem::TransformedCos() {
	return Amplitude * FMath::Cos(RunningTime * TimeConstant);
}

void AItem::OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	const FString OtherActorName = OtherActor->GetName();
	const FString OtherComponentName = OtherComp->GetName();

	FString ComponentName = OverlappedComponent->GetName();
	if(GEngine) {
		
		FString Message = FString::Printf(TEXT("AItem %s Component Begin Overlap with %s %s"), *ComponentName, *OtherActorName, *OtherComponentName);
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, Message);
		// UE_LOG(LogTemp, Warning, TEXT("Item Sphere Begin Overlap with %s"), *ComponentName);
	}

	ASlashCharacter* SlashCharacter = Cast<ASlashCharacter>(OtherActor);
	if (SlashCharacter) {
		//FString Message = FString::Printf(TEXT("Cast to SlashCharacter sucess"));
		//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, Message);
		SlashCharacter->SetOverlappingItem(this);
	}
}



//DECLARE_DYNAMIC_MULTICAST_SPARSE_DELEGATE_FourParams(FComponentEndOverlapSignature, UPrimitiveComponent, OnComponentEndOverlap, UPrimitiveComponent*, OverlappedComponent, AActor*, OtherActor, UPrimitiveComponent*, OtherComp, int32, OtherBodyIndex);
void AItem::OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	const FString OtherActorName = OtherActor->GetName();
	UE_LOG(LogTemp, Warning, TEXT("Sphere End Overlap"));

	if(GEngine) {
		/*FString Message = FString::Printf(TEXT("Sphere End Overlap with %s"), *OtherActorName);
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, Message);*/
		UE_LOG(LogTemp, Warning, TEXT("Sphere End Overlap with %s"), *OtherActorName);
	}
}