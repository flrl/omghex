#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <iostream>

#define GL_GLEXT_PROTOTYPES

#include "SDL.h"

//#include <OpenGL/gl3.h>
#include "SDL_opengl.h"

#include "utils.h"

SDL_Window *g_window;
SDL_GLContext g_glContext;

bool g_quit;

void setup(int w, int h) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        fprintf(stderr, "Unable to init SDL: %s\n", SDL_GetError());
        exit(1);
    }
    atexit(SDL_Quit);

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BUFFER_SIZE, 32 );

    g_window = SDL_CreateWindow(
       ":O",
       SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
       w, h,
       SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL
   );

    if (not g_window) {
        fprintf(stderr, "Unable to create window: %s\n", SDL_GetError());
        exit(1);
    }

    SDL_GLContext g_glContext = SDL_GL_CreateContext(g_window);
    if (not g_glContext) {
        fprintf(stderr, "Unable to create opengl context: %s\n", SDL_GetError());
        exit(1);
    }

    glViewport(0, 0, w, h);
}

void foo() {
    char *shader_src;
    size_t shader_len;

    if ((shader_src = (char *) file_contents("shader-src/blah.vert", &shader_len))) {
        std::cout << "shader source (" << shader_len << "):" << std::endl << shader_src << std::endl;
        free(shader_src);
    }
}

#define gl_assert_ok()  do {                                \
    GLenum error = glGetError();                            \
    if (error) {                                            \
        do {                                                \
            std::cout << "gl error in " << __FILE__ <<      \
                " at line " << __LINE__ <<                  \
                ": " << error << std::endl;                 \
        } while ((error = glGetError()));                   \
        exit(1);                                            \
    }                                                       \
} while(0)                                                  \

int main (int argc, char **argv) {
    setup(480, 480);
    foo();

    glClearColor(0, 0, 0, 0);

    g_quit = false;

    const float sqrt_3 = 1.732051f;
    std::cout << "sqrt(3): " << sqrt_3 << std::endl;
    printf("sqrt(3): %f\n", sqrt_3);
    printf("sqrt(3): %f\n", sqrtf(3));

    float r = 1.0f;
    float sqrt_3_2_r = sqrt_3 * r / 2.0f;
    float r_2 = r / 2.0f;

    static const GLfloat vertices[] = {
        0.0f, 0.0f,
        sqrt_3_2_r, -r_2,
        sqrt_3_2_r, r_2,
        0.0f, r,
        -sqrt_3_2_r, r_2,
        -sqrt_3_2_r, -r_2,
        0.0f, -r,
    };

    static const GLuint elements[] = {
        0, 1, 2,
        0, 2, 3,
        0, 3, 4,
        0, 4, 5,
        0, 5, 6,
        0, 6, 1,
    };

    GLuint vbuf, ebuf, vao;

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glGenBuffers(1, &vbuf);
    glBindBuffer(GL_ARRAY_BUFFER, vbuf);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glGenBuffers(1, &ebuf);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebuf);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(elements), elements, GL_STATIC_DRAW);

    size_t tmp_size;

    GLchar *vsrc;
    GLint vlen;
    GLuint vshader;
    GLint shader_ok;

    vsrc = (GLchar *) file_contents("shader-src/blah.vert", &tmp_size);
    vlen = tmp_size;
    vshader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vshader, 1, (const GLchar **) &vsrc, &vlen);
    free(vsrc);
    glCompileShader(vshader);
    glGetShaderiv(vshader, GL_COMPILE_STATUS, &shader_ok);
    if (not shader_ok) {
        fprintf(stderr, "failed to compile vertex shader\n");
//        show_info_log(vshader, glGetShaderiv, glGetShaderInfoLog);

        GLint log_length;
        char *log;
        glGetShaderiv(vshader, GL_INFO_LOG_LENGTH, &log_length);
        log = (char *) malloc(log_length);
        glGetShaderInfoLog(vshader, log_length, NULL, log);
        fprintf(stderr, "%s", log);
        free(log);

        glDeleteShader(vshader);
        exit(1);
    }

    GLchar *fsrc;
    GLint flen;
    GLuint fshader;
    fsrc = (GLchar *) file_contents("shader-src/blah.frag", &tmp_size);
    flen = tmp_size;
    fshader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fshader, 1, (const GLchar **) &fsrc, &flen);
    free(fsrc);
    glCompileShader(fshader);
    glGetShaderiv(fshader, GL_COMPILE_STATUS, &shader_ok);
    if (not shader_ok) {
        fprintf(stderr, "failed to compile fragment shader\n");

        GLint log_length;
        char *log;
        glGetShaderiv(fshader, GL_INFO_LOG_LENGTH, &log_length);
        log = (char *) malloc(log_length);
        glGetShaderInfoLog(fshader, log_length, NULL, log);
        fprintf(stderr, "%s", log);
        free(log);

        glDeleteShader(fshader);
        exit(1);
    }

    GLint program_ok;
    GLuint program = glCreateProgram();
    glAttachShader(program, vshader);
    glAttachShader(program, fshader);
    glBindAttribLocation(program, 0, "position");
    glLinkProgram(program);

//    glDeleteShader(vshader);
//    glDeleteShader(fshader);

    glGetProgramiv(program, GL_LINK_STATUS, &program_ok);
    if (not program_ok) {
        fprintf(stderr, "failed to link\n");

        GLint log_length;
        char *log;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &log_length);
        log = (char *) malloc(log_length);
        glGetProgramInfoLog(program, log_length, NULL, log);
        fprintf(stderr, "%s", log);
        free(log);

        glDeleteProgram(program);
        exit(1);
    }

    glUseProgram(program);

    while(not g_quit) {
        SDL_Event e;

        while (SDL_PollEvent(&e)) {
            switch (e.type) {
                case SDL_QUIT:
                case SDL_KEYDOWN:
                case SDL_MOUSEBUTTONDOWN:
                    g_quit = true;
                    break;

                default:
                    // do nothing for now
                    break;
            }
        }

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glBindBuffer(GL_ARRAY_BUFFER, vbuf);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (void *) 0);
        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebuf);
        glDrawElements(GL_TRIANGLES, sizeof(elements) / sizeof(elements[0]), GL_UNSIGNED_INT, (void *) 0);

        gl_assert_ok();
        
        SDL_GL_SwapWindow(g_window);
    }

    std::cout << "received event, exiting..." << std::endl;

















    exit(0);
}
