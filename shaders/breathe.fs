#version 400

out vec4 FragColor;

in vec4 eyePos;
in vec3 eyeNorm;

uniform vec4 lightPosition;
uniform vec3 reflectivity;
uniform vec3 lightSourceIntensity;

void main()
{
   vec3 n = normalize(eyeNorm);
   vec3 s = normalize(lightPosition.xyz - eyePos.xyz);
   vec3 v = normalize(-eyePos.xyz);
   vec3 r = reflect(-s, n);
   
   vec3 ambientReceived = reflectivity * lightSourceIntensity * 0.2;
   vec3 diffuseReceived = reflectivity * lightSourceIntensity * max(dot(s, n), 0.0);
   vec3 specularReceived = reflectivity * lightSourceIntensity * pow(max(dot(r, v), 0.0), 100.0);
   vec3 received = ambientReceived + diffuseReceived + specularReceived;

   FragColor = vec4(received, 1.0);
}
