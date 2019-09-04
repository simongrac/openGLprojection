// Example gl_diffuse
// - Demonstration of simple diffuse lighting with one directional light source

#include <glm/glm.hpp>
#include <glm/gtc/random.hpp>

#include <ppgso/ppgso.h>
#include <unistd.h>

#include <shaders/diffuse_vert_glsl.h>
#include <shaders/diffuse_frag_glsl.h>

//#include <opencv2/imgproc/imgproc.hpp>
//#include <opencv2/highgui/highgui.hpp>


using namespace std;
using namespace glm;
using namespace ppgso;

const unsigned int SIZE = 500;

// desired rotations and num. of rotation steps
vector<vector<int>> rotations;
int stepsX = 20;
int stepsY = 100;
int stepsZ = 100;
vector<int> view;

/*!
 * Custom window for displaying a mesh with diffuse lighting
 */
class DiffuseWindow : public Window {
private:
    Shader program = {diffuse_vert_glsl, diffuse_frag_glsl};
    Texture texture = {image::loadBMP("duck.bmp")};
    Mesh object = {"duck_scene_blender_triangulate.obj"};

    int spashotID = 0;
    int rotationX = 0;
    int rotationY = 0;
    int rotationZ = 0;

public:
    /*!
     * Create new custom window
     */
    DiffuseWindow() : Window{"gl7_diffuse", SIZE, SIZE} {
        // Set camera position with perspective projection
        program.setUniform("ProjectionMatrix", perspective((PI / 180.f) * 60.0f, 1.0f, 0.1f, 10.0f));

        // Set the light direction, assumes simple white directional light
        program.setUniform("LightDirection", normalize(vec3{-1.0f, -1.0f, -1.0f}));

        // Set texture as program input
        program.setUniform("Texture", texture);

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
            program.setUniform("LightDirection", (vec3) sphericalRand(1.0));
        }

        if (action == GLFW_PRESS || action == GLFW_REPEAT) {
            switch (key) {
                case GLFW_KEY_DOWN:
                    rotationX = (rotationX - 5 + 360) % 360;
                    break;
                case GLFW_KEY_UP:
                    rotationX = (rotationX + 5 + 360) % 360;
                    break;
                case GLFW_KEY_RIGHT:
                    rotationY = (rotationY - 5 + 360) % 360;
                    break;
                case GLFW_KEY_LEFT:
                    rotationY = (rotationY + 5 + 360) % 360;
                    break;
                case GLFW_KEY_N:
                    rotationZ = (rotationZ - 5 + 360) % 360;
                    break;
                case GLFW_KEY_M:
                    rotationZ = (rotationZ + 5 + 360) % 360;
                    break;
                case GLFW_KEY_C:
                    cout << endl << "x:" << rotationX << " y:" << rotationY << " z:" << rotationZ << endl;
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
        glClearColor(1.0f, 1.0f, 1.0f, 0);

        // Clear depth and color buffers
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Get time
        auto time = glfwGetTime();

        /*
        // Rotate camera according to view
        auto cameraMat = translate(mat4{1.0f}, {0.0f, 0.0f, -0.2f});
        cameraMat = rotate(cameraMat, (float(view[0]) / 180.0f) * 3.14f, {1.0f, 0.0f, 0.0f});
        cameraMat = rotate(cameraMat, (float(view[1]) / 180.0f) * 3.14f, {0.0f, 1.0f, 0.0f});
        cameraMat = rotate(cameraMat, (float(view[2]) / 180.0f) * 3.14f, {0.0f, 0.0f, 1.0f});
        program.setUniform("ViewMatrix", cameraMat);
        */

        // Rotate camera using buttons
        // Set camera position/rotation - for example, translate camera a bit backwards (positive value in Z axis), so we can see the objects
        auto cameraMat = translate(mat4{1.0f}, {0.0f, 0.0f, -0.5f});
        cameraMat = rotate(cameraMat, (float(rotationX) / 180.0f) * 3.14f, {1.0f, 0.0f, 0.0f});
        cameraMat = rotate(cameraMat, (float(rotationY) / 180.0f) * 3.14f, {0.0f, 1.0f, 0.0f});
        cameraMat = rotate(cameraMat, (float(rotationZ) / 180.0f) * 3.14f, {0.0f, 0.0f, 1.0f});

        // Set the matrix that rotates camera
        program.setUniform("ViewMatrix", cameraMat);


        // Create object matrix that rotates in time
        auto sphereMat = mat4{1.0f};
        //sphereMat = rotate(sphereMat, float(rotationX) / 10.0f, {1.0f, 0.0f, 0.0f});
        //sphereMat = rotate(sphereMat, float(rotationY) / 10.0f, {0.0f, 1.0f, 0.0f});
        //sphereMat = rotate(sphereMat, float(rotationZ) / 10.0f, {0.0f, 0.0f, 1.0f});

        // Set the matrix as model matrix for current program
        program.setUniform("ModelMatrix", sphereMat);

        // Render object
        spashotID++;
        object.renderAndMakeSnapshots(SIZE, SIZE, spashotID);
    }
};

void generateRotationCombinations() {
    int stepSizeX = int(360 / stepsX);
    int stepSizeY = int(360 / stepsY);
    int stepSizeZ = int(360 / stepsZ);

    for (int x = 180; x >= 0; x -= stepSizeX) {
        //for (int y = 0; y < 360; y += stepSizeY) {
            for (int z = 0; z < 360; z += stepSizeZ) {
                vector<int> point = {x, 0, z};
                rotations.emplace_back(point);
        //    }
        }
    }
}

int main() {
    // Create a window with OpenGL 3.3 enabled
    DiffuseWindow window;

    // Generate rotation combinations matrix
     generateRotationCombinations();

     cout << rotations.size();

    // Main execution loop
    while (window.pollEvents()) {}

    // Rotate for every rotation point
    for (int i = 0; i < rotations.size(); i++) {
        view = rotations[i];
        window.pollEvents();
        usleep(10000);
    }

    return EXIT_SUCCESS;
}
