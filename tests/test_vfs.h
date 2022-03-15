#pragma once

#include "simulant/simulant.h"


namespace {

class VFSTests : public smlt::test::SimulantTestCase {
public:
    void test_vfs_caching() {
        auto vfs = application->vfs.get();
        vfs->clear_location_cache();
        assert_equal(vfs->location_cache_size(), 0);

        auto f0 = vfs->locate_file("simulant/textures/simulant-icon.png");
        assert_equal(vfs->location_cache_size(), 1);

        auto f1 = vfs->locate_file("simulant/textures/simulant-icon.png");
        assert_equal(vfs->location_cache_size(), 1);

        assert_true(f0);
        assert_true(f1);
        assert_equal(f0.value(), f1.value());

        // Should purge the cache
        vfs->add_search_path(".");
        assert_equal(vfs->location_cache_size(), 0);

        vfs->locate_file("simulant/textures/simulant-icon.png");
        assert_equal(vfs->location_cache_size(), 1);
    }
};

}
