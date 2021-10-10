
#pragma once

#include <map>
#include <queue>
#include "../../types.h"
#include "../../event_listener.h"
#include "ui_config.h"
#include "../../generic/containers/polylist.h"
#include "../stage_node.h"

namespace smlt {

class VirtualFileSystem;
class SharedAssetManager;

namespace ui {

class Widget;
class Button;
class Label;
class ProgressBar;
class Image;

}

template<typename PoolType, typename IDType, typename T, typename ...Subtypes>
class StageNodeManager;

typedef Polylist<
    StageNode,
    Actor, MeshInstancer, Camera, Geom, Light, ParticleSystem, Sprite,
    ui::Button, ui::Image, ui::Label, ui::ProgressBar,
    Skybox
> StageNodePool;

namespace ui {

typedef StageNodeManager<StageNodePool, WidgetID, Widget, Button, Label, ProgressBar, Image> WidgetManager;

enum UIEventType {
    UI_EVENT_TYPE_TOUCH
};

struct UIEvent {
    UIEvent(const TouchEvent& evt):
        type(UI_EVENT_TYPE_TOUCH),
        touch(evt) {}

    UIEventType type;
    union {
        TouchEvent touch;
    };
};

class UIManager:
    public EventListener {

    friend class Widget;

public:
    UIManager(Stage* stage, StageNodePool* pool, UIConfig config=UIConfig());
    virtual ~UIManager();

    Button* new_widget_as_button(const unicode& text, float width=.0f, float height=.0f);
    Label* new_widget_as_label(const unicode& text, float width=.0f, float height=.0f);
    ProgressBar* new_widget_as_progress_bar(float min=.0f, float max=100.0f, float value=.0f);
    Image* new_widget_as_image(const TexturePtr& texture);

    Widget* widget(WidgetID widget_id);

    void destroy_widget(WidgetID widget);

    Stage* stage() const { return stage_; }

    /* Implementation for TypedDestroyableObject (INTERNAL) */
    void destroy_object(Widget* object);
    void destroy_object_immediately(Widget* object);

    const UIConfig* config() const {
        return &config_;
    }
private:
    Stage* stage_ = nullptr;
    Window* window_ = nullptr;

    std::shared_ptr<WidgetManager> manager_;
    UIConfig config_;

    void on_touch_begin(const TouchEvent &evt) override;
    void on_touch_end(const TouchEvent &evt) override;
    void on_touch_move(const TouchEvent &evt) override;

    void queue_event(const TouchEvent& evt);
    void process_event_queue(const Camera *camera, const Viewport& viewport) const;
    void clear_event_queue();

    std::queue<UIEvent> queued_events_;

    WidgetPtr find_widget_at_window_coordinate(const Camera *camera, const Viewport& viewport, const Vec2& window_coord) const;

    sig::connection frame_finished_connection_;
    sig::connection pre_render_connection_;

    FontPtr load_or_get_font(
        const std::string& family, const Px& size, const FontWeight &weight
    );

private:
    friend class ::smlt::Window;
    static FontPtr _load_or_get_font(VirtualFileSystem* vfs,
        AssetManager* assets, AssetManager* shared_assets,
        const std::string& family, const Px& size, const FontWeight &weight
    );
};

}
}
