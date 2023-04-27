#version 400

// layout (location = 0) out vec4 FragColor;
// layout (location = 1) out vec4 FragColor_bright;

out vec4 FragColor;
out vec4 FragColor_bright;

in vec4 eyePos;
in vec3 eyeNorm;
in vec3 ReflectDir;
in vec3 worldPos;
in vec3 worldNorm;

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

void main() {
  vec3 redAlbedo = vec3(200, 27, 27) / 255.0;
  vec3 brownAlbedo = vec3(70, 12, 12) / 255.0;
  vec3 whiteAlbedo = vec3(0.9f);

  vec4 cubeMapColor = texture(cubemap, ReflectDir);
  cubeMapColor = vec4(0.0f);

  vec3 surfaceNormal = normalize(cross(dFdx(eyePos.xyz), dFdy(eyePos.xyz)));
  vec3 n = normalize(surfaceNormal);
  vec3 v = normalize(-eyePos.xyz);

  float blend1 = 0.0f;
  blend1 = (dot(normalize(worldNorm), vec3(0, 1, 0)) > 0.1f)
          && ((abs(dot(normalize(worldNorm), vec3(0,0,1))) < 0.5) || worldPos.y > 2.7f) ? 0.0f : 1.0f;
  float blend2 = 0.0f;
  blend2 = abs(worldPos.z) > 1.18f ? 1.0f : 0.0f;
  vec3 blendedAlbedo = mix(brownAlbedo, redAlbedo, blend1);

  vec3 redWhiteBlend = mix(redAlbedo, whiteAlbedo, blend2);
  blendedAlbedo = mix(brownAlbedo, redWhiteBlend, blend1);

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
  if (length(lightSourceIntensity1) > 0) {
    averageIntensity += lightSourceIntensity1;
    denominator++;
  }
  if (length(lightSourceIntensity2) > 0) {
    averageIntensity += lightSourceIntensity2;
    denominator++;
  }
  if (length(lightSourceIntensity3) > 0) {
    averageIntensity += lightSourceIntensity3;
    denominator++;
  }
  averageIntensity /= denominator;

  vec3 ambientReceived = (averageIntensity)*ambientCoefficient;
  vec3 diffuseReceived = vec3(0);
  diffuseReceived +=
      diffuseCoefficient * lightSourceIntensity1 * max(dot(s1, n), 0.0);
  diffuseReceived +=
      diffuseCoefficient * lightSourceIntensity2 * max(dot(s2, n), 0.0);
  diffuseReceived +=
      diffuseCoefficient * lightSourceIntensity3 * max(dot(s3, n), 0.0);

  vec3 diffuseEnv =
      diffuseCoefficient * cubeMapColor.xyz * max(dot(sEnv, nEnv), 0.0);
  diffuseReceived += diffuseEnv;

  vec3 specularReceived = vec3(0);
  specularReceived += specularCoefficient * lightSourceIntensity1 *
                      pow(max(dot(r1, v), 0.0), shininess);
  specularReceived += specularCoefficient * lightSourceIntensity2 *
                      pow(max(dot(r2, v), 0.0), shininess);
  specularReceived += specularCoefficient * lightSourceIntensity3 *
                      pow(max(dot(r3, v), 0.0), shininess);

  vec3 specularEnv = specularCoefficient * cubeMapColor.xyz *
                     pow(max(dot(sEnv, v), 0.0), shininess);
  specularReceived += specularEnv;

  vec3 received = ambientReceived + diffuseReceived + specularReceived;

  FragColor = vec4(blendedAlbedo * received, 1.0);

  float brightness = dot(FragColor.rgb, vec3(0.2126, 0.7152, 0.0722));
  if (brightness > 1.0) {
    FragColor_bright = FragColor;
  } else {
    FragColor_bright = vec4(0.0, 0.0, 0.0, 1.0);
  }
}
