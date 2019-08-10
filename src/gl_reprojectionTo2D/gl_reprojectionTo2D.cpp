#include <glm/glm.hpp>
#include <glm/gtc/random.hpp>

#include <ppgso/ppgso.h>
#include <unistd.h>

#include <shaders/diffuse_vert_glsl.h>
#include <shaders/diffuse_frag_glsl.h>
#include <shaders/depth_frag_glsl.h>

using namespace std;
using namespace glm;
using namespace ppgso;

const unsigned int SIZE = 800;

// desired rotations and num. of rotation steps
vector<vector<int>> rotations;
int steps = 8;
vector<int> view;

/*!
 * Custom window for displaying a mesh with diffuse lighting
 */
class DiffuseWindow : public Window {
private:
    Shader program = {diffuse_vert_glsl, diffuse_frag_glsl};
    Shader programDepth = {diffuse_vert_glsl, depth_frag_glsl};

    Texture texture = {image::loadBMP("duck.bmp")};
    Mesh object = {"duck_scene_blender_triangulate.obj"};

    std::string snapshotName;
    int spashotID = 0;

public:
    /*!
     * Create new custom window
     */
    DiffuseWindow() : Window{"gl7_diffuse", SIZE, SIZE} {
        // Set camera position with perspective projection
        program.setUniform("ProjectionMatrix", perspective((PI / 180.f) * 60.0f, 1.0f, 0.1f, 10.0f));
        programDepth.setUniform("ProjectionMatrix", perspective((PI / 180.f) * 60.0f, 1.0f, 0.1f, 10.0f));

        // Set the light direction, assumes simple white directional light
        program.setUniform("LightDirection", normalize(vec3{-1.0f, -1.0f, -1.0f}));
        programDepth.setUniform("LightDirection", normalize(vec3{-1.0f, -1.0f, -1.0f}));

        // Set texture as program input
        program.setUniform("Texture", texture);
        programDepth.setUniform("Texture", texture);

        // Enable Z-buffer
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LEQUAL);

        // Enable polygon culling
        glEnable(GL_CULL_FACE);
        glFrontFace(GL_CCW);
        glCullFace(GL_BACK);
    }


    void onKey(int key, int scanCode, int action, int mods) override {
        if (key == GLFW_KEY_SPACE && action == GLFW_PRESS) {
            // Set new random light direction
            program.setUniform("LightDirection", (vec3)sphericalRand(1.0));

        }
    }

    /*!
     * Window update implementation that will be called automatically from pollEvents
     */
    void onIdle() override {
        // Set gray background
        glClearColor(1.0f, 1.0f, 1.0f, 0);

        // Clear depth and color buffers
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Get time
        auto time = glfwGetTime();

        // Rotate camera according to view
        auto cameraMat = translate(mat4{1.0f}, {0.0f, 0.0f, -0.2f});
        cameraMat = rotate(cameraMat, (float(view[0]) / 360.0f) * 2.0f * 3.14f, {1.0f, 0.0f, 0.0f});
        cameraMat = rotate(cameraMat, (float(view[1]) / 360.0f) * 2.0f * 3.14f, {0.0f, 1.0f, 0.0f});
        cameraMat = rotate(cameraMat, (float(view[2]) / 360.0f) * 2.0f * 3.14f, {0.0f, 0.0f, 1.0f});

        program.setUniform("ViewMatrix", cameraMat);
        programDepth.setUniform("ViewMatrix", cameraMat);



        // Create object matrix
        auto sphereMat = mat4{1.0f};
//        sphereMat = rotate(sphereMat, (float(view[0]) / 360.0f) * 2.0f * 3.14f, {1.0f, 0.0f, 0.0f});
//        sphereMat = rotate(sphereMat, (float(view[1]) / 360.0f) * 2.0f * 3.14f, {0.0f, 1.0f, 0.0f});
//        sphereMat = rotate(sphereMat, (float(view[2]) / 360.0f) * 2.0f * 3.14f, {0.0f, 0.0f, 1.0f});

        // Render object view
        spashotID++;

        // Set the matrix as model matrix for current program
        programDepth.setUniform("ModelMatrix", sphereMat);

        // Render object depth map
        snapshotName = "depthMaps/depth_snap" + to_string(spashotID);;
        object.render();

        // Set the matrix as model matrix for current program
        program.setUniform("ModelMatrix", sphereMat);

        snapshotName = "snapshots/snap" + to_string(spashotID);
        object.render();
    }
};

void generateRotationCombinations() {
    int stepSize = int(360 / steps);

    for (int x = 0; x < 360; x += stepSize) {
        for (int y = 0; y < 360; y += stepSize) {
            for (int z = 0; z < 360; z += stepSize) {
                vector<int> point = {x, y, z};
                rotations.emplace_back(point);
            }
        }
    }
}

int main() {
    // Create a window with OpenGL 3.3 enabled
    DiffuseWindow window;

    // Generate rotation combinations matrix
    generateRotationCombinations();

    // Rotate for every rotation point
    for (int i = 0; i < rotations.size(); i++) {
        view = rotations[i];
        window.pollEvents();
        usleep(100000);
    }
    return EXIT_SUCCESS;
}
