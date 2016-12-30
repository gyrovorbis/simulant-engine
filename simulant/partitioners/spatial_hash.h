#pragma once


#include "../partitioner.h"
#include "./impl/spatial_hash.h"
#include "../stage.h"
#include "../generic/threading/shared_mutex.h"

namespace smlt {

enum PartitionerEntryType {
    PARTITIONER_ENTRY_TYPE_LIGHT,
    PARTITIONER_ENTRY_TYPE_ACTOR,
    PARTITIONER_ENTRY_TYPE_GEOM,
    PARTITIONER_ENTRY_TYPE_PARTICLE_SYSTEM
};

struct PartitionerEntry : public SpatialHashEntry {
    PartitionerEntry(ActorID actor_id):
        type(PARTITIONER_ENTRY_TYPE_ACTOR),
        actor_id(actor_id) {}

    PartitionerEntry(LightID light_id):
        type(PARTITIONER_ENTRY_TYPE_LIGHT),
        light_id(light_id) {}

    PartitionerEntry(GeomID geom_id):
        type(PARTITIONER_ENTRY_TYPE_GEOM),
        geom_id(geom_id) {}

    PartitionerEntry(ParticleSystemID ps_id):
        type(PARTITIONER_ENTRY_TYPE_PARTICLE_SYSTEM),
        particle_system_id(ps_id) {}

    ~PartitionerEntry() {}

    PartitionerEntryType type;
    union {
        ActorID actor_id;
        LightID light_id;
        GeomID geom_id;
        ParticleSystemID particle_system_id;
    };
};

class SpatialHashPartitioner : public Partitioner {
public:
    SpatialHashPartitioner(Stage* ss);
    ~SpatialHashPartitioner();

    void add_actor(ActorID obj);
    void remove_actor(ActorID obj);

    void add_geom(GeomID geom_id);
    void remove_geom(GeomID geom_id);

    void add_light(LightID obj);
    void remove_light(LightID obj);

    void add_particle_system(ParticleSystemID ps);
    void remove_particle_system(ParticleSystemID ps);

    std::vector<LightID> lights_visible_from(CameraID camera_id);
    std::vector<RenderablePtr> geometry_visible_from(CameraID camera_id);

    void _update_actor(AABB bounds, ActorID actor);
private:
    SpatialHash* hash_ = nullptr;

    typedef std::shared_ptr<PartitionerEntry> PartitionerEntryPtr;

    std::unordered_map<ActorID, PartitionerEntryPtr> actor_entries_;
    std::unordered_map<LightID, PartitionerEntryPtr> light_entries_;
    std::unordered_map<ParticleSystemID, PartitionerEntryPtr> particle_system_entries_;

    shared_mutex lock_;

    std::unordered_map<ActorID, sig::connection> actor_updates_;
};

}
