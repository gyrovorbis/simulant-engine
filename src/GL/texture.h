#ifndef TEXTURE_H_INCLUDED
#define TEXTURE_H_INCLUDED

#include <cstdint>
#include <vector>
#include <SDL_opengl.h>

namespace GL {

class Texture {
public:
    typedef std::vector<uint8_t> Data;

    GLuint gl_tex() const;

    Texture():
        width_(0),
        height_(0),
        bpp_(32),
        gl_tex_(0) { }

    ~Texture();

    void set_bpp(int bits=32);
    void resize(int width, int height);
    void upload(bool free_after=true); //Upload to GL, initializes the tex ID
    void free(); //Frees the data used to construct the texture

    uint32_t width() const { return width_; }
    uint32_t height() const { return height_; }
    uint32_t bpp() const { return bpp_; }
    Texture::Data& data() { return data_; }

private:
    uint32_t width_;
    uint32_t height_;
    uint32_t bpp_;

    Texture::Data data_;

    GLuint gl_tex_;
};

}

#endif // TEXTURE_H_INCLUDED
