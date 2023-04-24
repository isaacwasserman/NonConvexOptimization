#version 400

in vec3 lightReceived;

out vec4 FragColor;

void main()
{
   FragColor = vec4(lightReceived, 1.0);
}
