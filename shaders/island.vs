#version 400
#extension GL_EXT_shader_derivative : enable

layout (location = 0) in vec3 vPos;
layout (location = 1) in vec3 vNormals;
layout (location = 2) in vec2 vTextureCoords;

uniform mat3 NormalMatrix;
uniform mat4 ModelViewMatrix;
uniform mat4 MVP;
uniform bool HasUV;
uniform vec4 lightPosition1;
uniform vec4 lightPosition2;
uniform vec4 lightPosition3;

out vec3 eyeNorm;
out vec4 eyePos;
out vec3 ReflectDir;
out vec3 worldNorm;
out vec3 worldPos;
out vec4 lightPositionEyeSpace1;
out vec4 lightPositionEyeSpace2;
out vec4 lightPositionEyeSpace3;

void main()
{
   ReflectDir = vPos;
   eyeNorm = normalize(NormalMatrix * vNormals);
   eyePos = ModelViewMatrix * vec4(vPos, 1.0);
   if(lightPosition1 != vec4(0.0, 0.0, 0.0, 0.0)){
      lightPositionEyeSpace1 = ModelViewMatrix * lightPosition1;
   }
   else {
      lightPositionEyeSpace1 = vec4(0.0, 0.0, 0.0, 0.0);
   }
   if(lightPosition2 != vec4(0.0, 0.0, 0.0, 0.0)){
      lightPositionEyeSpace2 = ModelViewMatrix * lightPosition2;
   }
   else {
      lightPositionEyeSpace2 = vec4(0.0, 0.0, 0.0, 0.0);
   }
   if(lightPosition3 != vec4(0.0, 0.0, 0.0, 0.0)){
      lightPositionEyeSpace3 = ModelViewMatrix * lightPosition3;
   }
   else {
      lightPositionEyeSpace3 = vec4(0.0, 0.0, 0.0, 0.0);
   }
   worldNorm = vNormals;
   worldPos = vPos;
   gl_Position = MVP * vec4(vPos, 1.0);
}
