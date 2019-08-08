#include <glm/glm.hpp>

#include <sstream>
#include <ctime>

#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

#include "mesh.h"

using namespace std;
using namespace glm;
using namespace ppgso;

Mesh::Mesh(const string &obj_file) {
  // Load OBJ file
  shapes.clear();
  materials.clear();
  string err = tinyobj::LoadObj(shapes, materials, obj_file.c_str());

  if (!err.empty()) {
    stringstream msg;
    msg << err << endl << "Failed to load OBJ file " << obj_file << "!" << endl;
    throw runtime_error(msg.str());
  }

  // Initialize OpenGL Buffers
  for(auto& shape : shapes) {
    gl_buffer buffer;

    if(!shape.mesh.positions.empty()) {
      // Generate a vertex array object
      glGenVertexArrays(1, &buffer.vao);
      glBindVertexArray(buffer.vao);

      // Generate and upload a buffer with vertex positions to GPU
      glGenBuffers(1, &buffer.vbo);
      glBindBuffer(GL_ARRAY_BUFFER, buffer.vbo);
      glBufferData(GL_ARRAY_BUFFER, shape.mesh.positions.size() * sizeof(float), shape.mesh.positions.data(),
                   GL_STATIC_DRAW);

      // Bind the buffer to "Position" attribute in program
      glEnableVertexAttribArray(0);
      glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    }

    if(!shape.mesh.texcoords.empty()) {
      // Generate and upload a buffer with texture coordinates to GPU
      glGenBuffers(1, &buffer.tbo);
      glBindBuffer(GL_ARRAY_BUFFER, buffer.tbo);
      glBufferData(GL_ARRAY_BUFFER, shape.mesh.texcoords.size() * sizeof(float), shape.mesh.texcoords.data(),
                   GL_STATIC_DRAW);

      glEnableVertexAttribArray(1);
      glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
    }

    if(!shape.mesh.normals.empty()) {
      // Generate and upload a buffer with texture coordinates to GPU
      glGenBuffers(1, &buffer.nbo);
      glBindBuffer(GL_ARRAY_BUFFER, buffer.nbo);
      glBufferData(GL_ARRAY_BUFFER, shape.mesh.normals.size() * sizeof(float), shape.mesh.normals.data(),
                   GL_STATIC_DRAW);

      glEnableVertexAttribArray(2);
      glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    }

    // Generate and upload a buffer with indices to GPU
    glGenBuffers(1, &buffer.ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer.ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, shape.mesh.indices.size() * sizeof(unsigned int), shape.mesh.indices.data(), GL_STATIC_DRAW);
    buffer.size = (GLsizei) shape.mesh.indices.size();

    // Copy it to the end of the buffers vector
    buffers.push_back(buffer);
  }
}

Mesh::~Mesh() {
  for(auto& buffer : buffers) {
    glDeleteBuffers(1, &buffer.ibo);
    glDeleteBuffers(1, &buffer.nbo);
    glDeleteBuffers(1, &buffer.tbo);
    glDeleteBuffers(1, &buffer.vbo);
    glDeleteVertexArrays(1, &buffer.vao);
  }
}

void Mesh::render() {
  for(auto& buffer : buffers) {
    // Draw object
    glBindVertexArray(buffer.vao);
    glDrawElements(GL_TRIANGLES, buffer.size, GL_UNSIGNED_INT, nullptr);
  }
}

//void CopyDepthBuffer(GLuint texId, int x, int y, int imageWidth, int imageHeight)
//{
//    glBindTexture(GL_TEXTURE_2D, texId);
//
//    glReadBuffer(GL_BACK); // Ensure we are reading from the back buffer.
//    glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, x, y, imageWidth, imageHeight, 0);
//}

void Mesh::renderAndMakeSnapshots(int width, int height, int spashotID) {
    for(auto& buffer : buffers) {

        // Draw object
        glBindVertexArray(buffer.tbo);
        glDrawElements(GL_TRIANGLES, buffer.size, GL_UNSIGNED_INT, nullptr);

        // make and save snapshot

        glReadBuffer(GL_BACK);
        cv::Mat img(width, height, CV_8UC3);
        glPixelStorei(GL_PACK_ALIGNMENT, (img.step & 3)?1:4);
        glPixelStorei(GL_PACK_ROW_LENGTH, img.step/img.elemSize());
        glReadPixels(0, 0, img.cols, img.rows, GL_BGR_EXT, GL_UNSIGNED_BYTE, img.data);
        cv::Mat flipped(img);
        cv::flip(img, flipped, 0);
        string filename = "/home/simon/openGLprojection/snapshots/snapshot" + to_string(spashotID) + ".png";
        cv::imwrite(filename, img);

        // TODO: DEPTH MAP
        // http://www.opengl-tutorial.org/intermediate-tutorials/tutorial-14-render-to-texture/
        // http://glampert.com/2014/01-26/visualizing-the-depth-buffer/
        // CopyDepthBuffer(buffer.tbo, 0, 0, width, height);

    }
}