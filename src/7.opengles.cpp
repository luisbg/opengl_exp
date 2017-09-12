#include <GLES2/gl2.h>
#include <EGL/egl.h>

#include <stdlib.h>
#include <iostream>
#include <string.h>
#include <fstream>
#include <sstream>

#define ES_WINDOW_RGB           0
#define ES_WINDOW_ALPHA         1
#define ES_WINDOW_DEPTH         2
#define ES_WINDOW_STENCIL       4
#define ES_WINDOW_MULTISAMPLE   8

typedef struct _context
{
    GLuint programObject;

    GLint width = 320;
    GLint height = 240;
    Display *x_display;

    EGLNativeWindowType  hWnd;
    EGLDisplay  eglDisplay;
    EGLContext  eglContext;
    EGLSurface  eglSurface;

} Context;


GLuint LoadShader(GLenum type, const char *shaderPath)
{
    GLuint shader;
    GLint compiled;
    std::ifstream shaderFile;
    const char* shaderSrc;

    shader = glCreateShader(type);

    if (shader == 0)
        return 0;

    try {
        shaderFile.open(shaderPath);
        std::stringstream shaderStream;
        shaderStream << shaderFile.rdbuf();
        shaderFile.close();

        shaderSrc = shaderStream.str().c_str();
    } catch(std::ifstream::failure e) {
        std::cout << "ERROR::SHADER::FILE_NOT_SUCCESSFULLY_READ" << std::endl;
    }

    glShaderSource(shader, 1, &shaderSrc, NULL);
    glCompileShader(shader);

    glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
    if (!compiled) {
        GLint infoLen = 0;

        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);

        if (infoLen > 1) {
            char* infoLog = (char*) malloc (sizeof(char) * infoLen);

            glGetShaderInfoLog(shader, infoLen, NULL, infoLog);
            std::cout << "Error compiling shader: " << infoLog << std::endl;

            free(infoLog);
        }

        glDeleteShader(shader);
        return 0;
    }

    return shader;
}

GLuint Init(Context *contxt)
{
    GLuint vertexShader;
    GLuint fragmentShader;
    GLuint programObject;
    GLint linked;

    vertexShader = LoadShader(GL_VERTEX_SHADER, "../src/7.opengles.vs");
    fragmentShader = LoadShader(GL_FRAGMENT_SHADER, "../src/7.opengles.fs");

    programObject = glCreateProgram();

    if (programObject == 0)
        return 0;

    glAttachShader(programObject, vertexShader);
    glAttachShader(programObject, fragmentShader);

    // Bind vPosition to attribute 0
    glBindAttribLocation(programObject, 0, "vPosition" );

    // Link the program
    glLinkProgram(programObject);

    // Check the link status
    glGetProgramiv(programObject, GL_LINK_STATUS, &linked);

    if (!linked) {
        GLint infoLen = 0;

        glGetProgramiv(programObject, GL_INFO_LOG_LENGTH, &infoLen);

        if (infoLen > 1) {
            char* infoLog = (char *) malloc(sizeof(char) * infoLen);

            glGetProgramInfoLog(programObject, infoLen, NULL, infoLog);
            std::cout << "Error linking program: " << infoLog << std::endl;

            free (infoLog);
        }

        glDeleteProgram(programObject);
        return GL_FALSE;
    }

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

    contxt->programObject = programObject;

    return programObject;
}

void Draw(Context *contxt)
{
    GLfloat vVertices[] = { 0.0f,  0.5f, 0.0f,
                           -0.5f, -0.5f, 0.0f,
                            0.5f, -0.5f, 0.0f};

    glViewport(0, 0, contxt->width, contxt->height);

    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(contxt->programObject);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, vVertices);
    glEnableVertexAttribArray(0);

    glDrawArrays(GL_TRIANGLES, 0, 3);
}

EGLBoolean WinCreate(Context *contxt, const char *title)
{
    Display *x_display;
    Window root;
    XSetWindowAttributes swa;
    XSetWindowAttributes  xattr;
    Atom wm_state;
    XWMHints hints;
    XEvent xev;
    Window win;

    x_display = XOpenDisplay(NULL);
    if (x_display == NULL)
        return EGL_FALSE;

    root = DefaultRootWindow(x_display);

    swa.event_mask  =  ExposureMask | PointerMotionMask | KeyPressMask;
    win = XCreateWindow(x_display, root,
               0, 0, contxt->width, contxt->height, 0,
               CopyFromParent, InputOutput,
               CopyFromParent, CWEventMask,
               &swa);

    xattr.override_redirect = false;
    XChangeWindowAttributes(x_display, win, CWOverrideRedirect, &xattr);

    hints.input = true;
    hints.flags = InputHint;
    XSetWMHints(x_display, win, &hints);

    // make the window visible on the screen
    XMapWindow(x_display, win);
    XStoreName(x_display, win, title);

    // get identifiers for the provided atom name strings
    wm_state = XInternAtom(x_display, "_NET_WM_STATE", false);

    memset(&xev, 0, sizeof(xev));
    xev.type                 = ClientMessage;
    xev.xclient.window       = win;
    xev.xclient.message_type = wm_state;
    xev.xclient.format       = 32;
    xev.xclient.data.l[0]    = 1;
    xev.xclient.data.l[1]    = false;
    XSendEvent(x_display,
       DefaultRootWindow (x_display),
       false,
       SubstructureNotifyMask,
       &xev);

    contxt->hWnd = (EGLNativeWindowType) win;
    contxt->x_display = x_display;
    return EGL_TRUE;
}

EGLBoolean CreateEGLContext(EGLNativeWindowType hWnd, EGLDisplay* eglDisplay,
                            EGLContext* eglContext, EGLSurface* eglSurface,
                            EGLint attribList[], Context *contxt)
{
    EGLint numConfigs;
    EGLint majorVersion;
    EGLint minorVersion;
    EGLDisplay display;
    EGLContext context;
    EGLSurface surface;
    EGLConfig config;
    EGLint contextAttribs[] = {EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE, EGL_NONE};

    display = eglGetDisplay((EGLNativeDisplayType)contxt->x_display);
    if (display == EGL_NO_DISPLAY)
        return EGL_FALSE;

    if (!eglInitialize(display, &majorVersion, &minorVersion))
        return EGL_FALSE;

    if (!eglGetConfigs(display, NULL, 0, &numConfigs))
        return EGL_FALSE;

    if (!eglChooseConfig(display, attribList, &config, 1, &numConfigs))
        return EGL_FALSE;

    surface = eglCreateWindowSurface(display, config, (EGLNativeWindowType)hWnd, NULL);
    if (surface == EGL_NO_SURFACE)
        return EGL_FALSE;

    context = eglCreateContext(display, config, EGL_NO_CONTEXT, contextAttribs);
    if (context == EGL_NO_CONTEXT )
        return EGL_FALSE;

    if (!eglMakeCurrent(display, surface, surface, context))
        return EGL_FALSE;

    *eglDisplay = display;
    *eglSurface = surface;
    *eglContext = context;

    return EGL_TRUE;
}

GLboolean esCreateWindow(Context *contxt, const char* title, GLuint flags)
{
    EGLint attribList[] =
    {
        EGL_RED_SIZE,       5,
        EGL_GREEN_SIZE,     6,
        EGL_BLUE_SIZE,      5,
        EGL_ALPHA_SIZE,     (flags & ES_WINDOW_ALPHA) ? 8 : EGL_DONT_CARE,
        EGL_DEPTH_SIZE,     (flags & ES_WINDOW_DEPTH) ? 8 : EGL_DONT_CARE,
        EGL_STENCIL_SIZE,   (flags & ES_WINDOW_STENCIL) ? 8 : EGL_DONT_CARE,
        EGL_SAMPLE_BUFFERS, (flags & ES_WINDOW_MULTISAMPLE) ? 1 : 0,
        EGL_NONE
    };

    if (contxt == NULL)
        return GL_FALSE;

    if (!WinCreate (contxt, title))
        return GL_FALSE;


    if (!CreateEGLContext(contxt->hWnd,
                          &contxt->eglDisplay,
                          &contxt->eglContext,
                          &contxt->eglSurface,
                          attribList,
                          contxt))
        return GL_FALSE;

    return GL_TRUE;
}

GLboolean userInterrupt(Context *contxt)
{
    XEvent xev;
    KeySym key;
    GLboolean userinterrupt = GL_FALSE;
    char text;

    while (XPending(contxt->x_display)) {
        XNextEvent(contxt->x_display, &xev);
        if (xev.type == KeyPress) {
            if (XLookupString(&xev.xkey, &text, 1, &key, 0) == 1) {
            }
        }

        if (xev.type == DestroyNotify)
            userinterrupt = GL_TRUE;
    }

    return userinterrupt;
}

void esMainLoop(Context *contxt)
{
    while (userInterrupt(contxt) == GL_FALSE)
    {
        Draw(contxt);

        eglSwapBuffers(contxt->eglDisplay, contxt->eglSurface);
    }
}


int main(int argc, char *argv[])
{
   Context contxt;

   esCreateWindow (&contxt, "GLES", ES_WINDOW_RGB);
   if (!Init (&contxt))
      return 0;

   esMainLoop (&contxt);
}
