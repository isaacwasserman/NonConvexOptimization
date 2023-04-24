#version 400

layout (location = 0) in vec3 vPos;
layout (location = 1) in vec3 vNormals;
layout (location = 2) in vec2 vTextureCoords;

uniform mat3 NormalMatrix;
uniform mat4 ModelViewMatrix;
uniform mat4 MVP;
uniform bool HasUV;
uniform vec4 lightPosition;

out vec3 eyeNorm;
out vec4 eyePos;
out vec4 lightPositionEyeSpace;

void main()
{
   eyeNorm = normalize(NormalMatrix * vNormals);
   eyePos = ModelViewMatrix * vec4(vPos, 1.0);
   lightPositionEyeSpace = ModelViewMatrix * lightPosition;
   gl_Position = MVP * vec4(vPos, 1.0);
}
