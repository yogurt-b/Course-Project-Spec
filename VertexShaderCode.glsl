#version 330 

in layout(location=0) vec3 position;
in layout(location=1) vec2 vertexUV;
in layout(location=2) vec3 normal;

out vec2 UV;
out vec3 FragPos;
out vec3 Normal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    vec4 v = vec4(position, 1.0);
    FragPos = vec3(model * v);
    Normal = mat3(transpose(inverse(model))) * normal;  
    vec4 out_posotion = projection * view * model * v;
    gl_Position = out_posotion;

	UV = vertexUV;
}