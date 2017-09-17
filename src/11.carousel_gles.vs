uniform float xOffset;
uniform float yOffset;
uniform float zOffset;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

attribute vec4 v_position;
attribute vec2 a_texCoord;

varying vec2 v_texCoord;

void main()
{
    gl_Position = projection * view * model *
                  vec4(v_position.x + xOffset, v_position.y + yOffset,
                       v_position.z + zOffset, 1.0f);
    v_texCoord = a_texCoord;
}
