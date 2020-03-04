// Fill out your copyright notice in the Description page of Project Settings.


#include "KnowledgeGraph.h"
#define print(text) if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 10, FColor::White,text)

FSimpleOctree::FSimpleOctree(const FVector& InOrigin,float InExtent) : TOctree(InOrigin,InExtent) {
}

void FOctreeSematics::SetElementId(FOctreeSematics::FOctree& thisOctree, const FOctreeElement& Element, FOctreeElementId Id)
{
    ((FSimpleOctree&)thisOctree).all_elements.Emplace(Element.MyActor->id, Id);
}

AKnowledgeGraph::AKnowledgeGraph()
{
    PrimaryActorTick.bCanEverTick = true;
}

AKnowledgeGraph::~AKnowledgeGraph()
{
    
}

void AKnowledgeGraph::BeginPlay() {
    Super::BeginPlay();
    InitOctree(FBox(FVector(-200, -200, -200), FVector(200, 200, 200)));
    
    //json crap
    const FString JsonFilePath = FPaths::ProjectContentDir() + "/data/graph.json";
    FString JsonString; //Json converted to FString
   
    FFileHelper::LoadFileToString(JsonString,*JsonFilePath);

    TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject());
    TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(JsonString);

    if (FJsonSerializer::Deserialize(JsonReader, JsonObject) && JsonObject.IsValid()) {

       //Retrieving an array property and printing each field
       TArray<TSharedPtr<FJsonValue>> jnodes = JsonObject->GetArrayField("nodes");
       for(int32 i = 0; i < jnodes.Num(); i++) {
           auto jobj = jnodes[i]->AsObject();
           int jid = jobj->GetIntegerField("id");
           AKnowledgeNode* kn = GetWorld()->SpawnActor<AKnowledgeNode>();
           AddNode(jid, kn, FVector(0,0,0));
       }
        
        TArray<TSharedPtr<FJsonValue>> jedges = JsonObject->GetArrayField("edges");
        for(int32 i = 0; i < jedges.Num(); i++) {
            auto jobj = jedges[i]->AsObject();
            int jid = jobj->GetIntegerField("id");
            int jsource = jobj->GetIntegerField("source");
            int jtarget = jobj->GetIntegerField("target");
            AddEdge(jid, jsource, jtarget);
        }
   }
}


void AKnowledgeGraph::AddNode(int32 id, AKnowledgeNode* kn, FVector location) {
    if(!all_nodes.Contains(id)) {
        kn->id = id;
        kn->strength = nodeStrength;
        all_nodes.Emplace(id, kn);
        FOctreeElement ote;
        ote.MyActor = kn;
        ote.strength = 1.0; // update with strength
        ote.BoxSphereBounds = FBoxSphereBounds(location, FVector(1.0f, 1.0f, 1.0f), 1.0f);
        AddOctreeElement(ote);
    }
    
}

void AKnowledgeGraph::AddOctreeElement(const FOctreeElement& inNewOctreeElement) {
    OctreeData->AddElement(inNewOctreeElement);
}
                    
void AKnowledgeGraph::AddEdge(int32 id, int32 source, int32 target) {
    UObject* SpawnClass = Cast<UObject>(StaticLoadObject(UObject::StaticClass(), NULL, TEXT("Blueprint'/Game/cylinder.cylinder'")));
    UBlueprint* GeneratedObj = Cast<UBlueprint>(SpawnClass);
    AKnowledgeEdge* e = GetWorld()->SpawnActor<AKnowledgeEdge>(GeneratedObj->GeneratedClass);
    e->source = source;
    e->target = target;
    e->strength = 1;//temp
    e->distance = edgeDistance;
    all_links.Emplace(id, e);
}

void AKnowledgeGraph::InitNodes() {
    for(auto &node : all_nodes) {
        float radius = initialRadius * sqrt(node.Key);
        float angle = node.Key * initialAngle;
        FVector init_pos = FVector(cos(angle), sin(angle), tan(angle)) * radius;
        node.Value->SetActorLocation(init_pos, false);
//        print("init position");
//        print(node.Value->GetActorLocation().ToString());
        node.Value->velocity = FVector(0,0,0);
    }
}

void AKnowledgeGraph::InitOctree(const FBox& inNewBounds) {
    //OctreeData = new FSimpleOctree(FVector(0.0f, 0.0f, 0.0f), 100.0f);
    OctreeData = new FSimpleOctree(inNewBounds.GetCenter(), inNewBounds.GetExtent().GetMax());
}

void AKnowledgeGraph::InitForces() {
    //link forces
    float n = all_nodes.Num();
    float m = all_links.Num();

    for (auto& link : all_links) {
        all_nodes[link.Value->source]->numberOfConnected += 1;
        all_nodes[link.Value->target]->numberOfConnected += 1;
    }
    
    for (auto& link : all_links) {
        float bias = all_nodes[link.Value->source]->numberOfConnected / (all_nodes[link.Value->source]->numberOfConnected + all_nodes[link.Value->target]->numberOfConnected);
        link.Value->bias = bias > 0.5 ? (1 - bias) * 0.5 + bias : bias * 0.5;
        link.Value->strength = 1.0 / fmin(all_nodes[link.Value->source]->numberOfConnected, all_nodes[link.Value->target]->numberOfConnected);
    }
    
    //charge forces
    for (auto& node : all_nodes) {
        node.Value->strength = node.Value->strength; // nothing for now
    }
    
    //center forces
    //nothing
    init = true;
}

void AKnowledgeGraph::RemoveElement(int key) {
    OctreeData->RemoveElement(OctreeData->all_elements[key]);
    all_nodes.Remove(key);
}

void AKnowledgeGraph::ApplyForces() {
    //link forces
    for (auto &link : all_links) {
        auto source_node = all_nodes[link.Value->source];
        auto target_node = all_nodes[link.Value->target];
        
        FVector source_pos = source_node->GetActorLocation();
        FVector source_velocity = source_node->velocity;
        
        FVector target_pos = target_node->GetActorLocation();
        FVector target_velocity = target_node->velocity;
        
        FVector new_v = target_pos + target_velocity - source_pos - source_velocity;
        float l = new_v.Size();
        l = (l - link.Value->distance) / l * alpha * link.Value->strength;
        new_v *= l;
        target_node->velocity -= new_v * (1 - link.Value->bias);
        source_node->velocity += new_v * (link.Value->bias);
        
        if(target_node->id == 7 && alpha > 0.2)
            print("LINK VEL: " + (-1 * new_v * (1 - link.Value->bias)).ToString());
        if(source_node->id == 7 && alpha > 0.2)
            print("LINK VEL: " + (new_v * (1 - link.Value->bias)).ToString());
    }
    
    //charge forces
    octree_node_strengths.Empty();
    for(auto &node : all_nodes) {
        int key = node.Key;
        auto kn = node.Value;
        RemoveElement(node.Key); //need to remove then update with new location when adding
        AddNode(key, kn, kn->GetActorLocation());
    }
    
    Accumulate();
    for(auto &node : all_nodes) {
        ApplyManyBody(node.Value);
    }
    
}

NodeStrength AKnowledgeGraph::AddUpChildren(const FSimpleOctree::FNode& node, FString node_id) {
    if(!octree_node_strengths.Contains(node_id)) {
        octree_node_strengths.Emplace(node_id,NodeStrength());
    }
    else {
        //reset
        octree_node_strengths[node_id].strength = 0;
        octree_node_strengths[node_id].direction = FVector(0,0,0);
    }
    
    if(!node.IsLeaf()) {
        int count = 0;
        float c = 0.0;
        float strength = 0.0;
        float weight = 0.0;
        FVector tempvec;
        FOREACH_OCTREE_CHILD_NODE(ChildRef) { //go find the leaves
            if (node.HasChild(ChildRef)) {
                NodeStrength ns = AddUpChildren(*node.GetChild(ChildRef), node_id + FString::FromInt(count)); //add up children
                //math for vector and strength
                c = abs(ns.strength);
                strength += ns.strength;
                weight += c;
                tempvec += c * ns.direction;
                
                count++;
            }
        }
        octree_node_strengths[node_id].strength = strength; //hash of ID of node for map
        octree_node_strengths[node_id].direction = tempvec / weight;
    }
    else {
        for (FSimpleOctree::ElementConstIt ElementIt(node.GetElementIt()); ElementIt; ++ElementIt) {
            //all the way down to elements
            const FOctreeElement& Sample = *ElementIt;
            octree_node_strengths[node_id].strength += Sample.MyActor->strength;
            octree_node_strengths[node_id].direction += Sample.MyActor->GetActorLocation();
        }
    }

    octree_node_strengths[node_id].direction.ToString(); //?
    return octree_node_strengths[node_id];
}

//get weights for every node before applying
void AKnowledgeGraph::Accumulate() {
    for (FSimpleOctree::TConstIterator<> NodeIt(*OctreeData); NodeIt.HasPendingNodes(); NodeIt.Advance()) {
        const FSimpleOctree::FNode& CurrentNode = NodeIt.GetCurrentNode();
        AddUpChildren(CurrentNode, "0");
        break;
    }
}

//use nodes to find velocity
void AKnowledgeGraph::FindManyBodyForce(AKnowledgeNode* kn, const FSimpleOctree::FNode& node, const FOctreeNodeContext CurrentContext, FString node_id) {
    NodeStrength ns = octree_node_strengths[node_id];

    //if no strength, ignore
//    if(ns.strength == 0)
//        return;
    
    const FBoxCenterAndExtent& CurrentBounds = CurrentContext.Bounds;
    FVector center = CurrentBounds.Center;
    FVector width = CurrentBounds.Extent;
    FVector dir = ns.direction - kn->GetActorLocation();
    float l = dir.Size()*dir.Size();
    
    //if size of current box is less than distance between nodes
    if(width.X * width.X / theta2 < l) {
//        print("GOING IN HERE");
        if(l < distancemax) {
            if(l < distancemin)
                l = sqrt(distancemin * l);
            if(kn->id == 7 && alpha > 0.2)
                print((dir * ns.strength * alpha / l).ToString());
            //print(FString::SanitizeFloat(ns.strength));
            float mult = pow(ns.strength / nodeStrength, 1.0);
            kn->velocity += dir * ns.strength * alpha / (l / 2.0) * mult;
        }
        return;
    }
    
    // if not leaf, get all children
    if(!node.IsLeaf() || l >= distancemax) {
        //recurse down this dude
//        print("IM NO LEAF");
        //print("NOT A LEAF");
        int count = 0;
        FOREACH_OCTREE_CHILD_NODE(ChildRef) {
            if (node.HasChild(ChildRef)) {
                FindManyBodyForce(kn,*node.GetChild(ChildRef), CurrentContext.GetChildContext(ChildRef), node_id + FString::FromInt(count));
                count++;
            }
        }
    } //if leaf and close, apply elements directly
    else if(node.IsLeaf()) {
        //print("IM LEAF");
        if(l < distancemin)
            l = sqrt(distancemin * l);
        for (FSimpleOctree::ElementConstIt ElementIt(node.GetElementIt()); ElementIt; ++ElementIt) {
            const FOctreeElement& Sample = *ElementIt;
            if(Sample.MyActor->id != kn->id) {
                dir = Sample.MyActor->GetActorLocation() - kn->GetActorLocation();
                l = dir.Size()*dir.Size();
                float mult = pow(Sample.MyActor->numberOfConnected, 3.0);
                if(kn->id == 7 && alpha > 0.2) {
                    print(FString::FromInt(Sample.MyActor->id));
                    print((dir * Sample.MyActor->strength * alpha / l * mult).ToString());
                }
                kn->velocity += dir * Sample.MyActor->strength * alpha / l * mult;
            }
        }
    }
    
    
}

void AKnowledgeGraph::ApplyManyBody(AKnowledgeNode* kn) {
    FVector dir;
    if(alpha > 0.2 && kn->id == 7)
        print("--------------------------------------");
    for (FSimpleOctree::TConstIterator<> NodeIt(*OctreeData); NodeIt.HasPendingNodes(); NodeIt.Advance()) {
        FindManyBodyForce(kn, NodeIt.GetCurrentNode(), NodeIt.GetCurrentContext(), "0");
        break;
    }
}
                    
void AKnowledgeGraph::Tick(float DeltaTime) {
    Super::Tick(DeltaTime);
    if(!init) {
        InitNodes();
        InitForces();
    }

    if(alpha < alphaMin)
        return;
                
    for(int i = 0; i < iterations; i ++) {
        alpha += (alphaTarget - alpha) * alphaDecay; //need to restart this if want to keep moving
        ApplyForces();
        
        for(auto &node: all_nodes) {
            auto kn = node.Value;
//            print("FINAL VELOCITY!");
//            print(kn->velocity.ToString());
            kn->velocity *= velocityDecay;
            if(kn->id == 7 && alpha > 0.2)
                print("FINAL VELOCITY: " + kn->velocity.ToString());
            kn->SetActorLocation(kn->GetActorLocation() + kn->velocity);
            kn->velocity *= 0; //reset velocities
            
//            print("FINAL POSITION!");
//            print(FString::FromInt(node.Key));
//            print(kn->GetActorLocation().ToString());
        }
        
        for(auto &link: all_links) {
            auto l = link.Value;
//            print("LOCCCCCC");
//            print(all_nodes[l->source]->GetActorLocation().ToString());
            l->ChangeLoc(all_nodes[l->source]->GetActorLocation(), all_nodes[l->target]->GetActorLocation());
        }
    }
                
}
