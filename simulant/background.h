/* *   Copyright (c) 2011-2017 Luke Benstead https://simulant-engine.appspot.com
 *
 *     This file is part of Simulant.
 *
 *     Simulant is free software: you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation, either version 3 of the License, or
 *     (at your option) any later version.
 *
 *     Simulant is distributed in the hope that it will be useful,
 *     but WITHOUT ANY WARRANTY; without even the implied warranty of
 *     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *     GNU General Public License for more details.
 *
 *     You should have received a copy of the GNU General Public License
 *     along with Simulant.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef BACKGROUND_H
#define BACKGROUND_H

#include "types.h"

#include "generic/managed.h"
#include "generic/identifiable.h"

#include "interfaces/printable.h"
#include "interfaces/nameable.h"
#include "interfaces/updateable.h"

namespace smlt {

class BackgroundManager;

enum BackgroundResizeStyle {
    BACKGROUND_RESIZE_ZOOM,
    BACKGROUND_RESIZE_SCALE
};

class Background:
    public Managed<Background>,
    public generic::Identifiable<BackgroundID>,
    public Updateable,
    public Nameable,
    public Printable {

public:
    Background(BackgroundID background_id, BackgroundManager *manager);

    bool init() override;
    void cleanup() override;
    void update(double dt) override;

    void set_horizontal_scroll_rate(float x_rate);
    void set_vertical_scroll_rate(float y_rate);
    void set_texture(TextureID texture_id);
    void set_resize_style(BackgroundResizeStyle style);

    //Ownable interface
    void ask_owner_for_destruction();

    unicode to_unicode() const {
        return Nameable::to_unicode();
    }

private:
    BackgroundManager* manager_;

    void update_camera(const Viewport& viewport);

    StageID stage_id_;
    CameraID camera_id_;
    PipelineID pipeline_id_;
    MaterialID material_id_;
    ActorID actor_id_;

    BackgroundResizeStyle style_ = BACKGROUND_RESIZE_ZOOM;
    float x_rate_ = 0.0;
    float y_rate_ = 0.0;
};

}

#endif // BACKGROUND_H
