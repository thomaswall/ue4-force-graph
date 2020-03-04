// Fill out your copyright notice in the Description page of Project Settings.


#include "KnowledgeNode.h"

// Sets default values
AKnowledgeNode::AKnowledgeNode()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
    
    
    MySphere = CreateDefaultSubobject<USphereComponent>(TEXT("Sphere Comp"));
    MySphere->SetHiddenInGame(false, true);
    RootComponent = MySphere;
    
    SphereMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SphereMesh"));
    SphereMesh->AttachToComponent(GetRootComponent(), FAttachmentTransformRules::SnapToTargetNotIncludingScale);
    //RootComponent->SetupAttachment(SphereMesh);

    static ConstructorHelpers::FObjectFinder<UStaticMesh>SphereMeshAsset(TEXT("StaticMesh'/Engine/BasicShapes/Sphere.Sphere'"));
    SphereMesh->SetStaticMesh(SphereMeshAsset.Object);
    
    SphereMesh->SetWorldScale3D(FVector(0.2, 0.2, 0.2));

}

// Called when the game starts or when spawned
void AKnowledgeNode::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AKnowledgeNode::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

