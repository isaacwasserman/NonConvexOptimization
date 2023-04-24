#version 400

out vec4 FragColor;

in vec4 eyePos;
in vec3 eyeNorm;

uniform vec4 lightPosition;
uniform vec3 reflectivity;
uniform vec3 lightSourceIntensity;
uniform float time;

void main()
{
   vec4 lightPos = lightPosition;
   // rotate light source around the origin
   lightPos.x = lightPosition.x * cos(time) - lightPosition.z * sin(time);
   lightPos.z = lightPosition.x * sin(time) + lightPosition.z * cos(time);
   vec3 n = normalize(eyeNorm);
   vec3 s = normalize(lightPos.xyz - eyePos.xyz);
   vec3 v = normalize(-eyePos.xyz);
   vec3 r = reflect(-s, n);
   
   float ambientReceived = 0.2;
   float diffuseReceived = max(dot(s, n), 0.0);
   float specularReceived = pow(max(dot(r, v), 0.0), 100.0);
   vec3 received = reflectivity * lightSourceIntensity * (ambientReceived + diffuseReceived) + (r * 0.6 * diffuseReceived);

   FragColor = vec4(received, 1.0);
}
