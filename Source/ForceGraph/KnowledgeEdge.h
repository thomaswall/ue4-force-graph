// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "KnowledgeEdge.generated.h"

UCLASS()
class FORCEGRAPH_API AKnowledgeEdge : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
    AKnowledgeEdge();
    void UpdateLoc();
    void ChangeLoc(FVector _loc1, FVector _loc2);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
    FVector node1_loc;
    FVector node2_loc;
    
    UPROPERTY(VisibleAnywhere)
    class USphereComponent* MySphere;
    
    int32 source;
    int32 target;
    float bias;
    float strength;
    float distance;

};
