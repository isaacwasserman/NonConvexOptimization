//--------------------------------------------------
// Author: Isaac Wasserman
// Date: 02/28/2023
// Description: Views PLY files
//--------------------------------------------------

#include <cmath>
#include <string>
#include <vector>
#include <map>
#include <chrono>
#include <ctime>

#include "agl/image.h"
#include "agl/window.h"
#include "osutils.h"
#include "perlin.h"
#include "plymesh.h"

using namespace std;
using namespace glm;
using namespace agl;

// set seed to current time in seconds
// unsigned int seed = (unsigned int)std::chrono::system_clock::now().time_since_epoch().count();
unsigned int seed = 2548309051;

float* perlin = NULL;
float terrainRadius = 1.0f;
float islandFloatHeight = 0.0f;
float d = 0.02;
std::vector<int> terrainVerts;

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
    int ind = std::min(255*255, abs((int)((x + 1.0f) * 128.0f) * 256 + (int)((z + 1.0f) * 128.0f)));
    float y = perlin[ind];
    // std::cout << "y: " << y << std::endl;
    float edgeDecay = -0.8 * (length(vec2(x, z)) * length(vec2(x, z)));
    float heightScale = 1.0f;
    return y * (heightScale + edgeDecay);
}

float worldCoordsToTerrainHeightWithFloat(float x_, float z_) {
    return worldCoordsToTerrainHeight(x_, z_) + islandFloatHeight;
}

std::vector<vec3> getVertsOfNearestTerrainTri(float x_, float z_){
    int nTris = island.numTriangles();
    int nearestTriIndex = -1;
    for(int triIndex = 0; triIndex < nTris; triIndex++){
        GLuint* verts = island.getFace(triIndex);
        bool allInTerrainVerts = true;
        for(int k = 0; k < 3; k++){
            bool inTerrainVerts = false;
            for(int j = 0; j < terrainVerts.size(); j++){
                if(verts[k] == terrainVerts[j]){
                    inTerrainVerts = true;
                    break;
                }
            }
            if(!inTerrainVerts){
                allInTerrainVerts = false;
                break;
            }
        }
        if(!allInTerrainVerts){
            continue;
        }
        vec3 v0 = vec3(island.getVert(verts[0])[0],0.0f,island.getVert(verts[0])[2]);
        vec3 v1 = vec3(island.getVert(verts[1])[0],0.0f,island.getVert(verts[1])[2]);
        vec3 v2 = vec3(island.getVert(verts[2])[0],0.0f,island.getVert(verts[2])[2]);
        
        vec3 p = vec3(x_, 0.0f, z_);
        vec3 v0v1 = v1 - v0;
        vec3 v0v2 = v2 - v0;
        vec3 v0p = p - v0;
        float d00 = dot(v0v1, v0v1);
        float d01 = dot(v0v1, v0v2);
        float d11 = dot(v0v2, v0v2);
        float d20 = dot(v0p, v0v1);
        float d21 = dot(v0p, v0v2);
        float denom = d00 * d11 - d01 * d01;
        if(denom == 0.0f){
            continue;
        }
        float v = (d11 * d20 - d01 * d21) / denom;
        float w = (d00 * d21 - d01 * d20) / denom;
        float u = 1.0f - v - w;
        if(u >= 0 && u <= 1 && v >= 0 && v <= 1 && w >= 0 && w <= 1){
            nearestTriIndex = triIndex;
            break;
        }
    }
    if(nearestTriIndex == -1){
        return vector<vec3>{vec3(0), vec3(0), vec3(0)};
    }
    else {
        GLuint* verts = island.getFace(nearestTriIndex);
        vec3 v0 = vec3(island.getVert(verts[0])[0],island.getVert(verts[0])[1],island.getVert(verts[0])[2]);
        vec3 v1 = vec3(island.getVert(verts[1])[0],island.getVert(verts[1])[1],island.getVert(verts[1])[2]);
        vec3 v2 = vec3(island.getVert(verts[2])[0],island.getVert(verts[2])[1],island.getVert(verts[2])[2]);
        return vector<vec3>{v0, v1, v2};
    }
}

quat slerp(quat q1, quat q2, float t) {
    float dot = glm::dot(q1, q2);
    if(dot < 0.0f){
        q1 = -q1;
        dot = -dot;
    }
    if(dot > 0.9995f){
        return glm::normalize(q1 + t * (q2 - q1));
    }
    float theta0 = acos(dot);
    float theta = theta0 * t;
    float sinTheta = sin(theta);
    float sinTheta0 = sin(theta0);
    float s0 = cos(theta) - dot * sinTheta / sinTheta0;
    float s1 = sinTheta / sinTheta0;
    return (s0 * q1) + (s1 * q2);
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

vec3 findPeak(){
    float max = 0.0f;
    vec3 peak = vec3(0.0f, 0.0f, 0.0f);
    for(int i = 0; i < terrainVerts.size(); i++){
        vec3 v = vec3(island.getVert(terrainVerts[i])[0], island.getVert(terrainVerts[i])[1], island.getVert(terrainVerts[i])[2]);
        if(v.y > max){
            max = v.y;
            peak = v;
        }
    }
    return peak;
}

class Keyframe {
    public:
        Keyframe(){}
        void set(std::string attribute, float value){
            attributes[attribute] = value;
        }
        std::map<std::string, float> attributes;
};

class Animation {
    public:
        Animation(){}
        void addKeyframe(Keyframe keyframe, float frameTime, float currentTime){
            keyframes.push_back(keyframe);
            times.push_back(frameTime);
        }
        float get(std::string attribute, float currentTime){
            if(times.size() == 0){
                return 0.0f;
            }
            if(times.size() == 1){
                return keyframes[0].attributes[attribute];
            }
            Keyframe f1, f2;
            float t1 = -1;
            float t2 = -1;
            for(int i = 0; i < times.size() - 1; i++){
                if(times[i] <= currentTime && times[i + 1] >= currentTime){
                    f1 = keyframes[i];
                    f2 = keyframes[i + 1];
                    t1 = times[i];
                    t2 = times[i + 1];
                    break;
                }
            }
            if(t1 == -1){
                return keyframes[keyframes.size() - 1].attributes[attribute];
            }
            float t;
            if(t1 > t2){
                t = 1.0f;
            }
            else {
                t = (currentTime - t1) / (t2 - t1);
            }
            float value = f1.attributes[attribute] * (1 - t) + f2.attributes[attribute] * t;
            return value;
        }
        std::vector<float> times;
        std::vector<Keyframe> keyframes;
};

class Sheep {
    public:
        Sheep() {
            GLfloat far[3] = {-99.0f, -99.0f, -99.0f};
            int startVertIndex = -1;
            GLfloat* start_coords = far;
            while(length(vec3(start_coords[0], start_coords[1], start_coords[2])) > 0.8f){
                startVertIndex = (rand() * seed) % terrainVerts.size();
                start_coords = island.getVert(startVertIndex);
            }
            coords = vec3(start_coords[0], 0, start_coords[2]);
            Keyframe init;
            init.set("x", coords.x);
            init.set("z", coords.z);
            animation.addKeyframe(init, 0, 0);
            jumpStagger = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
            rotation = identity<mat3>();
        }
        void jump(float dt, float elapsedTime) {
            hasJumped = true;
            lastJumpTime = elapsedTime;
            startPoint = coords;
            goal = coords + (gradient * (0.1f * vec3(1, 0, 1)));
            goal.y = worldCoordsToTerrainHeightWithFloat(goal.x, goal.z);
            std::vector<vec3> verts = getVertsOfNearestTerrainTri(goal.x, goal.z);
            if(verts[0] == vec3(0) && verts[1] == vec3(0) && verts[2] == vec3(0) && !isDead){
                isDead = true;
                goal.y = -10;
            }
            Keyframe start;
            start.set("x", coords.x);
            start.set("z", coords.z);
            animation.addKeyframe(start, elapsedTime, elapsedTime);
            Keyframe end;
            end.set("x", goal.x);
            end.set("z", goal.z);
            animation.addKeyframe(end, elapsedTime + jumpTime, elapsedTime);
        }
        mat3 rotationForPosition(float x_, float z_){
            normal = worldCoordsToTerrainNormal(x_, z_);
            gradient = vec3(0, 1, 0);
            right = normalize(cross(normal, gradient));
            gradient = normalize(cross(normal, right));
            if(abs(right.y) > abs(gradient.y)){
                vec3 temp = right;
                right = gradient;
                gradient = temp;
            }
            if(gradient.y > 0){
                gradient = -gradient;
            }
            right = normalize(cross(normal, gradient));
            rotation = mat3(normalize(right), normalize(normal), normalize(gradient));
            return rotation;
        }
        bool isJumping(float elapsedTime){
            return hasJumped && (elapsedTime - lastJumpTime < jumpTime) && !isDead;
        }
        bool isFalling(){
            return isDead && goal.y <= -10;
        }
        void capture(){
            isCaptured = true;
            shouldDraw = false;
        }
        void update(float dt, float elapsedTime){
            if(elapsedTime - lastJumpTime > timeBetweenJumps && elapsedTime > timeBetweenJumps + jumpStagger){
                lastJumpTime = elapsedTime;
                jump(dt, elapsedTime);
            }
            coords.x = animation.get("x", elapsedTime);
            coords.z = animation.get("z", elapsedTime);
            coords.y = worldCoordsToTerrainHeightWithFloat(coords.x, coords.z);
            // std::cout << "coords: " << coords.x << ", " << coords.y << ", " << coords.z << std::endl;
            float t = (elapsedTime - lastJumpTime);
            if(isJumping(elapsedTime) && t > 0){
                float yOffset = -t * (t - jumpTime) * (jumpHeight / jumpTime);
                coords.y += yOffset;
                quat startRotation = quat(rotationForPosition(startPoint.x, startPoint.z));
                quat goalRotation = quat(rotationForPosition(goal.x, goal.z));
                quat interpolatedRotation = slerp(startRotation, goalRotation, t / jumpTime);
                rotation = mat3(interpolatedRotation);
            }
            else if(isFalling()){
                float yOffset = -t * (t - jumpTime) * (jumpHeight / jumpTime);
                coords.y += yOffset;
            }
            else {
                rotation = rotationForPosition(coords.x, coords.z);
            }
        }

        vec3 normal;
        vec3 gradient;
        vec3 right;

        vec3 coords;
        mat3 rotation;

        vec3 startPoint;
        vec3 goal;

        Animation animation;
        bool hasJumped = false;
        float lastJumpTime = 0.0f;
        float jumpTime = 0.5f;
        float jumpHeight = 0.5f;
        float timeBetweenJumps = 2.0f;
        float jumpStagger = 0.0f;

        bool isDead = false;
        bool isCaptured = false;
        bool shouldDraw = true;
};

class MeshViewer : public Window {
   public:
    MeshViewer() : Window() {}

    void refreshShaders() {
        std::vector<string> shaderNames = {"bloom", "island", "sheep", "sky", "barn", "text"};
        for(string name : shaderNames){
            renderer.loadShader(name, "../shaders/" + name + ".vs", "../shaders/" + name + ".fs");
        }
    }

    void reset(){
        sheep.clear();
        for (int i = 0; i < 10; i++) {
            Sheep s = Sheep();
            sheep.push_back(s);
        }
        GLfloat far[3] = {-99.0f, -99.0f, -99.0f};
        int startVertIndex = -1;
        GLfloat* start_coords = far;
        while(length(vec3(start_coords[0], start_coords[1], start_coords[2])) > 0.5f){
            startVertIndex = (rand() * seed) % terrainVerts.size();
            start_coords = island.getVert(startVertIndex);
        }
        barnSpawn = vec3(start_coords[0], 0, start_coords[2]);
    }

    void setup() {
        sheepMesh = PLYMesh("../models/sheep.ply");
        barnMesh = PLYMesh("../models/barn_blend.ply");
        refreshShaders();
        renderer.loadRenderTexture("view", 5, width() * MSAA, height() * MSAA);
        setupTerrain();
        reset();
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
        if (key == GLFW_KEY_R) {
            reset();
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

    bool intersectsBarn(vec3 pos) {
        float x = pos.x;
        float z = pos.z;
        float barnX = barnSpawn.x;
        float barnZ = barnSpawn.z;
        float barnRadius = 0.15f;
        float dist = length(vec2(x,z) - vec2(barnX, barnZ));
        return dist < barnRadius;
    }

    void drawSheep(){
        renderer.beginShader("sheep");
        configureLights();
        renderer.setUniform("ambientCoefficient", 0.9f);
        renderer.setUniform("diffuseCoefficient", 0.4f);
        renderer.setUniform("specularCoefficient", 0.0f);
        renderer.setUniform("shininess", 1.0f);
        for(int i = 0; i < sheep.size(); i++) {
            if(sheep[i].shouldDraw){
                if(intersectsBarn(sheep[i].coords) && !sheep[i].isCaptured){
                    sheep[i].capture();
                    numHerdedSheep++;
                }
                bool wasDead = sheep[i].isDead;
                sheep[i].update(dt(), elapsedTime());
                bool isDead = sheep[i].isDead;
                if(!wasDead && isDead){
                    numDeadSheep++;
                }
                renderer.push();
                    renderer.translate(sheep[i].coords);
                    renderer.rotate(sheep[i].rotation);
                    renderer.scale(vec3(2.0f));
                    renderer.mesh(sheepMesh);
                renderer.pop();
            }
        }
        renderer.endShader();
    }

    void drawBarn(){
        vec3 normal = worldCoordsToTerrainNormal(barnSpawn.x, barnSpawn.z);
        vec3 gradient = vec3(0, 1, 0);
        vec3 right = normalize(cross(normal, gradient));
        gradient = normalize(cross(normal, right));
        if(abs(right.y) > abs(gradient.y)){
            vec3 temp = right;
            right = gradient;
            gradient = temp;
        }
        if(gradient.y > 0){
            gradient = -gradient;
        }
        right = normalize(cross(normal, gradient));
        mat3 rotation = mat3(normalize(right), normalize(normal), normalize(gradient));
        renderer.beginShader("barn");
        configureLights();
        renderer.setUniform("ambientCoefficient", 0.9f);
        renderer.setUniform("diffuseCoefficient", 0.4f);
        renderer.setUniform("specularCoefficient", 0.0f);
        renderer.setUniform("shininess", 1.0f);
        renderer.push();
            renderer.translate(barnSpawn + vec3(0, worldCoordsToTerrainHeightWithFloat(barnSpawn.x, barnSpawn.z),0));
            renderer.rotate(rotation);
            renderer.scale(vec3(0.1f));
            renderer.mesh(barnMesh);
        renderer.pop();
        renderer.endShader();
    }

    void drawScene() {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        renderer.beginShader("text");
            renderer.push();
                renderer.fontColor(vec4(0.1, 0.1, 0.1, 1));
                renderer.fontSize(50.0f);
                renderer.text("Sheep Herded: " + to_string(numHerdedSheep) + "                         Sheep Lost: " + to_string(numDeadSheep), 50.0f, 80.0f);
            renderer.pop();
        renderer.endShader();
        drawIsland();
        drawSheep();
        drawBarn();
        
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
    std::vector<Sheep> sheep;
    agl::PLYMesh sheepMesh;
    agl::PLYMesh barnMesh;
    int numDeadSheep = 0;
    int numHerdedSheep = 0;
    vec3 barnSpawn = vec3(0, 0, 0);
};

int main(int argc, char** argv) {
    MeshViewer viewer;
    viewer.run();
    return 0;
}
