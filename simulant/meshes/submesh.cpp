#include <cfloat>

#include "submesh.h"
#include "mesh.h"
#include "private.h"
#include "adjacency_info.h"

#include "../asset_manager.h"
#include "../window.h"

namespace smlt {

SubMesh::SubMesh(Mesh* parent, const std::string& name,
        MaterialPtr material, MeshArrangement arrangement, IndexType index_type):
    parent_(parent),
    arrangement_(arrangement) {

    set_name(name);

    index_data_ = new IndexData(index_type);

    if(!material) {
        //Set the material to the default one (store the pointer to increment the ref-count)
        material = parent_->asset_manager().clone_default_material();
    }

    set_material(material);

}

VertexData *SubMesh::get_vertex_data() const {
    return parent_->vertex_data.get();
}

IndexData* SubMesh::get_index_data() const {
    return index_data_;
}

void SubMesh::set_diffuse(const smlt::Colour& colour) {
    for(auto i: *index_data) {
        vertex_data->move_to(i);
        vertex_data->diffuse(colour);
    };

    vertex_data->done();
}

void SubMesh::reverse_winding() {
    if(arrangement_ != MESH_ARRANGEMENT_TRIANGLES) {
        assert(0 && "Not implemented");
    }

    auto original = index_data->all();

    index_data->clear();
    for(uint32_t i = 0; i < original.size() / 3; ++i) {
        index_data->index(original[i * 3]);
        index_data->index(original[(i * 3) + 2]);
        index_data->index(original[(i * 3) + 1]);
    }
    index_data->done();
}

/**
 * @brief SubMesh::recalc_bounds
 *
 * Recalculate the bounds of the submesh. This involves interating over all of the
 * vertices that make up the submesh and so is potentially quite slow.
 */
void SubMesh::_recalc_bounds(AABB& bounds) {
    float minx = FLT_MAX, miny = FLT_MAX, minz = FLT_MAX;
    float maxx = -FLT_MAX, maxy = -FLT_MAX, maxz = -FLT_MAX;

    if(!index_data_->count()) {
        bounds = AABB();
        return;
    }

    // Store a raw-pointer for performance
    VertexData* vdata = vertex_data.get();

    if(vdata->empty()) {
        /* MD2 frames don't necessarily have vertex data until they
         * have been animated */
        return;
    }

    auto& pos_attr = vdata->vertex_specification().position_attribute;

    /* Awful switching is for performance
       FIXME: Is there a better way to do this? I guess templated lambda or method
    */
    if(pos_attr == VERTEX_ATTRIBUTE_2F) {
        for(auto idx: *index_data_) {
            auto pos = vdata->position_at<Vec2>(idx);
            if(pos->x < minx) minx = pos->x;
            if(pos->y < miny) miny = pos->y;
            if(pos->x > maxx) maxx = pos->x;
            if(pos->y > maxy) maxy = pos->y;
        }
    } else if(pos_attr == VERTEX_ATTRIBUTE_3F) {
        for(auto idx: *index_data_) {
            auto pos = vdata->position_at<Vec3>(idx);
            if(pos->x < minx) minx = pos->x;
            if(pos->y < miny) miny = pos->y;
            if(pos->z < minz) minz = pos->z;
            if(pos->x > maxx) maxx = pos->x;
            if(pos->y > maxy) maxy = pos->y;
            if(pos->z > maxz) maxz = pos->z;
        }
    } else {
        assert(pos_attr == VERTEX_ATTRIBUTE_4F);

        for(auto idx: *index_data_) {
            auto pos = vdata->position_at<Vec4>(idx);
            if(pos->x < minx) minx = pos->x;
            if(pos->y < miny) miny = pos->y;
            if(pos->z < minz) minz = pos->z;
            if(pos->x > maxx) maxx = pos->x;
            if(pos->y > maxy) maxy = pos->y;
            if(pos->z > maxz) maxz = pos->z;
        }
    }

    bounds.set_min(Vec3(minx, miny, minz));
    bounds.set_max(Vec3(maxx, maxy, maxz));
}

void SubMesh::each_triangle(std::function<void (uint32_t, uint32_t, uint32_t)> cb) {
    if(arrangement_ == MESH_ARRANGEMENT_TRIANGLES) {
        assert(index_data_->count() % 3 == 0);
        for(uint32_t i = 0; i < index_data_->count(); i += 3) {
            cb(index_data_->at(i), index_data_->at(i + 1), index_data_->at(i + 2));
        }
    } else if(arrangement_ == MESH_ARRANGEMENT_TRIANGLE_STRIP) {
        assert(index_data_->count() >= 3);

        for(uint32_t i = 2; i < index_data_->count(); ++i) {
            cb(
                index_data_->at(i - 2),
                index_data_->at(i - 1),
                index_data_->at(i)
            );
        }
    } else if(arrangement_ == MESH_ARRANGEMENT_LINES) {
        for(uint32_t i = 0; i < index_data_->count(); i += 2) {
            // Pass the 3rd index the same as the first for lines
            cb(index_data_->at(i), index_data_->at(i + 1), index_data_->at(i));
        }
    } else if(arrangement_ == MESH_ARRANGEMENT_LINE_STRIP) {
        assert(index_data_->count() >= 2);

        for(uint32_t i = 1; i < index_data_->count(); ++i) {
            // Pass the 3rd index the same as the first for lines
            cb(
                index_data_->at(i - 1),
                index_data_->at(i),
                index_data_->at(i - 1)
            );
        }
    } else if(arrangement_ == MESH_ARRANGEMENT_TRIANGLE_FAN) {
        auto hub = index_data_->at(0);
        for(uint32_t i = 2; i < index_data_->count(); ++i) {
            cb(
                hub, index_data_->at(i - 1), index_data_->at(i)
            );
        }
    }
}

/**
 * @brief SubMesh::generate_texture_coordinates_cube
 * @param texture - which texture unit to generate coordinates for
 *
 * Generates cube coordinates.
 *
 * WARNING: Will not duplicate vertices. If you share vertices across polygons this
 * will currently not give correct results. In future a "duplicate_vertices" argument
 * might be added.
 */
void SubMesh::generate_texture_coordinates_cube(uint32_t texture) {
    AABB bounds;
    _recalc_bounds(bounds);

    auto& vd = vertex_data;

    vd->move_to_start();
    for(uint16_t i = 0; i < vd->count(); ++i) {
        auto v = vd->normal_at<Vec3>(i); // Get the vertex normal

        // Work out the component with the largest value
        float absx = std::fabs(v->x);
        float absy = std::fabs(v->y);
        float absz = std::fabs(v->z);

        bool x = (absx > absy && absx > absz);
        bool y = (absy > absx && absy > absz);

        // Generate a orthagonal direction vector

        smlt::Vec3 dir(0, 0, 0);

        if(x) {
            dir.x = (v->x < 0) ? -1 : 1;
        } else if(y) {
            dir.y = (v->y < 0) ? -1 : 1;
        } else {
            dir.z = (v->z < 0) ? -1 : 1;
        }

        // Create a plane at the origin with the opposite direction
        Plane plane(-dir.x, -dir.y, -dir.z, 0.0f);

        smlt::Vec3 v1 = *vd->position_at<Vec3>(i) - bounds.min();

        // Project the vertex position onto the plane
        smlt::Vec3 final = plane.project(v1);

        //Scale the final position on the plane by the size of the box
        // and subtract the lower corner so that everything is relative to 0,0,0
        // and scaled between 0 and 1
        final.x /= bounds.width();
        final.y /= bounds.height();
        final.z /= bounds.depth();

        // Finally, offset the uv coordinate to the right 'square' of the cubic texture
        if(x) {
            if(dir.x >= 0) {
                final.x = 2.0f / 3.0f + (final.y / 3.0f);
                final.y = 2.0f / 4.0f + (final.z / 4.0f);
            } else {
                final.x = final.y / 3.0f;
                final.y = 2.0f / 4.0f + (final.z / 4.0f);
            }
        } else if(y) {
            if(dir.y >= 0) {
                final.x = 1.0f / 3.0f + (final.z / 3.0f);
                final.y = 3.0f / 4.0f + (final.x / 4.0f);
            } else {
                final.x = 1.0f / 3.0f + (final.z / 3.0f);
                final.y = 1.0f / 4.0f + (final.x / 4.0f);
            }
        } else {
            if(dir.z >= 0) {
                final.x = 1.0f / 3.0f + (final.x / 3.0f);
                final.y = 2.0f / 4.0f + (final.y / 4.0f);
            } else {
                final.x = 1.0f / 3.0f + (final.x / 3.0f);
                final.y = (final.y / 4.0f);
            }
        }

        switch(texture) {
            case 0: vd->tex_coord0(final.x, final.y);
                break;
            case 1: vd->tex_coord1(final.x, final.y);
                break;
            case 2: vd->tex_coord2(final.x, final.y);
                break;
            case 3: vd->tex_coord3(final.x, final.y);
                break;
            default:
                break;
        }
        vd->move_next();
    }

    vd->done();

}

SubMesh::~SubMesh() {
    delete index_data_;
    index_data_ = nullptr;
}

void SubMesh::set_material(MaterialPtr material) {
    set_material_at_slot(MATERIAL_SLOT0, material);
}

void SubMesh::set_material_at_slot(MaterialSlot var, MaterialPtr mat) {
    auto old_material_id = (materials_[var]) ? materials_[var]->id() : MaterialID();

    if(old_material_id == mat->id()) {
        // Don't do anything, don't fire the changed signal
        return;
    }

    if(mat) {
        // Set the material, store the shared_ptr to increment the ref count
        materials_[var] = mat;
    } else {
        // Reset the material
        materials_[var].reset();
    }

    signal_material_changed_(this, var, old_material_id, mat->id());
    parent_->signal_submesh_material_changed_(
        parent_->id(),
        this,
        var,
        old_material_id,
        mat->id()
    );
}

MaterialPtr SubMesh::material() const {
    return materials_[MATERIAL_SLOT0];
}

MaterialPtr SubMesh::material_at_slot(MaterialSlot var, bool fallback) const {
    auto ret = materials_[var];
    if(!ret && fallback) {
        return materials_[MATERIAL_SLOT0];
    } else {
        return ret;
    }
}

SubMeshInterface::~SubMeshInterface() {

}


}
