/*
  Copyright (C) 1997-2020 Sam Lantinga <slouken@libsdl.org>

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely.
*/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "SDL_test_common.h"

#if defined(__IPHONEOS__) || defined(__ANDROID__)
#define HAVE_OPENGLES
#endif

#ifdef HAVE_OPENGLES

#define USE_GLES1 0

#if USE_GLES1
#include "SDL_opengles.h"
#else
#include "SDL_opengles2.h"

#define VERTEX_POS_INDEX 0
#define VERTEX_COLOR_INDEX 2

#endif

static SDLTest_CommonState *state;
static SDL_GLContext *context = NULL;

static GLuint *shaderPrograms;

static int depth = 16;

/* Call this instead of exit(), so we can clean up SDL: atexit() is evil. */
static void
quit(int rc)
{
    int i;

    if (context != NULL) {
        for (i = 0; i < state->num_windows; i++) {
            if (context[i]) {
                SDL_GL_DeleteContext(context[i]);
            }
        }

        SDL_free(context);
    }

    if (shaderPrograms)
    {
        SDL_free(shaderPrograms);
    }

    SDLTest_CommonQuit(state);
    exit(rc);
}

static void RotateMatrix(float angle, float x, float y, float z, float *matrixOut)
{
    float length = sqrtf(x * x + y * y + z * z);
    float angle_radians  = angle * (3.14159265358979323f / 180.0f);
    float c              = cos(angle_radians);
    float ci             = 1.f - c;
    float s              = sin(angle_radians);

    x = x / length;
    y = y / length;
    z = z / length;

    float x2 = x * x;
    float y2 = y * y;
    float z2 = z * z;

    float xy = x * y;
    float yz = y * z;
    float zx = z * x;

    matrixOut[0] = c + ci * x2;
    matrixOut[1] = ci * xy + s * z;
    matrixOut[2] = ci * zx - s * y;
    matrixOut[3] = 0.f;

    matrixOut[4] = ci * xy - s * z;
    matrixOut[5] = c + ci * y2;
    matrixOut[6] = ci * yz + s * x;
    matrixOut[7] = 0.f;

    matrixOut[8]  = ci * zx + s * y;
    matrixOut[9]  = ci * yz - s * x;
    matrixOut[10] = c + ci * z2;
    matrixOut[11] = 0.f;

    matrixOut[12] = 0.f;
    matrixOut[13] = 0.f;
    matrixOut[14] = 0.f;
    matrixOut[15] = 1.f;
}

static void
Render(
#if !USE_GLES1
       GLuint shaderProgram
#endif
       )
{
    static GLubyte color[8][4] = { {255, 0, 0, 0},
    {255, 0, 0, 255},
    {0, 255, 0, 255},
    {0, 255, 0, 255},
    {0, 255, 0, 255},
    {255, 255, 255, 255},
    {255, 0, 255, 255},
    {0, 0, 255, 255}
    };
    static GLfloat cube[8][3] = { {0.5, 0.5, -0.5},
    {0.5f, -0.5f, -0.5f},
    {-0.5f, -0.5f, -0.5f},
    {-0.5f, 0.5f, -0.5f},
    {-0.5f, 0.5f, 0.5f},
    {0.5f, 0.5f, 0.5f},
    {0.5f, -0.5f, 0.5f},
    {-0.5f, -0.5f, 0.5f}
    };
    static GLubyte indices[36] = { 0, 3, 4,
        4, 5, 0,
        0, 5, 6,
        6, 1, 0,
        6, 7, 2,
        2, 1, 6,
        7, 4, 3,
        3, 2, 7,
        5, 4, 7,
        7, 6, 5,
        2, 3, 1,
        3, 0, 1
    };


    /* Do our drawing, too. */
    glClearColor(0.0, 0.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

#if USE_GLES1
    /* Draw the cube */
    glColorPointer(4, GL_UNSIGNED_BYTE, 0, color);
    glEnableClientState(GL_COLOR_ARRAY);
    glVertexPointer(3, GL_FLOAT, 0, cube);
    glEnableClientState(GL_VERTEX_ARRAY);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_BYTE, indices);

    glMatrixMode(GL_MODELVIEW);
    glRotatef(5.0, 1.0, 1.0, 1.0);
#else
    glVertexAttribPointer(VERTEX_COLOR_INDEX, 4, GL_UNSIGNED_BYTE, GL_TRUE, 0, color);
    glEnableVertexAttribArray(VERTEX_COLOR_INDEX);
    glVertexAttribPointer(VERTEX_POS_INDEX, 3, GL_FLOAT, GL_FALSE, 0, cube);
    glEnableVertexAttribArray(VERTEX_POS_INDEX);

    float rotationMatrix[16];
    static float angle = 0.0;
    RotateMatrix(angle, 1.0, 1.0, 1.0, rotationMatrix);

    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "uMatrix"), 1, GL_FALSE, rotationMatrix);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_BYTE, indices);

    angle += .05f;
    if (angle >= 360.f)
    {
        angle -= 360.f;
    }
#endif
}

int
main(int argc, char *argv[])
{
    int fsaa, accel;
    int value;
    int i, done;
    SDL_DisplayMode mode;
    SDL_Event event;
    Uint32 then, now, frames;
    int status;

    /* Enable standard application logging */
    SDL_LogSetPriority(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_INFO);

    /* Initialize parameters */
    fsaa = 0;
    accel = 0;

    /* Initialize test framework */
    state = SDLTest_CommonCreateState(argv, SDL_INIT_VIDEO);
    if (!state) {
        return 1;
    }
    for (i = 1; i < argc;) {
        int consumed;

        consumed = SDLTest_CommonArg(state, i);
        if (consumed == 0) {
            if (SDL_strcasecmp(argv[i], "--fsaa") == 0) {
                ++fsaa;
                consumed = 1;
            } else if (SDL_strcasecmp(argv[i], "--accel") == 0) {
                ++accel;
                consumed = 1;
            } else if (SDL_strcasecmp(argv[i], "--zdepth") == 0) {
                i++;
                if (!argv[i]) {
                    consumed = -1;
                } else {
                    depth = SDL_atoi(argv[i]);
                    consumed = 1;
                }
            } else {
                consumed = -1;
            }
        }
        if (consumed < 0) {
            static const char *options[] = { "[--fsaa]", "[--accel]", "[--zdepth %d]", NULL };
            SDLTest_CommonLogUsage(state, argv[0], options);
            quit(1);
        }
        i += consumed;
    }

    /* Set OpenGL parameters */
    state->window_flags |= SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_BORDERLESS;
    state->gl_red_size = 5;
    state->gl_green_size = 5;
    state->gl_blue_size = 5;
    state->gl_depth_size = depth;
#if USE_GLES1
    state->gl_major_version = 1;
    state->gl_minor_version = 1;
#else
    state->gl_major_version = 2;
    state->gl_minor_version = 0;
#endif
    state->gl_profile_mask = SDL_GL_CONTEXT_PROFILE_ES;
    if (fsaa) {
        state->gl_multisamplebuffers=1;
        state->gl_multisamplesamples=fsaa;
    }
    if (accel) {
        state->gl_accelerated=1;
    }
    if (!SDLTest_CommonInit(state)) {
        quit(2);
    }

    context = (SDL_GLContext *)SDL_calloc(state->num_windows, sizeof(context));
    if (context == NULL) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Out of memory!\n");
        quit(2);
    }

#if !USE_GLES2
    shaderPrograms = (GLuint *)SDL_calloc(state->num_windows, sizeof(GLuint));
    if (shaderPrograms == NULL) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Out of memory!\n");
        quit(2);
    }
#endif

    /* Create OpenGL ES contexts */
    for (i = 0; i < state->num_windows; i++) {
        context[i] = SDL_GL_CreateContext(state->windows[i]);
        if (!context[i]) {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "SDL_GL_CreateContext(): %s\n", SDL_GetError());
            quit(2);
        }
    }

    if (state->render_flags & SDL_RENDERER_PRESENTVSYNC) {
        SDL_GL_SetSwapInterval(1);
    } else {
        SDL_GL_SetSwapInterval(0);
    }

    SDL_GetCurrentDisplayMode(0, &mode);
    SDL_Log("Screen bpp: %d\n", SDL_BITSPERPIXEL(mode.format));
    SDL_Log("\n");
    SDL_Log("Vendor     : %s\n", glGetString(GL_VENDOR));
    SDL_Log("Renderer   : %s\n", glGetString(GL_RENDERER));
    SDL_Log("Version    : %s\n", glGetString(GL_VERSION));
    SDL_Log("Extensions : %s\n", glGetString(GL_EXTENSIONS));
    SDL_Log("\n");

    status = SDL_GL_GetAttribute(SDL_GL_RED_SIZE, &value);
    if (!status) {
        SDL_Log("SDL_GL_RED_SIZE: requested %d, got %d\n", 5, value);
    } else {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to get SDL_GL_RED_SIZE: %s\n",
                SDL_GetError());
    }
    status = SDL_GL_GetAttribute(SDL_GL_GREEN_SIZE, &value);
    if (!status) {
        SDL_Log("SDL_GL_GREEN_SIZE: requested %d, got %d\n", 5, value);
    } else {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to get SDL_GL_GREEN_SIZE: %s\n",
                SDL_GetError());
    }
    status = SDL_GL_GetAttribute(SDL_GL_BLUE_SIZE, &value);
    if (!status) {
        SDL_Log("SDL_GL_BLUE_SIZE: requested %d, got %d\n", 5, value);
    } else {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to get SDL_GL_BLUE_SIZE: %s\n",
                SDL_GetError());
    }
    status = SDL_GL_GetAttribute(SDL_GL_DEPTH_SIZE, &value);
    if (!status) {
        SDL_Log("SDL_GL_DEPTH_SIZE: requested %d, got %d\n", depth, value);
    } else {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to get SDL_GL_DEPTH_SIZE: %s\n",
                SDL_GetError());
    }
    if (fsaa) {
        status = SDL_GL_GetAttribute(SDL_GL_MULTISAMPLEBUFFERS, &value);
        if (!status) {
            SDL_Log("SDL_GL_MULTISAMPLEBUFFERS: requested 1, got %d\n", value);
        } else {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to get SDL_GL_MULTISAMPLEBUFFERS: %s\n",
                    SDL_GetError());
        }
        status = SDL_GL_GetAttribute(SDL_GL_MULTISAMPLESAMPLES, &value);
        if (!status) {
            SDL_Log("SDL_GL_MULTISAMPLESAMPLES: requested %d, got %d\n", fsaa,
                   value);
        } else {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to get SDL_GL_MULTISAMPLESAMPLES: %s\n",
                    SDL_GetError());
        }
    }
    if (accel) {
        status = SDL_GL_GetAttribute(SDL_GL_ACCELERATED_VISUAL, &value);
        if (!status) {
            SDL_Log("SDL_GL_ACCELERATED_VISUAL: requested 1, got %d\n", value);
        } else {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to get SDL_GL_ACCELERATED_VISUAL: %s\n",
                    SDL_GetError());
        }
    }

    /* Set rendering settings for each context */
    for (i = 0; i < state->num_windows; ++i) {
        float aspectAdjust;

        status = SDL_GL_MakeCurrent(state->windows[i], context[i]);
        SDL_GL_GetDrawableSize(state->windows[i], &state->window_w, &state->window_h);
        if (status) {
            SDL_Log("SDL_GL_MakeCurrent(): %s\n", SDL_GetError());

            /* Continue for next window */
            continue;
        }

        aspectAdjust = (4.0f / 3.0f) / ((float)state->window_w / state->window_h);
        glViewport(0, 0, state->window_w, state->window_h);

        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);

#if USE_GLES1
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glOrthof(-2.0, 2.0, -2.0 * aspectAdjust, 2.0 * aspectAdjust, -20.0, 20.0);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        glShadeModel(GL_SMOOTH);
#else
        // Create simple shader program
        const char kVS[] = "#version 100\n"
        "attribute vec3 aPosition;\n"
        "attribute vec4 aColor;\n"

        "uniform mat4 uMatrix;\n"

        "varying vec4 vColor;\n"
        "void main()\n"
        "{\n"
        "   gl_Position = vec4(aPosition, 1.0) * uMatrix;\n"
        "   vColor      = aColor;\n"
        "}\n"
        "";
        const char kFS[] = "#version 100\n"
        "varying lowp vec4 vColor;\n"
        "void main()\n"
        "{\n"
        "   gl_FragColor = vColor;\n"
        "}\n";

        GLuint vs, fs;
        const GLchar *sources[] = {kVS, kFS};
        const GLint lengths[] = {-1};
        vs = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vs, 1, &sources[0], lengths);
        glCompileShader(vs);

        fs = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fs, 1, &sources[1], lengths);
        glCompileShader(fs);

        shaderPrograms[i] = glCreateProgram();
        glAttachShader(shaderPrograms[i], vs);
        glAttachShader(shaderPrograms[i], fs);

        glBindAttribLocation(shaderPrograms[i], VERTEX_POS_INDEX, "aPosition");
        glBindAttribLocation(shaderPrograms[i], VERTEX_COLOR_INDEX, "aColor");

        glLinkProgram(shaderPrograms[i]);

        glDeleteShader(vs);
        glDeleteShader(fs);

        glUseProgram(shaderPrograms[i]);
#endif
    }

    /* Main render loop */
    frames = 0;
    then = SDL_GetTicks();
    done = 0;
    while (!done) {
        /* Check for events */
        ++frames;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
            case SDL_WINDOWEVENT:
                switch (event.window.event) {
                    case SDL_WINDOWEVENT_RESIZED:
                        for (i = 0; i < state->num_windows; ++i) {
                            if (event.window.windowID == SDL_GetWindowID(state->windows[i])) {
                                status = SDL_GL_MakeCurrent(state->windows[i], context[i]);
                                if (status) {
                                    SDL_Log("SDL_GL_MakeCurrent(): %s\n", SDL_GetError());
                                    break;
                                }
                                /* Change view port to the new window dimensions */
                                glViewport(0, 0, event.window.data1, event.window.data2);
                                /* Update window content */
#if USE_GLES1
                                Render();
#else
                                Render(shaderPrograms[i]);
#endif
                                SDL_GL_SwapWindow(state->windows[i]);
                                break;
                            }
                        }
                        break;
                }
            }
            SDLTest_CommonEvent(state, &event, &done);
        }
        for (i = 0; i < state->num_windows; ++i) {
            if (state->windows[i] == NULL)
                continue;
            status = SDL_GL_MakeCurrent(state->windows[i], context[i]);
            if (status) {
                SDL_Log("SDL_GL_MakeCurrent(): %s\n", SDL_GetError());

                /* Continue for next window */
                continue;
            }
#if USE_GLES1
            Render();
#else
            Render(shaderPrograms[i]);
#endif
            SDL_GL_SwapWindow(state->windows[i]);
        }
    }

    /* Print out some timing information */
    now = SDL_GetTicks();
    if (now > then) {
        SDL_Log("%2.2f frames per second\n",
               ((double) frames * 1000) / (now - then));
    }
#if !defined(__ANDROID__)
    quit(0);
#endif        
    return 0;
}

#else /* HAVE_OPENGLES */

int
main(int argc, char *argv[])
{
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "No OpenGL ES support on this system\n");
    return 1;
}

#endif /* HAVE_OPENGLES */

/* vi: set ts=4 sw=4 expandtab: */
