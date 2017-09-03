project('opengl_experiments', 'cpp')
glewdep = dependency('glew')
glfwdep = dependency('glfw3')
incdir = include_directories('include')
executable('hello', 'src/1.hello.cpp', dependencies : [glewdep, glfwdep])
executable('shaders', 'src/2.shaders.cpp',
	include_directories : incdir,
	dependencies : [glewdep, glfwdep])
executable('textures', 'src/3.textures.cpp',
	include_directories : incdir,
	dependencies : [glewdep, glfwdep])
executable('transformations', 'src/4.transformations.cpp',
	include_directories : incdir,
	dependencies : [glewdep, glfwdep])
executable('coordinate_sys', 'src/5.coordinate_sys.cpp',
	include_directories : incdir,
	dependencies : [glewdep, glfwdep])
executable('camera', 'src/6.camera.cpp',
	include_directories : incdir,
	dependencies : [glewdep, glfwdep])
