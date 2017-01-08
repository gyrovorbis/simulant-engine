/* *   Copyright (c) 2011-2017 Luke Benstead https://simulant-engine.appspot.com
 *
 *     This file is part of Simulant.
 *
 *     Simulant is free software: you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation, either version 3 of the License, or
 *     (at your option) any later version.
 *
 *     Simulant is distributed in the hope that it will be useful,
 *     but WITHOUT ANY WARRANTY; without even the implied warranty of
 *     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *     GNU General Public License for more details.
 *
 *     You should have received a copy of the GNU General Public License
 *     along with Simulant.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef PARTITIONER_H
#define PARTITIONER_H

#include <memory>
#include <set>
#include <vector>

#include "generic/property.h"
#include "generic/managed.h"
#include "renderers/renderer.h"
#include "types.h"
#include "interfaces.h"

namespace smlt {

class SubActor;
class StaticChunk;
class StaticSubchunk;

enum StaticChunkChangeType {
    STATIC_CHUNK_CHANGE_TYPE_SUBCHUNK_CREATED,
    STATIC_CHUNK_CHANGE_TYPE_SUBCHUNK_DESTROYED
};

struct StaticSubchunkCreatedData {
    StaticSubchunk* subchunk = nullptr;
};

struct StaticSubchunkDestroyedData {
    StaticSubchunk* subchunk = nullptr;
};

struct StaticChunkChangeEvent {
    StaticChunkChangeType type;
    StaticSubchunkCreatedData subchunk_created;
    StaticSubchunkDestroyedData subchunk_destroyed;
};


class Partitioner:
    public Managed<Partitioner> {

public:
    Partitioner(Stage* ss):
        stage_(ss) {}

    virtual void add_particle_system(ParticleSystemID ps) = 0;
    virtual void remove_particle_system(ParticleSystemID ps) = 0;

    virtual void add_geom(GeomID geom_id) = 0;
    virtual void remove_geom(GeomID geom_id) = 0;

    virtual void add_actor(ActorID obj) = 0;
    virtual void remove_actor(ActorID obj) = 0;

    virtual void add_light(LightID obj) = 0;
    virtual void remove_light(LightID obj) = 0;

    virtual std::vector<LightID> lights_visible_from(CameraID camera_id) = 0;
    virtual std::vector<std::shared_ptr<Renderable>> geometry_visible_from(CameraID camera_id) = 0;

    typedef sig::signal<void (StaticChunk*)> StaticChunkCreated;
    typedef sig::signal<void (StaticChunk*)> StaticChunkDestroyed;
    typedef sig::signal<void (StaticChunk*, StaticChunkChangeEvent)> StaticChunkChanged;

    StaticChunkCreated& signal_static_chunk_created() { return signal_static_chunk_created_; }
    StaticChunkDestroyed& signal_static_chunk_destroyed() { return signal_static_chunk_destroyed_; }
    StaticChunkChanged& signal_static_chunk_changed() { return signal_static_chunk_changed_; }

    virtual MeshID debug_mesh_id() { return MeshID(); }
protected:
    Property<Partitioner, Stage> stage = { this, &Partitioner::stage_ };

    StaticChunkCreated signal_static_chunk_created_;
    StaticChunkDestroyed signal_static_chunk_destroyed_;
    StaticChunkChanged signal_static_chunk_changed_;
private:
    Stage* stage_;
};

}

#endif // PARTITIONER_H
