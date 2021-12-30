#include <vector>
#include "ui_manager.h"
#include "keyboard.h"
#include "button.h"
#include "frame.h"

namespace smlt {
namespace ui {

Keyboard::Keyboard(UIManager *owner, UIConfig *config):
    Widget(owner, config) {

    main_frame_ = owner->new_widget_as_frame("");
    main_frame_->set_parent(this);
    main_frame_->set_space_between(2);

    generate_alphabetical_layout();

    /* Highlight the first character */
    rows_[0]->packed_children()[0]->focus();
}

void Keyboard::clear() {
    for(auto& button: buttons_) {
        button.second->destroy();
    }

    for(auto& frame: rows_) {
        if(frame) {
            main_frame_->unpack_child(frame);
        }
        frame = nullptr;
    }
}

void Keyboard::generate_alphabetical_layout() {
    clear();

    std::vector<std::string> rows = {
        {"1234567890#!"},
        {"ABCDEFGHIJ+_"},
        {"KLMNOPQRST@-"},
        {"UVWXYZ.,<>?*"}
    };

    auto new_button = [this](std::string label) -> smlt::ui::Button* {
        auto btn = owner_->new_widget_as_button(label);
        btn->resize(32, 32);
        btn->set_padding(0);
        btn->set_border_width(0);
        btn->set_background_colour(smlt::ui::UIConfig().background_colour_);
        btn->signal_focused().connect(std::bind(&Keyboard::focus, this, btn));
        btn->signal_blurred().connect(std::bind(&Keyboard::unfocus, this, btn));
        return btn;
    };

    smlt::ui::Widget* prev = nullptr;

    for(std::size_t i = 0; i < 5; ++i) {
        auto& row = rows_[i];
        row = owner_->new_widget_as_frame("");
        row->set_anchor_point(0.5f, 0.5f);
        row->set_layout_direction(LAYOUT_DIRECTION_LEFT_TO_RIGHT);
        row->set_border_width(0);
        row->set_space_between(2);

        if(i < rows.size()) {
            prev = nullptr;
            for(auto ch: rows[i]) {
                buttons_[ch] = new_button(std::string(1, ch));
                if(prev) {
                    prev->set_focus_next(buttons_[ch]);
                }
                prev = buttons_[ch];
                row->pack_child(buttons_[ch]);
            }

            main_frame_->pack_child(row);
        }
    }

    /* Now, we add Backspace, space and return */
    buttons_[8] = new_button("<--");
    buttons_[8]->resize(32 * 2 + (2 * 1), 32);
    rows_[4]->pack_child(buttons_[8]);

    /* Now, we add space and return */
    buttons_[' '] = new_button("___");
    buttons_[' ']->resize(32 * 6 + (2 * 5), 32);
    rows_[4]->pack_child(buttons_[' ']);

    buttons_['\r'] = new_button("DONE");
    buttons_['\r']->resize(32 * 4 + (2 * 3), 32);
    rows_[4]->pack_child(buttons_['\r']);

    main_frame_->pack_child(rows_[4]);
}

void Keyboard::focus(Widget* widget) {
    widget->set_background_colour(smlt::ui::UIConfig().highlight_colour_);
    focussed_ = widget;
}

void Keyboard::unfocus(Widget* widget) {
    widget->set_background_colour(smlt::ui::UIConfig().background_colour_);
    focussed_ = nullptr;
}

}
}
