#pragma once

#include <vector>
#include <functional>

#include "../generic/managed.h"

namespace smlt {

class InputController;
class InputAxis;

typedef std::function<void (InputAxis*)> EachAxisCallback;
typedef std::vector<InputAxis*> AxisList;

class InputManager:
    public Managed<InputManager> {

public:
    InputManager(InputController* controller);

    InputAxis* new_axis(const std::string& name);
    AxisList axises(const std::string& name) const;
    void each_axis(EachAxisCallback callback);
    void delete_axises(const std::string& name);
    void delete_axis(InputAxis* axis);

    float axis_value(const std::string& name) const;

    void update(float dt);

private:
    InputController* controller_;

    std::vector<std::shared_ptr<InputAxis>> axises_;

    float _calculate_value(InputAxis* axis) const;
};

}
