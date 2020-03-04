// Fill out your copyright notice in the Description page of Project Settings.


#include "KnowledgeEdge.h"
#include "Kismet/KismetMathLibrary.h"
#define print(text) if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 1.5, FColor::White,text)

// Sets default values
AKnowledgeEdge::AKnowledgeEdge()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
    
    MySphere = CreateDefaultSubobject<USphereComponent>(TEXT("Sphere Comp"));
    MySphere->SetHiddenInGame(false, true);
    RootComponent = MySphere;

}

// Called when the game starts or when spawned
void AKnowledgeEdge::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void AKnowledgeEdge::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AKnowledgeEdge::ChangeLoc(FVector _loc1, FVector _loc2)
{
    node1_loc = _loc1;
    node2_loc = _loc2;
    UpdateLoc();
}

void AKnowledgeEdge::UpdateLoc()
{
    FVector diff = node2_loc - node1_loc;
    float _distance = diff.Size() / 100; // have to narrow this down
    
//    print(diff.ToString());
//    print(FString::SanitizeFloat(_distance));
    FVector current_scale = FVector(0.05,0.05,_distance);
    //current_scale.Z *= _distance;
    SetActorScale3D(current_scale);
    
    FRotator new_rot = UKismetMathLibrary::FindLookAtRotation(node2_loc, node1_loc);//.Add(0,90,0);
    //TubeMesh->SetWorldRotation(FRotator(0,90,0) + new_rot);
    SetActorRotation(new_rot);
    
    SetActorLocation(node2_loc - diff * 0.5); //narrow this too
}

