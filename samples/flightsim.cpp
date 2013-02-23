#include "kglt/kglt.h"
#include "kglt/shortcuts.h"
#include "kglt/kazbase/string.h"
#include "kglt/extra/skybox.h"

int main(int argc, char* argv[]) {        
    logging::get_logger("/")->add_handler(logging::Handler::ptr(new logging::StdIOHandler));

    if(argc < 2) {
        std::cout << "USAGE: flightsim filename" << std::endl;
        return 1;
    }

    std::string filename = argv[1];

    if(!str::contains(str::lower(filename), ".opt")) {
        std::cout << "Please specify a file with a .opt extension" << std::endl;
        return 2;
    }

    kglt::Window::ptr window = kglt::Window::create(1024, 768);
    window->set_title("Flight Sim Camera");

    kglt::SubScene& subscene = window->scene().subscene();
    subscene.set_ambient_light(kglt::Colour(1.0, 1.0, 1.0, 1.0));
    subscene.camera().set_perspective_projection(45.0, float(window->width()) / float(window->height()));

    kglt::Mesh& mesh = subscene.mesh(subscene.new_mesh());
    window->loader_for(filename)->into(mesh);

    kglt::Entity& entity = subscene.entity(subscene.new_entity(mesh.id()));
    entity.move_to(0, 0, -30);

    //Just stash the skybox along with the scene
    subscene.data().stash(kglt::extra::StarField::create(subscene), "skybox");

    //Set the camera to follow the model
    subscene.camera().follow(entity.id(), kglt::Vec3(0, 5, 50));


    //Connect keyboard signals
    window->keyboard().key_while_down_connect(kglt::KEY_CODE_LEFT, [=](kglt::KeyCode key) mutable {
        float dt = window->delta_time();
        window->scene().subscene().entity(entity.id()).rotate_y(-20.0 * dt);
    });

    window->keyboard().key_while_down_connect(kglt::KEY_CODE_RIGHT, [=](kglt::KeyCode key) mutable {
        float dt = window->delta_time();
        window->scene().subscene().entity(entity.id()).rotate_y(20.0 * dt);
    });

    window->keyboard().key_while_down_connect(kglt::KEY_CODE_UP, [=](kglt::KeyCode key) mutable {
        float dt = window->delta_time();
        window->scene().subscene().entity(entity.id()).rotate_x(20.0 * dt);
    });

    window->keyboard().key_while_down_connect(kglt::KEY_CODE_DOWN, [=](kglt::KeyCode key) mutable {
        float dt = window->delta_time();
        window->scene().subscene().entity(entity.id()).rotate_x(-20.0 * dt);
    });

    window->keyboard().key_while_down_connect(kglt::KEY_CODE_a, [=](kglt::KeyCode key) mutable {
        float dt = window->delta_time();
        window->scene().subscene().entity(entity.id()).rotate_z(-60.0 * dt);
    });

    window->keyboard().key_while_down_connect(kglt::KEY_CODE_d, [=](kglt::KeyCode key) mutable {
        float dt = window->delta_time();
        window->scene().subscene().entity(entity.id()).rotate_z(60.0 * dt);
    });

    /*if(window->joypad_count()) {
        window->joypad(0).axis_while_nonzero_connect(0, [=](kglt::AxisRange range, kglt::Axis) mutable {
            float dt = window->delta_time();
            window->scene().subscene().entity(entity.id()).rotate_y((range * 30) * dt);
        });

        window->joypad(0).axis_while_nonzero_connect(1, [=](kglt::AxisRange range, kglt::Axis) mutable {
            float dt = window->delta_time();
            window->scene().subscene().entity(entity.id()).rotate_x((range * -30) * dt);
        });
    }*/

    while(window->update()) {}

    return 0;
}

