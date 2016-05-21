#ifndef TEST_OCTREE_H
#define TEST_OCTREE_H

#include "kaztest/kaztest.h"

#include "kglt/kglt.h"
#include "global.h"

#include "kglt/partitioners/octree.h"
#include "kglt/types.h"

class OctreeTest : public KGLTTestCase {
public:
    void test_moving_objects() {
        kglt::Octree tree;

        Object obj(10, 10, 10);
        obj.set_centre(kglt::Vec3(0, 0, 0));

        tree.grow(&obj); //Insert the obj

        kglt::Vec3 expected_centre(0, 0, 0);
        assert_true(kmVec3AreEqual(&tree.root().centre(), &expected_centre));
        assert_equal(10, tree.root().strict_diameter());
        assert_equal(20, tree.root().loose_diameter());

        //CONTINUE: write tests for relocate
    }

    void test_insertion() {

        kglt::Octree tree;

        //Create an object 5 units high, centred a 10, 10, 10
        Object obj(2, 5, 2);
        obj.set_centre(kglt::Vec3(10, 10, 10));

        tree.grow(&obj);

        // The root node should have a strict diameter matching
        // the largest dimension of the object
        assert_equal(5, tree.root().strict_diameter());
        assert_equal(10, tree.root().loose_diameter());

        kmVec3 root_centre = tree.root().centre();
        kmVec3 obj_centre = obj.centre();
        assert_true(kmVec3AreEqual(&root_centre, &obj_centre));

        Object obj2(3, 3, 3); //Add a smaller object
        obj2.set_centre(kglt::Vec3(10, 10, 17));

        tree.grow(&obj2);
        /* This object was positioned outside the loose bounds of the
         * first one. This should have grown the tree upwards, the root
         * should now have 2 child nodes. The original root, and a new
         * child containing obj2
        */

        assert_equal(10, tree.root().strict_diameter());
        assert_equal(20, tree.root().loose_diameter());
        assert_equal((uint32_t)2, (uint32_t) tree.root().child_count());
        assert_close(12.5, tree.root().centre().z, 0.001);

        //Neither object should be in the root node
        assert_true(!tree.find(&obj).is_root());
        assert_true(!tree.find(&obj2).is_root());

        //And, they shouldn't be in the same node either
        assert_true(&tree.find(&obj) != &tree.find(&obj2));

        assert_equal(5, tree.find(&obj2).strict_diameter());
        assert_equal(10, tree.find(&obj2).loose_diameter());

        kmVec3 expected_centre;
        kmVec3Fill(&expected_centre, 10, 10, 15);
        assert_true(kmVec3AreEqual(&expected_centre, &tree.find(&obj2).centre()));

        //Root shouldn't have objects
        assert_true(!tree.root().has_objects());

        /** Situation Now
         *
         *  ---------------
         *  |      |      |
         *  |  o1  |  NE  |
         *  --------------
         *  |  o2  |      |
         *  |      |  NE  |
         *  ---------------
         *
         *  NE == Doesn't exist
         */

    }
private:
    class Object :
        public kglt::BoundableEntity {

    public:
        Object(float width, float height, float depth) {
            kmVec3Zero(&centre_);
            kmAABB3Initialize(&absolute_bounds_, nullptr, width, height, depth);
        }

        void set_bounds(float width, float height, float depth) {
            kmAABB3Initialize(&absolute_bounds_,
                &centre_,
                width,
                height,
                depth
            );
        }

        void set_centre(const kmVec3& centre) {
            //Reinitialize the AABB with the same dimensions but a different
            //central point.
            kmVec3Assign(&centre_, &centre);
            kmAABB3Initialize(&absolute_bounds_,
                &centre_,
                kmAABB3DiameterX(&absolute_bounds_),
                kmAABB3DiameterY(&absolute_bounds_),
                kmAABB3DiameterZ(&absolute_bounds_)
            );
        }

        const kglt::AABB transformed_aabb() const {
            return absolute_bounds_;
        }

        const kglt::AABB aabb() const {
            kglt::AABB local;
            kmAABB3Initialize(
                &local,
                nullptr,
                kmAABB3DiameterX(&absolute_bounds_),
                kmAABB3DiameterY(&absolute_bounds_),
                kmAABB3DiameterZ(&absolute_bounds_)
            );
            return local;
        }

        const kglt::Vec3 centre() const {
            return centre_;
        }
    private:
        kglt::AABB absolute_bounds_;
        kglt::Vec3 centre_;

    };
};


#endif // TEST_OCTREE_H
