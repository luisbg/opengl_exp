#include <GLES2/gl2.h>
#include <EGL/egl.h>

#include <stdlib.h>
#include <iostream>
#include <string.h>
#include <fstream>
#include <sstream>

#include <SDL.h>
#include <SDL_image.h>

#include <shader_gles.h>
#include <matrix_gles.h>

#define ES_WINDOW_RGB           0
#define ES_WINDOW_ALPHA         1
#define ES_WINDOW_DEPTH         2
#define ES_WINDOW_STENCIL       4
#define ES_WINDOW_MULTISAMPLE   8

#define VERTEX_POS_SIZE         3
#define VERTEX_NORMAL_SIZE      3
#define VERTEX_TEXCOORD0_SIZE   2

#define VERTEX_POS_INDX         0
#define VERTEX_NORMAL_INDX      1
#define VERTEX_TEXCOORD0_INDX   2

typedef struct _context
{
    GLuint programObject;

    GLint positionLoc;
    GLint texCoordLoc;

    GLint samplerLoc;
    GLuint textureId;

    GLint mvpLoc;
    Matrix mvpMatrix;

    GLfloat xOffset;
    GLfloat yOffset;

    GLfloat *vertices;
    GLuint *indices;
    GLfloat position[2] = {0.0f, 0.0f};
    int numIndices;

    GLint width = 1280;
    GLint height = 720;
    Display *x_display;

    EGLNativeWindowType  hWnd;
    EGLDisplay  eglDisplay;
    EGLContext  eglContext;
    EGLSurface  eglSurface;

} Context;


GLuint createTexture(const char *img_file)
{
    GLuint textureId;

    glGenTextures(1, &textureId);
    glBindTexture(GL_TEXTURE_2D, textureId);

    SDL_Surface* img_surface = IMG_Load(img_file);
    if (img_surface)
    {
        GLenum texture_format;

        if ((img_surface->w & (img_surface->w - 1)) != 0)
            std::cout << "Image width is not a power of 2" << std::endl;

        if ((img_surface->h & (img_surface->h - 1)) != 0)
            std::cout << "Image height is not a power of 2" << std::endl;

        GLint nOfColors = img_surface->format->BytesPerPixel;
        if (nOfColors == 4)     // contains an alpha channel
            texture_format = GL_RGBA;
        else if (nOfColors == 3)     // no alpha channel
            texture_format = GL_RGB;
        else
            std::cout << "the image is not truecolor.." << std::endl;

        std::cout << "Loaded sky image with size: " << img_surface->w << "," << img_surface->h << std::endl;

        glTexImage2D(GL_TEXTURE_2D, 0, texture_format, img_surface->h, img_surface->w, 0,
                     texture_format, GL_UNSIGNED_BYTE, img_surface->pixels);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cout << "Failed to load texture" << std::endl;
    }

    glBindTexture(GL_TEXTURE_2D, 0);

   return textureId;
}

int generateRect(float scale, GLfloat **vertices, GLuint **indices)
{
    int i;
    int numVertices = 4;
    int numIndices = 6;

    GLfloat quadVerts[] =
    {  // pos               // tex
       -0.5f,  0.5f,  0.0f, 0.0f, 0.0f,
        0.5f,  0.5f,  0.0f, 1.0f, 0.0f,
        0.5f, -0.5f,  0.0f, 1.0f, 1.0f,
       -0.5f,  -0.5f, 0.0f, 0.0f, 1.0f
    };

    if (vertices != NULL)
    {
       *vertices = (GLfloat *) malloc (sizeof(GLfloat) * 5 * numVertices);
       memcpy(*vertices, quadVerts, sizeof(quadVerts));
       for (i = 0; i < numVertices; i++)
       {
          int pos_x = i * 5;
          (*vertices)[pos_x] *= scale;
          (*vertices)[pos_x + 1] *= scale;  // y
          (*vertices)[pos_x + 2] *= scale;  // z
       }
    }

    if (indices != NULL)
    {
       GLuint cubeIndices[] =
       {
          0, 1, 3,
          1, 2, 3,
       };

       *indices = (GLuint *) malloc(sizeof(GLuint) * numIndices);
       memcpy(*indices, cubeIndices, sizeof(cubeIndices));
    }

    return numIndices;
}

void moveRect(Context *contxt, float x, float y)
{
    contxt->position[0] = x;
    contxt->position[1] = y;
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

void updateRect(Context* contxt)
{
    Matrix perspective;
    Matrix modelview;
    float aspect;

    aspect = (GLfloat) contxt->width / (GLfloat) contxt->height;

    MatrixLoadIdentity(&perspective);
    Perspective(&perspective, 60.0f, aspect, 1.0f, 20.0f);
    MatrixLoadIdentity(&modelview);
    Translate(&modelview, 0.0, 0.0, -2.0);
    MatrixMultiply(&contxt->mvpMatrix, &modelview, &perspective);
}

int main(int argc, char *argv[])
{
    Context contxt;

    esCreateWindow (&contxt, "GLES", ES_WINDOW_RGB);

    Shader ourShader("../src/11.carousel_gles.vs", "../src/11.carousel_gles.fs");
    contxt.programObject = ourShader.get_id();

    contxt.positionLoc = glGetAttribLocation(contxt.programObject, "v_position");
    contxt.texCoordLoc = glGetAttribLocation(contxt.programObject, "a_texCoord");

    contxt.samplerLoc = glGetUniformLocation(contxt.programObject, "s_texture");

    contxt.numIndices = generateRect(0.6, &contxt.vertices, &contxt.indices);
    contxt.mvpLoc = glGetUniformLocation(contxt.programObject, "u_mvpMatrix");

    contxt.xOffset = glGetUniformLocation(contxt.programObject, "xOffset");
    contxt.yOffset = glGetUniformLocation(contxt.programObject, "yOffset");

    contxt.textureId = createTexture("../img/sky.jpg");
    moveRect(&contxt, -1.5f, 0.66f);  // Window spans [-2:2],[-1:1] due to Perspective()

    glViewport(0, 0, contxt.width, contxt.height);

    while (userInterrupt(&contxt) == GL_FALSE)
    {
        glClear ( GL_COLOR_BUFFER_BIT );

        updateRect(&contxt);

        ourShader.use();

        glVertexAttribPointer(contxt.positionLoc, 3, GL_FLOAT,
                              GL_FALSE, 5 * sizeof(GLfloat), contxt.vertices);
        glVertexAttribPointer(contxt.texCoordLoc, 2, GL_FLOAT,
                              GL_FALSE, 5 * sizeof(GLfloat), &contxt.vertices[3]);

        glEnableVertexAttribArray (contxt.positionLoc);
        glEnableVertexAttribArray (contxt.texCoordLoc);

        glUniformMatrix4fv(contxt.mvpLoc, 1, GL_FALSE, (GLfloat*) &contxt.mvpMatrix.m[0][0]);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, contxt.textureId);

        glUniform1f(contxt.xOffset, contxt.position[0]);
        glUniform1f(contxt.yOffset, contxt.position[1]);

        glDrawElements(GL_TRIANGLES, contxt.numIndices, GL_UNSIGNED_INT, contxt.indices);

        eglSwapBuffers(contxt.eglDisplay, contxt.eglSurface);
    }

    return 0;
}
