#version 400

layout (location = 0) in vec3 vPos;
layout (location = 1) in vec3 vNormals;
layout (location = 2) in vec2 vTextureCoords;

uniform mat3 NormalMatrix;
uniform mat4 ModelViewMatrix;
uniform mat4 MVP;
uniform bool HasUV;
uniform float time;
uniform float meshScale;

out vec3 eyeNorm;
out vec4 eyePos;

float fmod(float x, float y){
   while(x > y){
      x -= y;
   }
   return x;
}

void main()
{
   float mag = 0.3 * length(vPos);
   eyeNorm = normalize(NormalMatrix * vNormals);
   vec4 offset = mag * vec4((0.1 * eyeNorm) * sin(fmod(time, 3.14)), 0.0);
   eyePos = ModelViewMatrix * (vec4(vPos, 1.0) + offset);
   gl_Position = MVP * (vec4(vPos, 1.0) + offset);
}
