// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "KnowledgeNode.h"
#include "KnowledgeEdge.h"
#include "Math/GenericOctree.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "CoreMinimal.h"
#include "KnowledgeGraph.generated.h"

/**
 *
 */
struct NodeStrength {
    float strength;
    FVector direction;
    
    NodeStrength() {
        strength = 0.0;
        direction = FVector(0,0,0);
    }
};

struct FOctreeElement
{
    AKnowledgeNode* MyActor;
    float strength = 0.0;
    FBoxSphereBounds BoxSphereBounds;
    int id;

    FOctreeElement()
    {
        MyActor = nullptr;
        BoxSphereBounds = FBoxSphereBounds(FVector(0.0f, 0.0f, 0.0f), FVector(1.0f, 1.0f, 1.0f), 1.0f);
    }
};

struct FOctreeSematics
{
    enum { MaxElementsPerLeaf = 1 }; // 16
    enum { MinInclusiveElementsPerNode = 7 };
    enum { MaxNodeDepth = 12 };

    typedef TInlineAllocator<MaxElementsPerLeaf> ElementAllocator;
    typedef TOctree<FOctreeElement, FOctreeSematics> FOctree;

    /**
    * Get the bounding box of the provided octree element. In this case, the box
    * is merely the point specified by the element.
    *
    * @param    Element    Octree element to get the bounding box for
    *
    * @return    Bounding box of the provided octree element
    */
    FORCEINLINE static FBoxSphereBounds GetBoundingBox(const FOctreeElement& Element)
    {
        return Element.BoxSphereBounds;
    }

    FORCEINLINE static bool AreElementsEqual(const FOctreeElement& A, const FOctreeElement& B)
    {
        return A.MyActor == B.MyActor;
    }
    
    static void SetElementId(FOctreeSematics::FOctree& thisOctree, const FOctreeElement& Element, FOctreeElementId Id); //need to define this later

    FORCEINLINE static void ApplyOffset(FOctreeElement& Element, FVector Offset)
    {
        FVector NewPostion = Element.MyActor->GetActorLocation() + Offset;
        Element.MyActor->SetActorLocation(NewPostion);
        Element.BoxSphereBounds.Origin = NewPostion;
    }

};

class FSimpleOctree : public TOctree<FOctreeElement, FOctreeSematics>
{
    public:
        FSimpleOctree(const FVector& InOrigin,float InExtent);
        TMap<int32,FOctreeElementId> all_elements; //to keep track of foctreeelementid
};

UCLASS()
class FORCEGRAPH_API AKnowledgeGraph : public AActor
{
    GENERATED_BODY()
    
public:
    AKnowledgeGraph();
    ~AKnowledgeGraph();
    void AddNode(int32 id, AKnowledgeNode* kn, FVector location);
    void AddEdge(int32 id, int32 source, int32 target);
    void AddOctreeElement(const FOctreeElement& inNewOctreeElement);
    void InitNodes();
    void InitOctree(const FBox& inNewBounds);
    void InitForces();
    void RemoveElement(int key);
    void ApplyForces();
    void Accumulate();
    NodeStrength AddUpChildren(const FSimpleOctree::FNode& node, FString node_id);
    void ApplyManyBody(AKnowledgeNode* kn);
    void FindManyBodyForce(AKnowledgeNode* kn, const FSimpleOctree::FNode& node, const FOctreeNodeContext CurrentContext, FString node_id);
    virtual void Tick(float DeltaTime) override;
    
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Attributes)
    float alpha = 1;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Attributes)
    float iterations = 1;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Attributes)
    float alphaMin = 0.001;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Attributes)
    float alphaDecay = pow(alphaMin, 0.05);
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Attributes)
    float edgeDistance = 5;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Attributes)
    float nodeStrength = -30;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Attributes)
    float distancemin = 1000;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Attributes)
    float distancemax = 10000;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Attributes)
    float theta2 = 0.81;

protected:
    // Called when the game starts or when spawned
    virtual void BeginPlay() override;
    
private:
    TMap<int32, AKnowledgeNode*> all_nodes;
    TMap<int32, AKnowledgeEdge*> all_links;
    TMap<FString, NodeStrength> octree_node_strengths;
//    FVector GetWeightedDistance(FVector prev_loc);
    float alphaTarget = 0;
    float velocityDecay = 0.6;
    float initialRadius = 10;
    float initialAngle = PI * (3 - sqrt(5));
    bool init = false;
    
    FSimpleOctree* OctreeData;
};
