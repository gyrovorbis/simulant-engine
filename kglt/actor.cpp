#include "stage.h"
#include "actor.h"
#include "animation.h"
#include "renderers/renderer.h"
#include "hardware_buffer.h"


namespace kglt {

Actor::Actor(ActorID id, Stage* stage):
    generic::Identifiable<ActorID>(id),
    ParentSetterMixin<MoveableObject>(stage),
    Source(stage),
    render_priority_(RENDER_PRIORITY_MAIN) {

}

Actor::Actor(ActorID id, Stage* stage, MeshID mesh):
    generic::Identifiable<ActorID>(id),
    ParentSetterMixin<MoveableObject>(stage),
    Source(stage),
    render_priority_(RENDER_PRIORITY_MAIN) {

    set_mesh(mesh);
}

Actor::~Actor() {

}

void Actor::override_material_id(MaterialID mat) {
    for(SubActor::ptr se: subactors_) {
        se->override_material_id(mat);
    }
}

void Actor::remove_material_id_override() {
    for(SubActor::ptr se: subactors_) {
        se->remove_material_id_override();
    }
}

VertexData* Actor::get_shared_data() const {
    return mesh_->shared_data.get();
}

void Actor::clear_subactors() {
    for(auto& subactor: subactors_) {
        signal_subactor_destroyed_(id(), subactor.get());
    }

    subactors_.clear();
}

void SubActor::prepare_buffers() {
    if(submesh_) {
        submesh_->prepare_buffers();
    }
}

VertexSpecification SubActor::vertex_attribute_specification() const {
    auto* vertex_data = get_vertex_data();
    if(vertex_data) {
        return vertex_data->specification();
    }

    return VertexSpecification();
}

HardwareBuffer* SubActor::vertex_attribute_buffer() const {
    if(parent_.has_animated_mesh()) {
        return parent_.interpolated_vertex_buffer_.get();
    }

    return submesh_->vertex_buffer();
}

HardwareBuffer* SubActor::index_buffer() const {
    return submesh_->index_buffer();
}

std::size_t SubActor::index_element_count() const {
    return submesh_->index_data->count();
}


void Actor::rebuild_subactors() {
    clear_subactors();

    mesh_->each([&](const std::string& name, SubMesh* submesh) {
        /*
         * It's important that subactors hold a reference to their counterpart
         * submesh. Otherwise if a thread destroys a submesh while rendering is happening
         * we get random crashes. We called shared_from_this to keep the submesh around
         */
        subactors_.push_back(
            SubActor::create(*this, submesh->shared_from_this())
        );
        signal_subactor_created_(id(), subactors_.back().get());
    });
}

void Actor::set_mesh(MeshID mesh) {
    if(!mesh) {
        submesh_created_connection_.disconnect();
        submesh_destroyed_connection_.disconnect();
        clear_subactors();
        mesh_.reset();
        return;
    }

    // Do nothing if already set
    if(mesh_ && mesh_->id() == mesh) {
        return;
    }

    //Increment the ref-count on this mesh
    mesh_ = stage->assets->mesh(mesh)->shared_from_this();

    /* FIXME: This logic should also happen if the associated Mesh has set_animation_enabled called */
    if(mesh_ && mesh_->is_animated()) {
        using namespace std::placeholders;

        auto shared_data_size = mesh_->shared_data->count();
        auto shared_vertices_per_frame = shared_data_size / mesh_->animation_frames();

        // Create an interpolated vertex hardware buffer if this is an animated mesh
        interpolated_vertex_buffer_ = stage->window->renderer->hardware_buffers->allocate(
            mesh_->shared_data->specification().stride() * shared_vertices_per_frame,
            HARDWARE_BUFFER_VERTEX_ATTRIBUTES
        );

        animation_state_ = std::make_shared<KeyFrameAnimationState>(
            mesh_,
            std::bind(&Actor::refresh_animation_state, this, _1, _2, _3)
        );

        animation_state_->play_first_animation();
    }

    //Watch the mesh for changes to its submeshes so we can adapt to it
    submesh_created_connection_ = mesh_->signal_submesh_created().connect(
        [=](MeshID m, SubMesh* s) {
            rebuild_subactors();
        }
    );

    submesh_destroyed_connection_ = mesh_->signal_submesh_destroyed().connect(
        [=](MeshID m, SubMesh* s) {
            rebuild_subactors();
        }
    );

    //Rebuild the subactors to match the meshes submeshes
    rebuild_subactors();

    signal_mesh_changed_(id());
}

void Actor::do_update(double dt) {
    update_source(dt);
    if(animation_state_) {
        animation_state_->update(dt);
    }
}

void Actor::refresh_animation_state(uint32_t current_frame, uint32_t next_frame, float interp) {
    if(!interpolated_vertex_buffer_) {
        // The animation buffer hasn't been configured yet
        return;
    }

    assert(mesh_ && mesh_->is_animated());

    auto shared_data_size = mesh_->shared_data->count();
    if(shared_data_size) {
        assert(shared_data_size % mesh_->animation_frames() == 0);
        auto shared_vertices_per_frame = shared_data_size / mesh_->animation_frames();

        auto source_offset = shared_vertices_per_frame * animation_state_->current_frame();
        auto target_offset = shared_vertices_per_frame * animation_state_->next_frame();

        VertexData* source_data = mesh_->shared_data.get();

        VertexData target(source_data->specification());
        target.resize(shared_vertices_per_frame);

        for(uint32_t i = 0; i < shared_vertices_per_frame; ++i) {
            source_data->interp_vertex(
                source_offset + i,
                *source_data, target_offset + i,
                target, i,
                animation_state_->interp()
            );
        }
        target.done();

        interpolated_vertex_buffer_->upload(target);
    }
}


const AABB Actor::aabb() const {
    if(has_mesh()) {
        return mesh()->aabb();
    }

    return AABB();
}

const AABB Actor::transformed_aabb() const {
    AABB box = aabb(); //Get the untransformed one

    auto pos = absolute_position();
    kmVec3Add(&box.min, &box.min, &pos);
    kmVec3Add(&box.max, &box.max, &pos);
    return box;
}

void Actor::ask_owner_for_destruction() {
    stage->delete_actor(id());
}

SubActor::SubActor(Actor& parent, std::shared_ptr<SubMesh> submesh):
    parent_(parent),
    submesh_(submesh),
    material_(0) {

    submesh_material_changed_connection_ = submesh->signal_material_changed().connect(
        [=](SubMesh*, MaterialID old, MaterialID newM) {
            if(!material_) {
                // No material override, so fire that the subactor material
                // changed.
                parent_.signal_subactor_material_changed_(
                    parent_.id(),
                    this,
                    old,
                    newM
                );
            }
        }
    );
}

SubActor::~SubActor() {
    submesh_material_changed_connection_.disconnect();
}

const MaterialID SubActor::material_id() const {
    if(material_) {
        return material_->id();
    }

    return submesh()->material_id();
}

void SubActor::override_material_id(MaterialID material) {
    if(material == material_id()) {
        return;
    }
    auto old_material = material_id();

    if(material) {
        //Store the pointer to maintain the ref-count
        material_ = parent_.stage->assets->material(material);
    } else {
        // If we passed a zero material ID, then remove the
        // material pointer
        material_.reset();
    }

    //Notify that the subactor material changed
    parent_.signal_subactor_material_changed_(
        parent_.id(),
        this,
        old_material,
        material
    );
}

void SubActor::remove_material_id_override() {
    override_material_id(MaterialID());
}

MeshPtr Actor::mesh() const {
    return stage->assets->mesh(mesh_id());
}

SubMesh* SubActor::submesh() {
    if(!submesh_) {
        throw std::logic_error("Submesh was not initialized");
    }

    return submesh_.get();
}

const SubMesh *SubActor::submesh() const {
    if(!submesh_) {
        throw std::logic_error("Submesh was not initialized");
    }

    return submesh_.get();
}

void Actor::each(std::function<void (uint32_t, SubActor*)> callback) {
    uint32_t i = 0;
    for(auto subactor: subactors_) {
        callback(i++, subactor.get());
    }
}


VertexData* SubActor::get_vertex_data() const {
    if(parent_.has_animated_mesh()) {
        return parent_.shared_vertex_animation_buffer_;
    }

    return submesh()->vertex_data.get();
}

IndexData* SubActor::get_index_data() const {
    return submesh()->index_data.get();
}

}
