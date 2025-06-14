#include "physics.hpp"
#include <Jolt/RegisterTypes.h>
#include <Jolt/Core/Factory.h>
#include <Jolt/Physics/Collision/BroadPhase/BroadPhaseLayer.h>
#include <Jolt/Physics/Collision/ObjectLayer.h>
#include <iostream>
#include <cstdarg>

using namespace JPH;
using namespace std;

TempAllocatorImpl* tempAllocator = nullptr;
JobSystemThreadPool* jobSystem = nullptr;
PhysicsSystem* physicsSystem = nullptr;

// Simple broad phase layer interface
class BPLayerInterfaceImpl final : public BroadPhaseLayerInterface
{
public:
    BPLayerInterfaceImpl()
    {
        // Create a mapping table from object to broad phase layer
        mObjectToBroadPhase[Layers::NON_MOVING] = BroadPhaseLayers::NON_MOVING;
        mObjectToBroadPhase[Layers::MOVING] = BroadPhaseLayers::MOVING;
    }

    virtual uint GetNumBroadPhaseLayers() const override
    {
        return BroadPhaseLayers::NUM_LAYERS;
    }

    virtual BroadPhaseLayer GetBroadPhaseLayer(ObjectLayer inLayer) const override
    {
        JPH_ASSERT(inLayer < Layers::NUM_LAYERS);
        return mObjectToBroadPhase[inLayer];
    }

#if defined(JPH_EXTERNAL_PROFILE) || defined(JPH_PROFILE_ENABLED)
    virtual const char* GetBroadPhaseLayerName(BroadPhaseLayer inLayer) const override
    {
        switch ((BroadPhaseLayer::Type)inLayer)
        {
        case (BroadPhaseLayer::Type)BroadPhaseLayers::NON_MOVING: return "NON_MOVING";
        case (BroadPhaseLayer::Type)BroadPhaseLayers::MOVING:     return "MOVING";
        default: JPH_ASSERT(false); return "INVALID";
        }
    }
#endif

private:
    BroadPhaseLayer mObjectToBroadPhase[Layers::NUM_LAYERS];
};

// Simple object vs broad phase layer filter
class ObjectVsBroadPhaseLayerFilterImpl : public ObjectVsBroadPhaseLayerFilter
{
public:
    virtual bool ShouldCollide(ObjectLayer inLayer1, BroadPhaseLayer inLayer2) const override
    {
        switch (inLayer1)
        {
        case Layers::NON_MOVING:
            return inLayer2 == BroadPhaseLayers::MOVING;
        case Layers::MOVING:
            return true;
        default:
            JPH_ASSERT(false);
            return false;
        }
    }
};

// Simple object vs object layer filter
class ObjectLayerPairFilterImpl : public ObjectLayerPairFilter
{
public:
    virtual bool ShouldCollide(ObjectLayer inObject1, ObjectLayer inObject2) const override
    {
        switch (inObject1)
        {
        case Layers::NON_MOVING:
            return inObject2 == Layers::MOVING;
        case Layers::MOVING:
            return true;
        default:
            JPH_ASSERT(false);
            return false;
        }
    }
};

// Global instances
static BPLayerInterfaceImpl broad_phase_layer_interface;
static ObjectVsBroadPhaseLayerFilterImpl object_vs_broadphase_layer_filter;
static ObjectLayerPairFilterImpl object_vs_object_layer_filter;

// Trace & Assert
static void TraceImpl(const char* inFMT, ...)
{
    va_list list;
    va_start(list, inFMT);
    char buffer[1024];
    vsnprintf(buffer, sizeof(buffer), inFMT, list);
    va_end(list);
    cout << buffer << endl;
}

#ifdef JPH_ENABLE_ASSERTS
static bool AssertFailedImpl(const char* inExpression, const char* inMessage, const char* inFile, uint inLine)
{
    cout << inFile << ":" << inLine << ": (" << inExpression << ") " << (inMessage ? inMessage : "") << endl;
    return true;
}
#endif

void InitJolt()
{
    // Register allocation hook
    RegisterDefaultAllocator();

    // Install callbacks
    Trace = TraceImpl;
    JPH_IF_ENABLE_ASSERTS(AssertFailed = AssertFailedImpl;)

    // Create a factory
    Factory::sInstance = new Factory();

    // Register all Jolt physics types
    RegisterTypes();

    // Create temp allocator
    tempAllocator = new TempAllocatorImpl(10 * 1024 * 1024);

    // Create job system
    jobSystem = new JobSystemThreadPool(cMaxPhysicsJobs, cMaxPhysicsBarriers, thread::hardware_concurrency() - 1);

    // Create physics system
    physicsSystem = new PhysicsSystem();
    
    // Initialize the physics system
    const uint cMaxBodies = 1024;
    const uint cNumBodyMutexes = 0; // Autodetect
    const uint cMaxBodyPairs = 1024;
    const uint cMaxContactConstraints = 1024;
    
    physicsSystem->Init(cMaxBodies, cNumBodyMutexes, cMaxBodyPairs, cMaxContactConstraints,
                       broad_phase_layer_interface, object_vs_broadphase_layer_filter, object_vs_object_layer_filter);

    cout << "Jolt initialized" << endl;
}

void ShutdownJolt()
{
    if (physicsSystem)
    {
        delete physicsSystem;
        physicsSystem = nullptr;
    }
    
    if (jobSystem)
    {
        delete jobSystem;
        jobSystem = nullptr;
    }
    
    if (tempAllocator)
    {
        delete tempAllocator;
        tempAllocator = nullptr;
    }
    
    if (Factory::sInstance)
    {
        delete Factory::sInstance;
        Factory::sInstance = nullptr;
    }
    
    cout << "Jolt shut down" << endl;
}
