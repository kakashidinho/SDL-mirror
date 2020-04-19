/*
  Simple DirectMedia Layer
  Copyright (C) 1997-2020 Sam Lantinga <slouken@libsdl.org>

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
*/
#include "../../SDL_internal.h"

#if SDL_VIDEO_DRIVER_UIKIT && (SDL_VIDEO_OPENGL_ES || SDL_VIDEO_OPENGL_ES2)

#import <MetalANGLE/GLES2/gl2.h>
#import <MetalANGLE/GLES2/gl2ext.h>
#import <MetalANGLE/GLES3/gl3.h>
#import <MetalANGLE/MGLKit.h>

#import "SDL_uikitopenglview.h"
#include "SDL_uikitwindow.h"

@implementation SDL_uikitopenglview {
    /* The renderbuffer and framebuffer used to render to this layer. */
    GLuint viewRenderbuffer, viewFramebuffer;

    /* The depth buffer that is attached to viewFramebuffer, if it exists. */
    // GLuint depthRenderbuffer;

    GLenum colorBufferFormat;

    /* format of depthRenderbuffer */
    GLenum depthBufferFormat;

    /* The framebuffer and renderbuffer used for rendering with MSAA. */
    // GLuint msaaFramebuffer, msaaRenderbuffer;

    /* The number of MSAA samples. */
    int samples;

    BOOL retainedBacking;
}

@synthesize context;
@synthesize backingWidth;
@synthesize backingHeight;

+ (Class)layerClass
{
    return [MGLLayer class];
}

- (instancetype)initWithFrame:(CGRect)frame
                        scale:(CGFloat)scale
                retainBacking:(BOOL)retained
                        rBits:(int)rBits
                        gBits:(int)gBits
                        bBits:(int)bBits
                        aBits:(int)aBits
                    depthBits:(int)depthBits
                  stencilBits:(int)stencilBits
                         sRGB:(BOOL)sRGB
                 multisamples:(int)multisamples
                      context:(MGLContext *)glcontext
{
    if ((self = [super initWithFrame:frame])) {
        const BOOL useStencilBuffer = (stencilBits != 0);
        const BOOL useDepthBuffer = (depthBits != 0);
        int colorFormat;

        context = glcontext;
        samples = multisamples;
        retainedBacking = retained;
        
        MGLLayer *eaglLayer = (MGLLayer *)self.layer;

        if (samples > 0) {
            GLint maxsamples = 0;
            glGetIntegerv(GL_MAX_SAMPLES, &maxsamples);
            /* Clamp the samples to the max supported count. */
            samples = MIN(samples, maxsamples);
        }

        if (sRGB) {
            colorFormat = MGLDrawableColorFormatRGBA8888;
            colorBufferFormat = GL_SRGB8_ALPHA8;
        } else if (rBits >= 8 || gBits >= 8 || bBits >= 8 || aBits > 0) {
            /* if user specifically requests rbg888 or some color format higher than 16bpp */
            colorFormat = MGLDrawableColorFormatRGBA8888;
            colorBufferFormat = GL_RGBA8;
        } else {
            /* default case (potentially faster) */
            colorFormat = MGLDrawableColorFormatRGB565;
            colorBufferFormat = GL_RGB565;
        }

        eaglLayer.opaque = YES;
        
        eaglLayer.drawableColorFormat = colorFormat;
        eaglLayer.retainedBacking = retained;

        /* Set the appropriate scale (for retina display support) */
        self.contentScaleFactor = scale;
        
        if (!context || ![MGLContext setCurrentContext:context]) {
            SDL_SetError("Could not create OpenGL ES drawable (could not make context current)");
            return nil;
        }
        
        viewFramebuffer = eaglLayer.defaultOpenGLFrameBufferID;

        /* Create the color Renderbuffer Object */
        //glGenRenderbuffers(1, &viewRenderbuffer);
        //glBindRenderbuffer(GL_RENDERBUFFER, viewRenderbuffer);

      //  if (![context renderbufferStorage:GL_RENDERBUFFER fromDrawable:eaglLayer]) {
      //      SDL_SetError("Failed to create OpenGL ES drawable");
      //      return nil;
      //  }

        /* Create the Framebuffer Object */
        // glGenFramebuffers(1, &viewFramebuffer);
        // glBindFramebuffer(GL_FRAMEBUFFER, viewFramebuffer);

        /* attach the color renderbuffer to the FBO */
        //glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, //GL_RENDERBUFFER, viewRenderbuffer);

        /* glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_WIDTH, &backingWidth);
        glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_HEIGHT, &backingHeight);*/
        
        backingWidth = eaglLayer.drawableSize.width;
        backingHeight = eaglLayer.drawableSize.height;

        /*if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            SDL_SetError("Failed creating OpenGL ES framebuffer1");
            return nil;
        }*/

        /* When MSAA is used we'll use a separate framebuffer for rendering to,
         * since we'll need to do an explicit MSAA resolve before presenting. */
        if (samples > 0) {
            /*
            glGenFramebuffers(1, &msaaFramebuffer);
            glBindFramebuffer(GL_FRAMEBUFFER, msaaFramebuffer);

            glGenRenderbuffers(1, &msaaRenderbuffer);
            glBindRenderbuffer(GL_RENDERBUFFER, msaaRenderbuffer);*/

            /*
            glRenderbufferStorageMultisample(GL_RENDERBUFFER, samples, colorBufferFormat, backingWidth, backingHeight);
             */

            /*glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, msaaRenderbuffer);*/
            eaglLayer.drawableMultisample = samples;
        }

        if (useDepthBuffer || useStencilBuffer) {
            
            
            //if (useStencilBuffer) {
                /* Apparently you need to pack stencil and depth into one buffer. */
            //    depthBufferFormat = GL_DEPTH24_STENCIL8_OES;
            //} else if (useDepthBuffer) {
                /* iOS only uses 32-bit float (exposed as fixed point 24-bit)
                 * depth buffers. */
            //    depthBufferFormat = GL_DEPTH_COMPONENT24_OES;
            //}

            // glGenRenderbuffers(1, &depthRenderbuffer);
            //glBindRenderbuffer(GL_RENDERBUFFER, depthRenderbuffer);

            /*
            if (samples > 0) {
                glRenderbufferStorageMultisample(GL_RENDERBUFFER, samples, depthBufferFormat, backingWidth, backingHeight);
            } else {
                glRenderbufferStorage(GL_RENDERBUFFER, depthBufferFormat, backingWidth, backingHeight);
            }*/
            
            

            if (useDepthBuffer) {
                /*glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthRenderbuffer);*/
                eaglLayer.drawableDepthFormat = MGLDrawableDepthFormat24;
            }
            if (useStencilBuffer) {
                /*glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, depthRenderbuffer);*/
                eaglLayer.drawableStencilFormat = MGLDrawableStencilFormat8;
            }
        }

        /*if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            SDL_SetError("Failed creating OpenGL ES framebuffer2");
            return nil;
        }*/

        //glBindRenderbuffer(GL_RENDERBUFFER, viewRenderbuffer);

        [self setDebugLabels];
    }

    return self;
}

- (GLuint)drawableRenderbuffer
{
    return viewRenderbuffer;
}

- (GLuint)drawableFramebuffer
{
    /* When MSAA is used, the MSAA draw framebuffer is used for drawing. */
    /*
    if (msaaFramebuffer) {
        return msaaFramebuffer;
    } else {
        
    }*/
    return viewFramebuffer;
}

- (GLuint)msaaResolveFramebuffer
{
    /* When MSAA is used, the MSAA draw framebuffer is used for drawing and the
     * view framebuffer is used as a MSAA resolve framebuffer. */
    /*if (msaaFramebuffer) {
        return viewFramebuffer;
    } else {
        return 0;
    }*/
    return viewFramebuffer;
}

- (void)updateFrame
{
    // GLint prevRenderbuffer = 0;
    // glGetIntegerv(GL_RENDERBUFFER_BINDING, &prevRenderbuffer);

    //glBindRenderbuffer(GL_RENDERBUFFER, viewRenderbuffer);
    // [context renderbufferStorage:GL_RENDERBUFFER fromDrawable:(CAEAGLLayer *)self.layer];
/*
    glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_WIDTH, &backingWidth);
    glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_HEIGHT, &backingHeight);
*/
    
    /*if (msaaRenderbuffer != 0) {
        glBindRenderbuffer(GL_RENDERBUFFER, msaaRenderbuffer);
        glRenderbufferStorageMultisample(GL_RENDERBUFFER, samples, colorBufferFormat, backingWidth, backingHeight);
    }*/

    /*if (depthRenderbuffer != 0) {
        glBindRenderbuffer(GL_RENDERBUFFER, depthRenderbuffer);

        if (samples > 0) {
            glRenderbufferStorageMultisample(GL_RENDERBUFFER, samples, depthBufferFormat, backingWidth, backingHeight);
        } else {
            glRenderbufferStorage(GL_RENDERBUFFER, depthBufferFormat, backingWidth, backingHeight);
        }
    }*/

    // glBindRenderbuffer(GL_RENDERBUFFER, prevRenderbuffer);
}

- (void)setDebugLabels
{
    /*
    if (viewFramebuffer != 0) {
        glLabelObjectEXT(GL_FRAMEBUFFER, viewFramebuffer, 0, "context FBO");
    }

    if (viewRenderbuffer != 0) {
        glLabelObjectEXT(GL_RENDERBUFFER, viewRenderbuffer, 0, "context color buffer");
    }

    if (depthRenderbuffer != 0) {
        if (depthBufferFormat == GL_DEPTH24_STENCIL8_OES) {
            glLabelObjectEXT(GL_RENDERBUFFER, depthRenderbuffer, 0, "context depth-stencil buffer");
        } else {
            glLabelObjectEXT(GL_RENDERBUFFER, depthRenderbuffer, 0, "context depth buffer");
        }
    }

    if (msaaFramebuffer != 0) {
        glLabelObjectEXT(GL_FRAMEBUFFER, msaaFramebuffer, 0, "context MSAA FBO");
    }

    if (msaaRenderbuffer != 0) {
        glLabelObjectEXT(GL_RENDERBUFFER, msaaRenderbuffer, 0, "context MSAA renderbuffer");
    }
     */
}

- (void)swapBuffers
{
    
    MGLLayer *eaglLayer = (MGLLayer *)self.layer;
    [MGLContext setCurrentContext:context forLayer:eaglLayer];
    glBindFramebuffer(GL_FRAMEBUFFER, eaglLayer.defaultOpenGLFrameBufferID);
    [context present:eaglLayer];
}

- (void)layoutSubviews
{
    /*
    MGLLayer *eaglLayer = (MGLLayer *)self.layer;
    [MGLContext setCurrentContext:context forLayer:eaglLayer];
    glBindFramebuffer(GL_FRAMEBUFFER, eaglLayer.defaultOpenGLFrameBufferID);
    [context present:eaglLayer];*/
    MGLLayer *eaglLayer = (MGLLayer *)self.layer;
    backingWidth = eaglLayer.drawableSize.width;
    backingHeight = eaglLayer.drawableSize.height;
}

- (void)destroyFramebuffer
{
    /*
    if (viewFramebuffer != 0) {
        glDeleteFramebuffers(1, &viewFramebuffer);
        viewFramebuffer = 0;
    }*/

    if (viewRenderbuffer != 0) {
        glDeleteRenderbuffers(1, &viewRenderbuffer);
        viewRenderbuffer = 0;
    }
    /*
    if (depthRenderbuffer != 0) {
        glDeleteRenderbuffers(1, &depthRenderbuffer);
        depthRenderbuffer = 0;
    }

    if (msaaFramebuffer != 0) {
        glDeleteFramebuffers(1, &msaaFramebuffer);
        msaaFramebuffer = 0;
    }

    if (msaaRenderbuffer != 0) {
        glDeleteRenderbuffers(1, &msaaRenderbuffer);
        msaaRenderbuffer = 0;
    }*/
}

- (void)dealloc
{
    if (context && context == [MGLContext currentContext]) {
        [self destroyFramebuffer];
        [MGLContext setCurrentContext:nil];
    }
}

@end

#endif /* SDL_VIDEO_DRIVER_UIKIT */

/* vi: set ts=4 sw=4 expandtab: */
