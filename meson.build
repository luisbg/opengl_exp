project('opengl_experiments', 'cpp')
glewdep = dependency('glew')
glfwdep = dependency('glfw3')
incdir = include_directories('include')
executable('hello', 'src/1.hello.cpp', dependencies : [glewdep, glfwdep])
executable('shaders', 'src/2.shaders.cpp',
	include_directories : incdir,
	dependencies : [glewdep, glfwdep])
