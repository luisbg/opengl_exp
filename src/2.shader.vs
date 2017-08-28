#version 330 core
layout (location = 0) in vec3 aPos;  // position variable has attribute position 0
layout (location = 1) in vec3 aColor;  // the color variable has attribute position 1

out vec3 ourColor;  // output a color to the fragment shader

uniform float xOffset;
uniform float yOffset;

void main()
{
    // add the xOffset to the x position of the vertex position
    // and same with yOffset and the y position
    gl_Position = vec4(aPos.x + xOffset, aPos.y + yOffset, aPos.z, 1.0);
    ourColor = aColor;  // set ourColor to the input color we got from the vertex data
}
