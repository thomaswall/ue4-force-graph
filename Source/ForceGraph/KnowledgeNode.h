// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "KnowledgeNode.generated.h"

struct ConnectedNode {
    AKnowledgeNode* node;
    float delay;
    FVector offset;
};

UCLASS()
class FORCEGRAPH_API AKnowledgeNode : public AActor
{
    GENERATED_BODY()
    
public:
    // Sets default values for this actor's properties
    AKnowledgeNode();

protected:
    // Called when the game starts or when spawned
    virtual void BeginPlay() override;

public:
    // Called every frame
    virtual void Tick(float DeltaTime) override;
    int numberOfConnected = 0;
    float maxDist = 200.0;
    float minDist = 80.0;
    UClass* MyClass;
    TArray<ConnectedNode*> connectedNodes;
    float spawnTime = 0.0;
    float strength = -30;
    FVector velocity;
    int id;
    
    UPROPERTY(VisibleAnywhere)
    class USphereComponent* MySphere;
    UStaticMeshComponent * SphereMesh;

};
