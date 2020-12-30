#pragma once

#include <vector>
#include <functional>

#include "../generic/managed.h"
#include "../keycodes.h"

namespace smlt {

class InputState;
class InputAxis;

typedef std::function<void (InputAxis*)> EachAxisCallback;
typedef std::vector<InputAxis*> AxisList;

class InputManager:
    public RefCounted<InputManager> {

public:
    InputManager(InputState* controller);

    InputAxis* new_axis(const std::string& name);
    AxisList axises(const std::string& name) const;
    void each_axis(EachAxisCallback callback);
    void destroy_axises(const std::string& name);
    void destroy_axis(InputAxis* axis);
    std::size_t axis_count(const std::string& name) const;

    float axis_value(const std::string& name) const;
    int8_t axis_value_hard(const std::string& name) const;

    void update(float dt);

    /* Returns true if the axis was just pressed this frame */
    bool axis_was_pressed(const std::string& name) const;

    /* Returns true if the axis was just released this frame */
    bool axis_was_released(const std::string& name) const;

private:
    InputState* controller_;

    std::vector<std::shared_ptr<InputAxis>> axises_;

    std::unordered_map<std::string, bool> prev_axis_states_;
    std::unordered_map<std::string, bool> axis_states_;

    float _calculate_value(InputAxis* axis) const;

    bool _update_keyboard_axis(InputAxis* axis, float dt);
    bool _update_mouse_button_axis(InputAxis* axis, float dt);
    bool _update_joystick_button_axis(InputAxis* axis, float dt);
    void _update_mouse_axis_axis(InputAxis *axis, float dt);
    bool _update_joystick_axis_axis(InputAxis* axis, float dt);
    bool _update_joystick_hat_axis(InputAxis* axis, float dt);
};

}
