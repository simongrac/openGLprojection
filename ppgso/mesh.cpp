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

void CopyDepthBuffer(GLuint texId, int x, int y, int imageWidth, int imageHeight){
    //glBindTexture(GL_TEXTURE_2D, texId);
    //glReadBuffer(GL_BACK); // Ensure we are reading from the back buffer.
    //glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, x, y, imageWidth, imageHeight, 0);
}

float near = 0.1;
float far  = 100.0;

float LinearizeDepth(float depth){
    float z = depth * 2.0 - 1.0; // back to NDC
    return (2.0 * near * far) / (far + near - z * (far - near));
}

void Mesh::renderAndMakeSnapshots(int width, int height, int spashotID) {

        for(auto& buffer : buffers) {

            // Draw object
            glBindVertexArray(buffer.vao);
            glDrawElements(GL_TRIANGLES, buffer.size, GL_UNSIGNED_INT, nullptr);

            // make and save snapshot

    //        //glReadBuffer(GL_BACK);
    //        cv::Mat img(width, height, CV_8UC3);
    //        glPixelStorei(GL_PACK_ALIGNMENT, (img.step & 3)?1:4);
    //        glPixelStorei(GL_PACK_ROW_LENGTH, img.step/img.elemSize());
    //        glReadPixels(0, 0, img.cols, img.rows, GL_BGR_EXT, GL_UNSIGNED_BYTE, img.data);
    //        cv::Mat flipped(img);
    //        cv::flip(img, flipped, 0);
    //        string filename = "/Users/simongrac/Documents/School/DP/ppgso/snapshots/snap" + to_string(spashotID) + ".png";
    //        cv::imwrite(filename, img);

            // TODO: DEPTH MAP
            // http://www.opengl-tutorial.org/intermediate-tutorials/tutorial-14-render-to-texture/
            // http://glampert.com/2014/01-26/visualizing-the-depth-buffer/
            //out vec4 FragColor;

//            cv::Mat A = cv::Mat::zeros(width,height, CV_32F);
//
//            for (int i = 1; i < width; i++) {
//                for (int j = 1; j < height; j++) {
//                    //float depth = 0.0f;
//    //                GLfloat* depths;
//    //                depths = new GLfloat[ width * height ];
//    //                glReadPixels (i, j, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, depths);
//    //                cout << *depths << " ";
//
//                    GLfloat depth_comp;
//                    glReadPixels( i, j, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &depth_comp);
//
//                    float depth = LinearizeDepth(depth_comp) / far;
//
//                    //vec4 FragColor = vec4(vec3(depth), 1.0);
//                    //cout << depth << endl;
//                    A.at<cv::Vec3b>(i,j) = depth;
//    //                GLfloat clip_z = (depth_comp - 0.5f) * 2.0f;
//    //                GLfloat world_z = 2*far_znear_z/(clip_z*(far_z-near_z)-(far_z+near_z));
//                }
//            }
//
//            cv::imshow("test", A);
//            cv::waitKey(0);

/*            cv::Mat imgD(width, height, CV_8U);
            glReadPixels(0, 0, imgD.cols, imgD.rows, GL_DEPTH_COMPONENT, GL_FLOAT, imgD.data);
            //cv::Mat flippedimgD(imgD);
            //cv::flip(imgD, flippedimgD, 0);
            string filename = "/Users/simongrac/Documents/School/DP/ppgso/snapshots/snapD" + to_string(spashotID) + ".png";
            //cv::imwrite(filename, imgD);
            cv::imshow("test", imgD);
            cv::waitKey(0);*/



        }


}