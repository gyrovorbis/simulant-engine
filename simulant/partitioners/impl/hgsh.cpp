#include "../../frustum.h"
#include "hgsh.h"

namespace smlt {

HGSH::HGSH(float min_cell_size, float max_cell_size):
    min_size_(min_cell_size),
    max_size_(max_cell_size) {

}


void HGSH::insert_object_for_box(const AABB &box, HGSHEntry *object) {
    auto cell_size = find_cell_size_for_box(box);

    auto xmin = int32_t(floor(box.min.x / cell_size));
    auto xmax = int32_t(floor(box.max.x / cell_size));
    auto ymin = int32_t(floor(box.min.y / cell_size));
    auto ymax = int32_t(floor(box.max.y / cell_size));
    auto zmin = int32_t(floor(box.min.z / cell_size));
    auto zmax = int32_t(floor(box.max.z / cell_size));

    for(int32_t x = xmin; x <= xmax; ++x) {
        for(int32_t y = ymin; y <= ymax; ++y) {
            for(int32_t z = zmin; z <= zmax; ++z) {
                auto hash = make_key(x, y, z);
                insert_object_for_hash(cell_size, hash, object);
            }
        }
    }
}

void HGSH::remove_object(HGSHEntry *object) {
    for(auto& buckethash: object->buckets()) {
        auto& hashspace = buckets_[buckethash.cell_size];
        auto hash_bucket = hashspace.find(buckethash.hash);
        if(hash_bucket != hashspace.end()) {
            auto bucket = (*hash_bucket).second;
            bucket->objects.erase(object);
        }
    }
    object->set_buckets(BucketHashList());
}

HGSHEntryList HGSH::find_objects_within_frustum(const Frustum &frustum) {
    // FIXME: This just builds an AABB around the frustum, for perspective
    // projections this likely isn't efficient (fine for ortho)

    const float min = std::numeric_limits<float>::lowest();
    const float max = std::numeric_limits<float>::max();

    AABB box;
    box.min.x = box.min.y = box.min.z = max;
    box.max.x = box.max.y = box.max.z = min;

    auto update_box = [&box](const Vec3& corner) {
        if(corner.x < box.min.x) box.min.x = corner.x;
        if(corner.y < box.min.y) box.min.y = corner.y;
        if(corner.z < box.min.z) box.min.z = corner.z;

        if(corner.x > box.max.x) box.max.x = corner.x;
        if(corner.y > box.max.y) box.max.y = corner.y;
        if(corner.z > box.max.z) box.max.z = corner.z;
    };

    for(auto& corner: frustum.near_corners()) {
        update_box(corner);
    }


    for(auto& corner: frustum.far_corners()) {
        update_box(corner);
    }

    return find_objects_within_box(box);
}

HGSHEntryList HGSH::find_objects_within_box(const AABB &box) {
    HGSHEntryList objects;

    for(auto& p: buckets_) {
        auto cell_size = p.first;

        auto xmin = int32_t(floor(box.min.x / cell_size));
        auto xmax = int32_t(floor(box.max.x / cell_size));
        auto ymin = int32_t(floor(box.min.y / cell_size));
        auto ymax = int32_t(floor(box.max.y / cell_size));
        auto zmin = int32_t(floor(box.min.z / cell_size));
        auto zmax = int32_t(floor(box.max.z / cell_size));

        auto& hashspace = buckets_[cell_size];

        for(int32_t x = xmin; x <= xmax; ++x) {
            for(int32_t y = ymin; y <= ymax; ++y) {
                for(int32_t z = zmin; z <= zmax; ++z) {
                    auto hash = make_key(x, y, z);

                    auto hash_bucket = hashspace.find(hash);
                    if(hash_bucket != hashspace.end()) {
                        auto bucket = hash_bucket->second;
                        for(auto& new_obj: bucket->objects) {
                            objects.insert(new_obj);
                        }
                    }
                }
            }
        }
    }

    return objects;
}

float HGSH::find_cell_size_for_box(AABB box) const {
    /*
     * We find the nearest hash size which is greater than double the max dimension of the
     * box. This increases the likelyhood that the object will not wastefully span cells
     */
    float i = min_size_;

    while(i < std::min(box.max_dimension() * 2.0f, max_size_)) {
        i *= 2.0;
    }

    return i;
}

void HGSH::insert_object_for_hash(float cell_size, Hash hash, HGSHEntry *entry) {    
    auto& hashspace = buckets_[cell_size];
    auto hash_bucket = hashspace.find(hash);

    std::shared_ptr<Bucket> bucket;
    if(hash_bucket == hashspace.end()) {
        bucket = std::make_shared<Bucket>();
        hashspace.insert(std::make_pair(hash, bucket));
    } else {
        bucket = hash_bucket->second;
    }

    bucket->objects.insert(entry);
    entry->push_bucket(BucketHash{cell_size, hash});
}



}
