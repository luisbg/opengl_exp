uniform mat4 u_mvpMatrix;
attribute vec4 v_position;
attribute vec2 a_texCoord;
varying vec2 v_texCoord;

void main()
{
    gl_Position = v_position * u_mvpMatrix;
    v_texCoord = a_texCoord;
}
