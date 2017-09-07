project('opengl_experiments', 'cpp')
glewdep = dependency('glew')
glfwdep = dependency('glfw3')
sdldep = dependency('sdl2')
sdlimagedep = dependency('SDL2_image')
glesdep = dependency('glesv2')
egldep = dependency('egl')
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
	dependencies : [glewdep, glfwdep, sdldep, sdlimagedep])
# executable('camera_gles', 'src/6b.camera_gles.cpp',
#	include_directories : incdir,
#	dependencies : [sdldep, glesdep, glfwdep])
# executable('tmp', 'src/tmp_opengles.cpp',
# 	include_directories : incdir,
# 	dependencies : [glesdep, egldep])
executable('tmp2', 'src/second_try.cpp',
	dependencies : [sdldep, glesdep])
