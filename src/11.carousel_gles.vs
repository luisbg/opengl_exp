uniform mat4 u_mvpMatrix;
uniform float xOffset;
uniform float yOffset;

attribute vec4 v_position;
attribute vec2 a_texCoord;

varying vec2 v_texCoord;

void main()
{
    gl_Position = vec4(v_position.x + xOffset, v_position.y + yOffset,
                       v_position.z, 1.0) * u_mvpMatrix;
    v_texCoord = a_texCoord;
}
