#version 400

layout (location = 0) in vec3 vPos;
layout (location = 1) in vec3 vNormals;
layout (location = 2) in vec2 vTextureCoords;

out vec3 lightReceived;

uniform vec4 lightPosition;
uniform vec3 reflectivity;
uniform vec3 lightSourceIntensity;

uniform mat3 NormalMatrix;
uniform mat4 ModelViewMatrix;
uniform mat4 MVP;
uniform bool HasUV;

void main()
{
   vec3 eyeNorm = normalize(NormalMatrix * vNormals);
   vec4 eyePos = ModelViewMatrix * vec4(vPos, 1.0);

   vec3 s = normalize(vec3(lightPosition - eyePos));
   
   vec3 diffuseReceived = reflectivity * lightSourceIntensity * max(dot(s, eyeNorm), 0.0);

   vec3 ambientReceived = reflectivity * lightSourceIntensity * 0.2;

   vec3 r = -1.0 * s + 2.0 * dot(s, eyeNorm) * eyeNorm;

   vec3 specularReceived = reflectivity * lightSourceIntensity * pow(max(dot(r, normalize(-eyePos.xyz)), 0.0), 100.0);

   lightReceived = ambientReceived + diffuseReceived + specularReceived;

   gl_Position = MVP * vec4(vPos, 1.0);
}