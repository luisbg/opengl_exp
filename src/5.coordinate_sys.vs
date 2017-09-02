#version 330 core
layout (location = 0) in vec3 aPos;  // position variable has attribute position 0
layout (location = 1) in vec3 aColor;  // the color variable has attribute position 1
layout (location = 2) in vec2 aTexCoord;  // the texture coordinates has attribute position 2

out vec3 ourColor;
out vec2 TexCoord;

uniform mat4 transform;

void main()
{
    gl_Position = transform * vec4(aPos, 1.0f);
    ourColor = aColor;
    TexCoord = vec2(aTexCoord.x, aTexCoord.y);
}
