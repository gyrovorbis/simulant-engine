#include "font.h"
#include "texture.h"
#include "assets/material.h"
#include "macros.h"

#define STB_TRUETYPE_IMPLEMENTATION  // force following include to generate implementation
#define STBTT_STATIC
#define STBTT_memcpy memcpy
#define STBTT_memset memset

#include "deps/stb_truetype/stb_truetype.h"

namespace smlt {

Font::Font(FontID id, AssetManager *asset_manager):
    Asset(asset_manager),
    generic::Identifiable<FontID>(id) {

}

TexturePtr Font::texture() const {
    return texture_;
}

MaterialPtr Font::material() const {
    return material_;
}

bool Font::init() {

    return true;
}

std::pair<Vec2, Vec2> Font::texture_coordinates_for_character(char32_t ch) {
    /* If we're out of range, just display a '?' */
    /* FIXME: Deal with unicode properly! */
    if(ch < 32) {
        ch = '?';
    } else {
        ch -= 32;
    }

    if(ch >= char_data_.size()) {
        ch = '?';
    }

    if(info_) {
        stbtt_aligned_quad q;
        float x = 0, y = 0;

        stbtt_bakedchar* tmp = (stbtt_bakedchar*) &char_data_[0];

        stbtt_GetBakedQuad(tmp, texture_->width(), texture_->height(), ch, &x, &y, &q, 1);

        return std::make_pair(
            Vec2(q.s0, q.t0),
            Vec2(q.s1, q.t1)
        );
    } else {
        auto data = char_data_[ch];
        auto pw = float(page_width(ch));
        auto ph = float(page_height(ch));

        return std::make_pair(
            Vec2(float(data.x0) / pw, float(ph - data.y0) / ph),
            Vec2(float(data.x1) / pw, float(ph - data.y1) / ph)
        );
    }
}

uint16_t Font::character_width(char32_t ch) {
    if(ch < 32) {
        return 0;
    }

    ch -= 32;

    /* If we're out of range, just display a '?' */
    /* FIXME: Deal with unicode properly! */
    if(ch >= char_data_.size()) {
        ch = '?';
    }

    auto *b = &char_data_.at(ch);
    return std::abs(b->x1 - b->x0);
}

uint16_t Font::character_height(char32_t ch) {
    if(ch < 32) {
        return this->size();
    }

    ch -= 32;

    /* If we're out of range, just display a '?' */
    /* FIXME: Deal with unicode properly! */
    if(ch >= char_data_.size()) {
        ch = '?';
    }

    auto *b = &char_data_.at(ch);
    return std::abs(b->y1 - b->y0);
}

float Font::character_advance(char32_t ch, char32_t next) {
    _S_UNUSED(next); // FIXME: Kerning!

    if(ch < 32) {
        return 0;
    }

    ch -= 32;

    /* If we're out of range, just display a '?' */
    /* FIXME: Deal with unicode properly! */
    if(ch >= char_data_.size()) {
        ch = '?';
    }

    auto *b = &char_data_.at(ch);
    return b->xadvance;
}

std::pair<int16_t, int16_t> Font::character_offset(char32_t ch) {
    if(ch < 32) {
        return std::make_pair(0, 0);
    }

    ch -= 32;

    /* If we're out of range, just display a '?' */
    /* FIXME: Deal with unicode properly! */
    if(ch >= char_data_.size()) {
        ch = '?';
    }

    auto *b = &char_data_.at(ch);

    return std::make_pair(
        (int16_t) b->xoff,
        (int16_t) b->yoff
    );
}

int16_t Font::ascent() const {
    return ascent_;
}

int16_t Font::descent() const {
    return descent_;
}

int16_t Font::line_gap() const {
    return line_gap_;
}

uint16_t Font::page_width(char ch) {
    _S_UNUSED(ch);

    // FIXME: This will need to change when we support multiple pages
    return page_width_;
}

uint16_t Font::page_height(char ch) {
    _S_UNUSED(ch);

    return page_height_;
}


}
