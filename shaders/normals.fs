#version 400
in vec3 Normal;
out vec4 FragColor;
void main()
{
   vec3 new_normal = 0.5 * (Normal + 1.0);
   FragColor = vec4(new_normal.x, new_normal.y, new_normal.z, 1.0);
}
