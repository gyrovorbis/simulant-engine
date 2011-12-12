#ifndef SCENE_H_INCLUDED
#define SCENE_H_INCLUDED

#include <stdexcept>
#include <map>

#include "object.h"
#include "mesh.h"
#include "camera.h"
#include "renderer.h"
#include "texture.h"

namespace GL {

class Scene : public Object {
public:
    void move(float x, float y, float z) {
        throw std::logic_error("You cannot move the scene");
    }

    Scene() {
        new_camera(); //Create a default camera

        //Set up the default render options
        render_options.wireframe_enabled = false;
        render_options.texture_enabled = true;
        render_options.backface_culling_enabled = true;
    }

    MeshID new_mesh();
    CameraID new_camera();
    TextureID new_texture();

    Mesh& mesh(MeshID m);
    Camera& camera(CameraID c = 0);
    Texture& texture(TextureID t);

    void render();

    void accept(ObjectVisitor& visitor) {
        for(Object* child: children_) {
            child->accept(visitor);
        }

        visitor.visit(this);
    }

    RenderOptions render_options;
private:
    std::map<MeshID, Mesh> meshes_;
    std::map<CameraID, Camera> cameras_;
    std::map<TextureID, Texture> textures_;
    CameraID current_camera_;
};

}

#endif // SCENE_H_INCLUDED
