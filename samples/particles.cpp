
#include <simulant/simulant.h>

class MainScene : public smlt::Scene<MainScene> {
public:
    MainScene(smlt::Window* window):
        smlt::Scene<MainScene>(window) {}

    void load() {
        prepare_basic_scene(stage_, camera_);

        auto ps = stage_->new_particle_system_from_file("simulant/particles/fire.kglp");
        ps->move_to(0.0, 0, -4);

        camera_->set_perspective_projection(
            smlt::Degrees(45.0),
            float(window->width()) / float(window->height()),
            0.1,
            1000.0
        );

        L_DEBUG("Scene loaded");
    }

private:
    smlt::StagePtr stage_;
    smlt::CameraPtr camera_;
};

class App : public smlt::Application {
public:
    App(const smlt::AppConfig& config):
        smlt::Application(config) {

        window->set_logging_level(smlt::LOG_LEVEL_DEBUG);
    }

    bool init() {
        scenes->register_scene<MainScene>("main");
        return true;
    }
};

int main(int argc, char* argv[]) {
    smlt::AppConfig config;
    config.title = "Particles";
    config.fullscreen = false;
    config.width = 1024;
    config.height = 768;

    App app(config);
    return app.run();
}
