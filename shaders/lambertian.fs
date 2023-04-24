#version 400

out vec4 FragColor;

in vec4 eyePos;
in vec3 eyeNorm;
in vec4 lightPositionEyeSpace;

uniform vec4 lightPosition;
uniform vec3 reflectivity;
uniform vec3 lightSourceIntensity;
uniform vec3 albedo;
uniform float attenuation;

void main()
{
   vec3 light_direction = normalize(lightPositionEyeSpace.xyz - eyePos.xyz);
   vec3 normal = normalize(eyeNorm);
   float d = dot(normal, light_direction);

   normal = length(normal) > 0.0 ? normal : vec3(1.0, 0.0, 0.0);

   FragColor = vec4(albedo * lightSourceIntensity * (d * 1.0), 1.0);
}
