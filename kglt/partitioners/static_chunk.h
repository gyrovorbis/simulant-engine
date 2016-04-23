#pragma once

#include <limits>
#include "../generic/property.h"
#include "../mesh.h"

namespace kglt {

class StaticChunk;

struct Polygon:
    public BoundableEntity {

    VertexData* source_data = nullptr;
    std::vector<uint32_t> indices;
    AABB bounding_box;

    void recalc_bounds() {
        if(!source_data) {
            return;
        }

        bounding_box.min = Vec3(
            std::numeric_limits<float>::max(),
            std::numeric_limits<float>::max(),
            std::numeric_limits<float>::max()
        );

        bounding_box.max = Vec3(
            std::numeric_limits<float>::lowest(),
            std::numeric_limits<float>::lowest(),
            std::numeric_limits<float>::lowest()
        );

        for(auto& idx: indices) {
            auto& data = (*source_data).at(idx);
            auto pos = data.position;

            /* Iterate the X, Y and Z attributes and assign them if necessary */
            auto attrs = { &Vec3::x, &Vec3::y, &Vec3::z };

            for(auto& attr: attrs) {
                if(pos.*attr < bounding_box.min.*attr) bounding_box.min.*attr = pos.*attr;
                if(pos.*attr > bounding_box.max.*attr) bounding_box.max.*attr = pos.*attr;
            }
        }
    }

    const AABB aabb() const { return bounding_box; }
    const AABB transformed_aabb() const { return bounding_box; }
};

class StaticSubchunk:
    public Renderable {

public:
    typedef std::shared_ptr<StaticSubchunk> ptr;

    StaticSubchunk(StaticChunk* parent, RenderPriority priority, MaterialID material_id, MeshArrangement arrangement);
    ~StaticSubchunk() {
        mesh_->delete_submesh(submesh_->id());
    }

    const AABB aabb() const { return submesh_->aabb(); }
    const AABB transformed_aabb() const { return submesh_->aabb(); }
    const VertexData& vertex_data() const { return submesh_->vertex_data(); }
    const IndexData& index_data() const { return submesh_->index_data(); }
    const MeshArrangement arrangement() const { return submesh_->arrangement(); }

    void _update_vertex_array_object() { submesh_->_update_vertex_array_object(); }
    void _bind_vertex_array_object() { submesh_->_bind_vertex_array_object(); }

    RenderPriority render_priority() const { return render_priority_; }

    Mat4 final_transformation() const { return Mat4(); }
    const MaterialID material_id() const { return submesh_->material_id(); }
    const bool is_visible() const { return true; }
    MeshID instanced_mesh_id() const { return mesh_->id(); }
    SubMeshID instanced_submesh_id() const { return submesh_->id(); }

    void add_polygon(const Polygon&);
private:
    StaticChunk* parent_;
    Mesh* mesh_;
    RenderPriority render_priority_;
    SubMesh* submesh_;
};

class StaticChunk {
public:
    typedef std::tuple<RenderPriority, MaterialID, MeshArrangement> KeyType;
    typedef std::unordered_map<KeyType, StaticSubchunk::ptr> SubchunkLookup;

    StaticChunk(Stage* stage);

    typedef std::shared_ptr<StaticChunk> ptr;

    StaticSubchunk* get_or_create_subchunk(KeyType key);

    Property<StaticChunk, Mesh> mesh = { this, &StaticChunk::mesh_ };
    Property<StaticChunk, SubchunkLookup> subchunks = { this, &StaticChunk::subchunks_ };

private:
    SubchunkLookup subchunks_;
    Mesh mesh_;
};

struct StaticChunkHolder {
    typedef std::shared_ptr<StaticChunkHolder> ptr;

    std::unordered_map<GeomID, StaticChunk::ptr> chunks;
};



}
