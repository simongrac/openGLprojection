// Example gl_diffuse
// - Demonstration of simple diffuse lighting with one directional light source

#include <glm/glm.hpp>
#include <glm/gtc/random.hpp>
#include <unistd.h>


#include <ppgso/ppgso.h>

#include <shaders/diffuse_vert_glsl.h>
#include <shaders/diffuse_frag_glsl.h>
#include <shaders/depth_frag_glsl.h>

using namespace std;
using namespace glm;
using namespace ppgso;

const unsigned int SIZE = 800;


vector<vector<int>> rotations;
int steps = 10;
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

    int rotationX = 0;
    int rotationY = 0;
    int rotationZ = 0;

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

    /*!
     * Handles pressed key when the window is focused
     * @param key Key code of the key being pressed/released
     * @param scanCode Scan code of the key being pressed/released
     * @param action Action indicating the key state change
     * @param mods Additional modifiers to consider
     */
    void onKey(int key, int scanCode, int action, int mods) override {
        if (key == GLFW_KEY_SPACE && action == GLFW_PRESS) {
            // Set new random light direction
            program.setUniform("LightDirection", (vec3)sphericalRand(1.0));

        }

        if (action == GLFW_PRESS || action == GLFW_REPEAT) {
            switch (key) {
                case GLFW_KEY_DOWN:
                    rotationX = (rotationX - 1) % 360;
                    break;
                case GLFW_KEY_UP:
                    rotationX = (rotationX + 1) % 360;
                    break;
                case GLFW_KEY_LEFT:
                    rotationY = (rotationY - 1) % 360;
                    break;
                case GLFW_KEY_RIGHT:
                    rotationY = (rotationY + 1) % 360;
                    break;
                default:
                    break;
            }
        }
    }

    /*!
     * Window update implementation that will be called automatically from pollEvents
     */
    void onIdle() override {
        // Set gray background
        glClearColor(.5f,.5f,.5f,0);

        // Clear depth and color buffers
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Get time
        auto time = glfwGetTime();

        // Create object matrix that rotates in time
        //auto sphereMat = mat4{1.0f};

        // Set camera position/rotation - for example, translate camera a bit backwards (positive value in Z axis), so we can see the objects
        auto cameraMat = translate(mat4{1.0f}, {0.0f, 0.0f, -0.2f});
        cameraMat = rotate(cameraMat, (float(view[0]) / 360.0f) * 2.0f * 3.14f, {1.0f, 0.0f, 0.0f});
        cameraMat = rotate(cameraMat, (float(view[1]) / 360.0f) * 2.0f * 3.14f, {0.0f, 1.0f, 0.0f});
        cameraMat = rotate(cameraMat, (float(view[2]) / 360.0f) * 2.0f * 3.14f, {0.0f, 0.0f, 1.0f});


        program.setUniform("ViewMatrix", cameraMat);
        programDepth.setUniform("ViewMatrix", cameraMat);

        // Set the matrix as model matrix for current program

        // Create object matrix
        auto sphereMat = mat4{1.0f};
        //sphereMat = rotate(sphereMat, (float(view[0]) / 360.0f) * 2.0f * 3.14f, {1.0f, 0.0f, 0.0f});
        //sphereMat = rotate(sphereMat, (float(view[1]) / 360.0f) * 2.0f * 3.14f, {0.0f, 1.0f, 0.0f});
        //sphereMat = rotate(sphereMat, (float(view[2]) / 360.0f) * 2.0f * 3.14f, {0.0f, 0.0f, 1.0f});

        program.setUniform("ModelMatrix", sphereMat);
        spashotID++;
        std::string snapshotName = "depthMaps/depth_snap" + to_string(spashotID);;
        // Render object
        object.renderAndMakeSnapshots(SIZE, SIZE, snapshotName);

        programDepth.setUniform("ModelMatrix", sphereMat);
        snapshotName = "snapshots/snap" + to_string(spashotID);

        object.renderAndMakeSnapshots(SIZE, SIZE, snapshotName);

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
