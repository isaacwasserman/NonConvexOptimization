//--------------------------------------------------
// Author: Isaac Wasserman
// Date: 02/27/2023
// Description: Loads PLY files in ASCII format
//--------------------------------------------------

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include "plymesh.h"

using namespace std;
using namespace glm;

/**
 * @brief Tokenizes a string by spaces
 * 
 * @param s Input string
 * @return Vector of tokens
 */
vector<string> tokenize(const string& s) {
   vector<string> tokens;
   string token;
   istringstream tokenStream(s);
   while (getline(tokenStream, token, ' ')) {
      tokens.push_back(token);
   }
   return tokens;
}

namespace agl {

   /**
    * @brief Construct a new PLYMesh::PLYMesh object
    */
   PLYMesh::PLYMesh() {
      setIsDynamic(true);
   }

   /**
    * @brief Construct a new PLYMesh::PLYMesh object based on a PLY file
    * 
    * @param filename Path to PLY file
    */
   PLYMesh::PLYMesh(const std::string& filename) {
      setIsDynamic(true);
      load(filename);
   }

   /**
    * @brief Initializes the mesh for use with OpenGL
    */
   void PLYMesh::init() {
      assert(_positions.size() != 0);
      initBuffers(&_faces, &_positions, &_normals);
   }

   /**
    * @brief Destroy the PLYMesh::PLYMesh object
    */
   PLYMesh::~PLYMesh() {
   }

   /**
    * @brief Loads a PLY file
    * 
    * @param filename Path to PLY file
    * @return Whether the file was loaded successfully
    */
   bool PLYMesh::load(const std::string& filename) {
      if (_positions.size() != 0) {
         std::cout << "WARNING: Cannot load different files with the same PLY mesh\n";
         return false;
      }
      // todo: your code here
      ifstream file(filename);
      string line;
      vector<string> tokens;
      bool isPLY = false;
      int numVertices = 0;
      int numFaces = 0;
      // Read header
      while(getline(file, line)){
         tokens = tokenize(line);
         if(tokens.size() < 1){
            continue;
         }
         if(tokens[0] == "ply"){
            isPLY = true;
         }
         else if(tokens[0] == "format"){
            continue;
         }
         else if(tokens[0] == "element"){
            if(tokens[1] == "vertex"){
               numVertices = stoi(tokens[2]);
            }
            else if(tokens[1] == "face"){
               numFaces = stoi(tokens[2]);
            }
         }
         else if(tokens[0] == "end_header"){
            break;
         }
      }
      if(!isPLY){
         cout << "Not a PLY file" << endl;
         return false;
      }
      // read vertices
      for(int i = 0; i < numVertices; i++){
         getline(file, line);
         tokens = tokenize(line);
         if(tokens.size() < 1){
            i--;
            continue;
         }
         _positions.push_back(stof(tokens[0]));
         _positions.push_back(stof(tokens[1]));
         _positions.push_back(stof(tokens[2]));
      }
      // read faces
      for(int i = 0; i < numFaces; i++){
         getline(file, line);
         tokens = tokenize(line);
         if(tokens.size() < 1){
            i--;
            continue;
         }
         _faces.push_back(stoi(tokens[1]));
         _faces.push_back(stoi(tokens[2]));
         _faces.push_back(stoi(tokens[3]));
      }
      file.close();

      // compute normals
      _normals.resize(_positions.size());
      for(int i = 0; i < _faces.size(); i+=3){
         int i1 = _faces[i];
         int i2 = _faces[i+1];
         int i3 = _faces[i+2];
         vec3 v1 = vec3(_positions[i1*3], _positions[i1*3+1], _positions[i1*3+2]);
         vec3 v2 = vec3(_positions[i2*3], _positions[i2*3+1], _positions[i2*3+2]);
         vec3 v3 = vec3(_positions[i3*3], _positions[i3*3+1], _positions[i3*3+2]);
         vec3 n = normalize(cross(v2-v1, v3-v1));
         _normals[i1*3] += n.x;
         _normals[i1*3+1] += n.y;
         _normals[i1*3+2] += n.z;
         _normals[i2*3] += n.x;
         _normals[i2*3+1] += n.y;
         _normals[i2*3+2] += n.z;
         _normals[i3*3] += n.x;
         _normals[i3*3+1] += n.y;
         _normals[i3*3+2] += n.z;
      }
      return true;
   }

   /**
    * @brief Returns the minimum bounds of the mesh
    * 
    * @return Vector of <minX, minY, minZ>
    */
   glm::vec3 PLYMesh::minBounds() const {
      float minX = 0;
      float minY = 0;
      float minZ = 0;
      for(int i = 0; i < _positions.size(); i+=3){
         if(_positions[i] < minX || i == 0){
            minX = _positions[i];
         }
         if(_positions[i+1] < minY || i == 0){
            minY = _positions[i+1];
         }
         if(_positions[i+2] < minZ || i == 0){
            minZ = _positions[i+2];
         }
      }
      return glm::vec3(minX, minY, minZ);
   }

   /**
    * @brief Returns the maximum bounds of the mesh
    * 
    * @return Vector of <maxX, maxY, maxZ>
    */
   glm::vec3 PLYMesh::maxBounds() const {
      float maxX = 0;
      float maxY = 0;
      float maxZ = 0;
      for(int i = 0; i < _positions.size(); i+=3){
         if(_positions[i] > maxX || i == 0){
            maxX = _positions[i];
         }
         if(_positions[i+1] > maxY || i == 0){
            maxY = _positions[i+1];
         }
         if(_positions[i+2] > maxZ || i == 0){
            maxZ = _positions[i+2];
         }
      }
      return glm::vec3(maxX, maxY, maxZ);
   }

   /**
    * @brief Gets number of vertices in the mesh
    * 
    * @return Number of vertices
    */
   int PLYMesh::numVertices() const {
      return _positions.size() / 3;
   }

   /**
    * @brief Gets number of triangles in the mesh
    * 
    * @return Number of triangles
    */
   int PLYMesh::numTriangles() const {
      return _faces.size() / 3;
   }

   /**
    * @brief Gets the positions of the vertices mesh
    * 
    * @return Returns _positions member variable
    */
   const std::vector<GLfloat>& PLYMesh::positions() const {
      return _positions;
   }

   /**
    * @brief Gets the normals of the vertices mesh
    * 
    * @return Returns _normals member variable
    */
   const std::vector<GLfloat>& PLYMesh::normals() const {
      return _normals;
   }

   /**
    * @brief Gets the indices of the triangles in the mesh
    * 
    * @return Returns _faces member variable
    */
   const std::vector<GLuint>& PLYMesh::indices() const {
      return _faces;
   }

   void PLYMesh::computeNormals(){
      _normals.resize(_positions.size());
      for(int i = 0; i < _faces.size(); i+=3){
         int i1 = _faces[i];
         int i2 = _faces[i+1];
         int i3 = _faces[i+2];
         vec3 v1 = vec3(_positions[i1*3], _positions[i1*3+1], _positions[i1*3+2]);
         vec3 v2 = vec3(_positions[i2*3], _positions[i2*3+1], _positions[i2*3+2]);
         vec3 v3 = vec3(_positions[i3*3], _positions[i3*3+1], _positions[i3*3+2]);
         vec3 n = normalize(cross(v2-v1, v3-v1));
         _normals[i1*3] = n.x;
         _normals[i1*3+1] = n.y;
         _normals[i1*3+2] = n.z;
         _normals[i2*3] = n.x;
         _normals[i2*3+1] = n.y;
         _normals[i2*3+2] = n.z;
         _normals[i3*3] = n.x;
         _normals[i3*3+1] = n.y;
         _normals[i3*3+2] = n.z;
      }
   }

   GLuint* PLYMesh::getFace(int i){
      return &_faces[i];
   }

   GLfloat* PLYMesh::getVert(int i){
      return &_positions[i*3];
   }

   void PLYMesh::setVert(int i, GLfloat x, GLfloat y, GLfloat z){
      _positions[i*3] = x;
      _positions[i*3+1] = y;
      _positions[i*3+2] = z;
      std::cout << i << " <= " << _nVerts << std::endl;
      if(i < _nVerts){
         setVertexData(POSITION, i, vec4(x, y, z, 1.0f));
      }
   }
}
