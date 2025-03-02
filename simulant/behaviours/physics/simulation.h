#pragma once

#include "../../generic/managed.h"
#include "../../signals/signal.h"
#include "../../types.h"

#include "collider.h"


struct b3World;
struct b3Vec3;
struct b3Mat33;
struct b3Quat;
struct b3Body;

namespace smlt {

class TimeKeeper;

namespace behaviours {

namespace impl {
    class Body;
    class DynamicBody;
    class ContactListener;
}

typedef sig::signal<void ()> SimulationPreStepSignal;

struct RayCastResult {
    impl::Body* other_body = nullptr;

    float distance = std::numeric_limits<float>::infinity();
    smlt::Vec3 normal;
    smlt::Vec3 impact_point;
};

class RigidBodySimulation:
    public RefCounted<RigidBodySimulation> {

    DEFINE_SIGNAL(SimulationPreStepSignal, signal_simulation_pre_step);

public:
    RigidBodySimulation(TimeKeeper* time_keeper);
    ~RigidBodySimulation();

    bool init() override;
    void clean_up() override;

    void fixed_update(float step);

    smlt::optional<RayCastResult> ray_cast(
        const Vec3& start,
        const Vec3& direction,
        float max_distance=std::numeric_limits<float>::max()
    );

    void set_gravity(const Vec3& gravity);

    bool body_exists(const impl::Body* body) const { return bodies_.count((impl::Body*) body); }
private:
    friend class impl::Body;
    friend class impl::DynamicBody;

    friend class RigidBody;
    friend class StaticBody;
    friend class KinematicBody;

    TimeKeeper* time_keeper_ = nullptr;

    std::shared_ptr<b3World> scene_;
    std::shared_ptr<impl::ContactListener> contact_listener_;

    // Used by the RigidBodyBehaviour on creation/destruction to register a body
    // in the simulation
    b3Body *acquire_body(impl::Body* body);
    void release_body(impl::Body *body);

    /* Returns the Simulant body for a b3Body */
    impl::Body* get_associated_body(b3Body* b);

    std::unordered_map<impl::Body*, b3Body*> bodies_;

    std::pair<Vec3, Quaternion> body_transform(const impl::Body *body);
    void set_body_transform(impl::Body *body, const Vec3& position, const Quaternion& rotation);    
};

// FIXME: Rename the actual class
typedef RigidBodySimulation PhysicsSimulation;
typedef std::shared_ptr<RigidBodySimulation> PhysicsSimulationPtr;

}
}
