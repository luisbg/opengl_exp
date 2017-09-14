uniform mat4 u_mvpMatrix;
attribute vec4 v_position;

void main()
{
    gl_Position = v_position * u_mvpMatrix;
}
