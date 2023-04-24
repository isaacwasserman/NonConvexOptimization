//--------------------------------------------------
// Author: Isaac Wasserman
// Date: 02/28/2023
// Description: Views PLY files
//--------------------------------------------------

#include <cmath>
#include <string>
#include <vector>

#include "agl/image.h"
#include "agl/window.h"
#include "osutils.h"
#include "perlin.h"
#include "plymesh.h"

using namespace std;
using namespace glm;
using namespace agl;

float* perlin = NULL;
float terrainRadius = 1.0f;
float islandFloatHeight = 0.0f;
float d = 0.02;

PLYMesh island = PLYMesh("../models/island.ply");

vec3 u;
vec3 v;
vec3 a;
vec3 b;
vec3 c;

float worldCoordsToTerrainHeight(float x_, float z_) {
    if(perlin == NULL){
        return 0;
    }
    float x = x_ / terrainRadius;
    float z = z_ / terrainRadius;
    float y = perlin[(int)((x + 1.0f) * 128.0f) * 256 + (int)((z + 1.0f) * 128.0f)];
    float edgeDecay = -0.5 * (length(vec2(x, z)) * length(vec2(x, z)));
    float heightScale = 1.0f;
    return y * (heightScale + edgeDecay);
}

float worldCoordsToTerrainHeightWithFloat(float x_, float z_) {
    return (0.1f * worldCoordsToTerrainHeight(x_, z_)) + islandFloatHeight;
}

std::vector<vec3> getVertsOfNearestTerrainTri(float x_, float z_){
    int nTris = island.numTriangles();
    int nearestTriIndex = -1;
    for(int triIndex = 0; triIndex < nTris; triIndex++){
        GLuint* verts = island.getFace(triIndex);
        vec3 v0 = vec3(island.getVert(verts[0])[0],island.getVert(verts[0])[1],island.getVert(verts[0])[2]);
        vec3 v1 = vec3(island.getVert(verts[1])[0],island.getVert(verts[1])[1],island.getVert(verts[1])[2]);
        vec3 v2 = vec3(island.getVert(verts[2])[0],island.getVert(verts[2])[1],island.getVert(verts[2])[2]);
        vec3 v0xz = vec3(v0.x, 0, v0.z);
        vec3 v1xz = vec3(v1.x, 0, v1.z);
        vec3 v2xz = vec3(v2.x, 0, v2.z);
        vec3 p = vec3(x_, 0, z_);
        float w0 = length(cross(v1xz - v0xz, p - v0xz)) / length(cross(v1xz - v0xz, v2xz - v0xz));
        float w1 = length(cross(v2xz - v1xz, p - v1xz)) / length(cross(v2xz - v1xz, v0xz - v1xz));
        float w2 = length(cross(v0xz - v2xz, p - v2xz)) / length(cross(v0xz - v2xz, v1xz - v2xz));
        if(w0 >= 0 && w1 >= 0 && w2 >= 0){
            nearestTriIndex = triIndex;
            break;
        }
    }
    if(nearestTriIndex == -1){
        return vector<vec3>();
    }
    GLuint* verts = island.getFace(nearestTriIndex);
    vec3 v0 = vec3(island.getVert(verts[0])[0],island.getVert(verts[0])[1],island.getVert(verts[0])[2]);
    vec3 v1 = vec3(island.getVert(verts[1])[0],island.getVert(verts[1])[1],island.getVert(verts[1])[2]);
    vec3 v2 = vec3(island.getVert(verts[2])[0],island.getVert(verts[2])[1],island.getVert(verts[2])[2]);
    return vector<vec3>{v0, v1, v2};
}

vec3 worldCoordsToTerrainGradient(float x_, float z_){
    float leftY = worldCoordsToTerrainHeight(x_ - d, z_);
    float rightY = worldCoordsToTerrainHeight(x_ + d, z_);
    float frontY = worldCoordsToTerrainHeight(x_, z_ + d);
    float backY = worldCoordsToTerrainHeight(x_, z_ - d);
    vec3 gradient = vec3(leftY - rightY, 2 * d, backY - frontY);
    gradient = normalize(gradient);
    return gradient;
}

vec3 worldCoordsToTerrainNormal(float x_, float z_) {
    std::vector<vec3> verts = getVertsOfNearestTerrainTri(x_, z_);
    if(verts.size() == 0){
        return vec3(0, 1, 0);
    }
    vec3 a = verts[0];
    vec3 b = verts[1];
    vec3 c = verts[2];
    u = b - a;
    v = c - a;
    vec3 normal = normalize(cross(b - a, c - a));
    return normal;
}

class Sheep {
    public:
        Sheep() {
            coords = vec3(0.1, 0.1, 0);
            rotation = identity<mat3>();
        }
        void jump() {
            midJump = true;
        }
        void update(float dt, float elapsedTime){
            coords.y = worldCoordsToTerrainHeightWithFloat(coords.x, coords.z);
            // set the rotation of the sheep to be parallel to the terrain
            gradient = worldCoordsToTerrainGradient(coords.x, coords.z);
            normal = worldCoordsToTerrainNormal(coords.x, coords.z);
            right = normalize(cross(normal, gradient));
            rotation = mat3(right, normal, gradient);
            if(midJump){
                velocity += acceleration * dt;
                coords += velocity * dt;
                if(coords.y < worldCoordsToTerrainHeightWithFloat(coords.x, coords.z)){
                    coords.y = worldCoordsToTerrainHeightWithFloat(coords.x, coords.z);
                    midJump = false;
                }
            }
        }
        vec3 coords;
        vec3 normal;
        vec3 gradient;
        vec3 right;
        mat3 rotation;

        bool midJump = false;
        vec3 goalCoords;
        vec3 acceleration = vec3(0, -9.8, 0);
        vec3 velocity = vec3(0);
};

class MeshViewer : public Window {
   public:
    MeshViewer() : Window() {}

    void refreshShaders() {
        renderer.loadShader("bloom", "../shaders/bloom.vs",
                            "../shaders/bloom.fs");
        renderer.loadShader("island", "../shaders/island.vs",
                            "../shaders/island.fs");
        renderer.loadShader("sheep", "../shaders/sheep.vs",
                            "../shaders/sheep.fs");    
        renderer.loadShader("sky", "../shaders/sky.vs", "../shaders/sky.fs");
    }

    void setup() {
        // island = PLYMesh("../models/island.ply");
        sheepMesh = PLYMesh("../models/sheep.ply");
        refreshShaders();
        renderer.loadRenderTexture("view", 0, width() * MSAA, height() * MSAA);
        setupTerrain();
        for (int i = 0; i < 1; i++) {
            Sheep s = Sheep();
            sheep.push_back(s);
        }
    }

    void mouseMotion(int x, int y, int dx, int dy) {
        if (left_mousedown && shiftdown) {
            radius = std::max(1.0f, radius - dy * 0.1f);
        }
        if (left_mousedown && !shiftdown) {
            azimuth = azimuth + dx * 0.01f;
            float maxElevation = 3.14159f / 2.0f - 0.01f;
            elevation = std::min(
                maxElevation,
                std::max(-1.0f * maxElevation, elevation + dy * 0.01f));
        }
    }

    void mouseDown(int button, int mods) {
        if (button == GLFW_MOUSE_BUTTON_LEFT) {
            left_mousedown = true;
        }
        if (button == GLFW_MOUSE_BUTTON_RIGHT) {
            right_mousedown = true;
        }
        if (button == GLFW_MOUSE_BUTTON_MIDDLE) {
            right_mousedown = true;
        }
    }

    void mouseUp(int button, int mods) {
        if (button == GLFW_MOUSE_BUTTON_LEFT) {
            left_mousedown = false;
        }
        if (button == GLFW_MOUSE_BUTTON_RIGHT) {
            right_mousedown = false;
        }
        if (button == GLFW_MOUSE_BUTTON_MIDDLE) {
            right_mousedown = false;
        }
    }

    void scroll(float dx, float dy) {
        radius = std::max(1.0f, radius - dy * 0.1f);
        zoom = std::max(0.1f, zoom + dy * 0.1f);
    }

    void keyDown(int key, int mods) {
        if (key == GLFW_KEY_LEFT_SHIFT || key == GLFW_KEY_RIGHT_SHIFT) {
            shiftdown = true;
        }
        if (key == GLFW_KEY_COMMA && !(perioddown)) {
            commadown = true;
        }
        if (key == GLFW_KEY_PERIOD && !(commadown)) {
            perioddown = true;
        }
    }

    void keyUp(int key, int mods) {
        if (key == GLFW_KEY_LEFT_SHIFT || key == GLFW_KEY_RIGHT_SHIFT) {
            shiftdown = false;
        }
        if (key == GLFW_KEY_COMMA) {
            commadown = false;
        }
        if (key == GLFW_KEY_PERIOD) {
            perioddown = false;
        }
    }

    void calculateView() {
        float w = viewWidth;
        renderer.ortho(-w * zoom, w * zoom, (-w + orthoOffset.y) * zoom, (w + orthoOffset.y) * zoom, -10, 500);
        eyePos = vec3(radius * cos(azimuth) * cos(elevation),
                      radius * sin(elevation),
                      radius * sin(azimuth) * cos(elevation));

        vec3 camZ = normalize(eyePos - lookPos);
        vec3 camX = normalize(cross(vec3(0, 1, 0), camZ));
        vec3 camY = normalize(cross(camZ, camX));
        up = camY;
        renderer.lookAt(eyePos, lookPos, up);
    }

    void configureLights() {
        for (int i = 1; i <= std::min(lightPositions.size(),
                                      lightSourceIntensities.size());
             i++) {
            renderer.setUniform("lightPosition" + std::to_string(i),
                                lightPositions[i - 1]);
            renderer.setUniform("lightSourceIntensity" + std::to_string(i),
                                lightSourceIntensities[i - 1]);
        }
    }

    void setupTerrain() {
        int nIslandVerts = island.numVertices();
        int nTerrainVerts = 0;
        float maxY = 0;
        float minX = NULL;
        float maxX = NULL;
        float minZ = NULL;
        float maxZ = NULL;
        for (int i = 0; i < nIslandVerts; i++) {
            GLfloat* v = island.getVert(i);
            if (v[1] > maxY) {
                maxY = v[1];
            }
            if (minX == NULL || v[0] < minX) {
                minX = v[0];
            }
            if (maxX == NULL || v[0] > maxX) {
                maxX = v[0];
            }
            if (minZ == NULL || v[2] < minZ) {
                minZ = v[2];
            }
            if (maxZ == NULL || v[2] > maxZ) {
                maxZ = v[2];
            }
        }
        terrainRadius = std::max(
            abs(maxX), std::max(abs(minX), std::max(abs(maxZ), abs(minZ))));
        for (int i = 0; i < nIslandVerts; i++) {
            GLfloat* v = island.getVert(i);
            vec3 vert = vec3(v[0], v[1], v[2]);
            if (vert.y > maxY - 0.01) {
                terrainVerts.push_back(i);
            }
        }
    }

    void updateTerrain() {
        if (commadown) {
            perlinZ -= 100 * dt();
        }
        if (perioddown) {
            perlinZ += 100 * dt();
        }
        perlin = perlinCrossSection(256, 256, 1, perlinZ);
        for (int i = 0; i < terrainVerts.size(); i++) {
            GLfloat* v = island.getVert(terrainVerts[i]);
            vec3 vert = vec3(v[0], v[1], v[2]);
            island.setVert(terrainVerts[i], vert.x, worldCoordsToTerrainHeight(vert.x, vert.z), vert.z);
        }
        delete[] perlin;
        
        island.computeNormals();
    }

    void drawSky() {
        renderer.beginShader("sky");
        renderer.setUniform("color", vec3(0.9, 0.95, 1));
        renderer.push();
        renderer.scale(vec3(100));
        renderer.cube();
        renderer.pop();
        renderer.endShader();
    }

    void drawIsland() {
        renderer.beginShader("island");
            configureLights();
            renderer.setUniform("ambientCoefficient", 0.9f);
            renderer.setUniform("diffuseCoefficient", 0.4f);
            renderer.setUniform("specularCoefficient", 0.0f);
            renderer.setUniform("shininess", 1.0f);
            renderer.setUniform("albedo", vec3(127, 180, 49) / 255.0f);
            islandFloatHeight = -0.5 + (sin(elapsedTime()) * 0.05);
            renderer.push();
                renderer.rotate(vec3(0));
                renderer.translate(vec3(0, islandFloatHeight, 0));
                renderer.mesh(island);
            renderer.pop();
        renderer.endShader();
    }

    void drawPostProcessedView() {
        renderer.ortho(-viewWidth, viewWidth, (-viewWidth + orthoOffset.y), (viewWidth + orthoOffset.y), -10, 500);
        renderer.beginShader("bloom");
        renderer.setUniform("resolution", vec2(width(), height()));
        renderer.texture("view", "view");
        renderer.push();
        renderer.rotate(atan2(eyePos.x, eyePos.z), vec3(0, 1, 0));
        renderer.rotate(-1.0f * atan2(eyePos.y, sqrt(eyePos.x * eyePos.x +
                                                     eyePos.z * eyePos.z)),
                        vec3(1, 0, 0));
        renderer.translate(vec3(0, orthoOffset.y, 0));
        float scale = 2 * viewWidth;
        renderer.scale(vec3(scale));
        renderer.cube();
        renderer.pop();
        renderer.endShader();
    }

    void drawScene() {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        drawSky();
        drawIsland();
        renderer.beginShader("sheep");
        configureLights();
        renderer.setUniform("ambientCoefficient", 0.9f);
        renderer.setUniform("diffuseCoefficient", 0.4f);
        renderer.setUniform("specularCoefficient", 0.0f);
        renderer.setUniform("shininess", 1.0f);
        for(int i = 0; i < sheep.size(); i++) {
            sheep[i].update(dt(), elapsedTime());
            renderer.push();
            renderer.translate(sheep[i].coords);
            renderer.rotate(sheep[i].rotation);
            renderer.mesh(sheepMesh);
            renderer.scale(vec3(0.05));
            renderer.plane();
            renderer.pop();
        }
        renderer.endShader();
        renderer.beginShader("lines");
        renderer.setUniform("color", vec3(0.9, 0.95, 1));
        renderer.push();
        renderer.line(sheep[0].coords, sheep[0].coords + sheep[0].normal, vec3(1,0,0), vec3(1,0,0));
        renderer.line(sheep[0].coords, sheep[0].coords + sheep[0].gradient, vec3(0,1,0), vec3(0,1,0));
        renderer.line(sheep[0].coords, sheep[0].coords + sheep[0].right, vec3(0,0,1), vec3(0,0,1));
        renderer.line(sheep[0].coords, sheep[0].coords + u, vec3(1,0,1), vec3(1,0,1));
        renderer.line(sheep[0].coords, sheep[0].coords + v, vec3(1,1,0), vec3(1,1,0));
        renderer.endShader();
        renderer.beginShader("unlit");
        renderer.setUniform("color", vec3(1, 0, 0));
        renderer.push();
        renderer.translate(a);
        renderer.scale(vec3(0.01));
        renderer.sphere();
        renderer.pop();
        
    }

    void draw() {
        refreshShaders();
        renderer.blendMode(BlendMode::BLEND);
        updateTerrain();
        renderer.beginRenderTexture("view");
        drawScene();
        renderer.endRenderTexture();
        drawPostProcessedView();
        calculateView();
    }

   protected:
    float fovDegrees = 10.0f;
    int meshIndex = 0;
    int shaderIndex = 0;
    float meshScale = 1.0f;
    float radius = 5.0f;
    float azimuth = 0.0f;
    float elevation = 0.0f;
    vec3 eyePos = vec3(10, 0, 0);
    vec3 lookPos = vec3(0, 0, 0);
    vec3 up = vec3(0, 1, 0);
    bool left_mousedown = false;
    bool right_mousedown = false;
    bool middle_mousedown = false;
    bool shiftdown = false;
    bool commadown = false;
    bool perioddown = false;
    float perlinZ = 0.1;
    float viewWidth = 1.1;
    int MSAA = 2;
    float zoom = 1.0f;
    vec3 orthoOffset = vec3(0, 0.1, 0);
    std::vector<vec4> lightPositions = {
        vec4(24, 10, 12, 1), vec4(9, 10, -12, 1), vec4(-9, 10, -12, 1)};
    std::vector<vec3> lightSourceIntensities = {
        vec3(0.8, 0.8, 0.8),
        vec3(0.8, 0.8, 0.8),
        vec3(0.8, 0.8, 0.8),
    };
    std::vector<int> terrainVerts;
    std::vector<Sheep> sheep;
    agl::PLYMesh sheepMesh;
};

int main(int argc, char** argv) {
    MeshViewer viewer;
    viewer.run();
    return 0;
}
