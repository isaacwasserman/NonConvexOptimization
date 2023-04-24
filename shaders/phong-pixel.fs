#version 400

out vec4 FragColor;

in vec4 eyePos;
in vec3 eyeNorm;
in vec3 ReflectDir;

in vec4 lightPositionEyeSpace1;
in vec4 lightPositionEyeSpace2;
in vec4 lightPositionEyeSpace3;

uniform vec3 lightSourceIntensity1;
uniform vec3 lightSourceIntensity2;
uniform vec3 lightSourceIntensity3;

uniform vec3 albedo;
uniform float ambientCoefficient;
uniform float diffuseCoefficient;
uniform float specularCoefficient;
uniform float shininess;

uniform samplerCube cubemap;

float random(vec2 seed){
    return fract(sin(dot(seed, vec2(9.12213, 88.411))) * 56299.3178);
}

void main()
{
//    vec4 cubeMapColor = texture(cubemap, ReflectDir);
//    cubeMapColor = vec4(0);

   vec3 surfaceNormal = normalize(cross(dFdx(eyePos.xyz), dFdy(eyePos.xyz)));
   vec3 n = normalize(eyeNorm);
   vec3 v = normalize(-eyePos.xyz);

   vec3 s1 = normalize(lightPositionEyeSpace1.xyz - eyePos.xyz);
   vec3 r1 = reflect(-s1, n);
   vec3 s2 = normalize(lightPositionEyeSpace2.xyz - eyePos.xyz);
   vec3 r2 = reflect(-s2, n);
   vec3 s3 = normalize(lightPositionEyeSpace3.xyz - eyePos.xyz);
   vec3 r3 = reflect(-s3, n);
   vec3 sEnv = normalize(ReflectDir);
   vec3 nEnv = normalize(cross(dFdx(ReflectDir), dFdy(ReflectDir)));


   vec3 averageIntensity = vec3(0);
   int denominator = 0;
   if (length(lightSourceIntensity1) > 0){ averageIntensity += lightSourceIntensity1; denominator++;}
   if (length(lightSourceIntensity2) > 0){ averageIntensity += lightSourceIntensity2; denominator++;}
   if (length(lightSourceIntensity3) > 0){ averageIntensity += lightSourceIntensity3; denominator++;}
   averageIntensity /= denominator;
   
   vec3 ambientReceived = (averageIntensity) * ambientCoefficient;
   vec3 diffuseReceived = vec3(0);
   diffuseReceived += diffuseCoefficient * lightSourceIntensity1 * max(dot(s1, n), 0.0);
   diffuseReceived += diffuseCoefficient * lightSourceIntensity2 * max(dot(s2, n), 0.0);
   diffuseReceived += diffuseCoefficient * lightSourceIntensity3 * max(dot(s3, n), 0.0);

   vec3 diffuseEnv = diffuseCoefficient * cubeMapColor.xyz * max(dot(sEnv, nEnv), 0.0);
   diffuseReceived += diffuseEnv;

   vec3 specularReceived = vec3(0);
   specularReceived += specularCoefficient * lightSourceIntensity1 * pow(max(dot(r1, v), 0.0), shininess);
   specularReceived += specularCoefficient * lightSourceIntensity2 * pow(max(dot(r2, v), 0.0), shininess);
   specularReceived += specularCoefficient * lightSourceIntensity3 * pow(max(dot(r3, v), 0.0), shininess);
   
   vec3 specularEnv = specularCoefficient * cubeMapColor.xyz * pow(max(dot(sEnv, v), 0.0), shininess);
   specularReceived += specularEnv;

   vec3 received = ambientReceived + diffuseReceived + specularReceived;

   FragColor = vec4(albedo * received, 1.0);
   FragColor = vec4(vec3(1), 1.0);
}
