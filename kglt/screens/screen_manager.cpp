#include "screen_manager.h"
#include "window_base.h"

namespace kglt {

ScreenManager::ScreenManager(WindowBase &window):
    window_(window) {

}

Screen::ptr ScreenManager::get_or_create_route(const unicode& route) {
    auto it = routes_.find(route);
    if(it == routes_.end()) {
        auto factory = screen_factories_.find(route);
        if(factory == screen_factories_.end()) {
            throw ValueError("No such route available: " + route);
        }

        routes_[route] = (*factory).second();
        it = routes_.find(route);
    }
    return it->second;
}

void ScreenManager::redirect(const unicode& route) {
    auto new_screen = get_or_create_route(route);

    new_screen->load();

    if(current_screen_) {
        current_screen_->deactivate();
    }

    std::swap(current_screen_, new_screen);
    current_screen_->activate();
}

void ScreenManager::background_load(const unicode& route) {
    auto screen = get_or_create_route(route);

    //Create a background task for loading the screen
    auto new_task = std::shared_ptr<BackgroundTask>(new BackgroundTask{
        route,
        std::async(std::launch::async, std::bind(&Screen::load, screen))
    });

    // Add an idle task to check for when the background task completes
    window_.idle().add([=]() -> bool {
        // Checks for complete or failed tasks
        auto status = new_task->future.wait_for(std::chrono::microseconds(0));
        if(status != std::future_status::ready) {
            return true; //Try again next frame
        }

        try {
            future.get();
        } catch(...) {
            L_ERROR("There was an error while loading route: " + route);
            throw;
        }

        return false;
    });
}

void ScreenManager::unload(const unicode& route) {
    auto it = routes_.find(route);
    if(it != routes_.end()) {
        it->second->unload();
    }
}

bool ScreenManager::is_loaded(const unicode& route) const {
    auto it = routes_.find(route);
    if(it == routes_.end()) {
        return false;
    } else {
        return it->second->is_loaded();
    }
}


}
