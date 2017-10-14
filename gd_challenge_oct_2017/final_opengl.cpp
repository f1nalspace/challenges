#include "final_opengl.h"

#include "final_utils.h"

namespace finalspace {
	namespace renderer {
		void *OpenGLRenderer::AllocateTexture(const u32 width, const u32 height, void * data)
		{
			GLuint handle;
			glGenTextures(1, &handle);
			glBindTexture(GL_TEXTURE_2D, handle);
			glTexImage2D(GL_TEXTURE_2D, 0,
						 GL_RGBA8,
						 width, height, 0,
						 GL_RGBA, GL_UNSIGNED_BYTE, data);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

			glBindTexture(GL_TEXTURE_2D, 0);

			assert(sizeof(handle) <= sizeof(void *));
			void *result = utils::ValueToPointer(handle);
			return(result);
		}

		void OpenGLRenderer::BeginFrame(const f32 halfGameWidth, const f32 halfGameHeight, const f32 aspectRatio)
		{
			// Calculate a letterboxed viewport offset and size
			f32 viewportScale = (f32)windowSize.w / (halfGameWidth * 2.0f);
			Vec2i viewportSize = Vec2i(windowSize.w, (u32)(windowSize.w / aspectRatio));
			if (viewportSize.h > windowSize.h) {
				viewportSize.h = windowSize.h;
				viewportSize.w = (u32)(viewportSize.h * aspectRatio);
				viewportScale = (f32)viewportSize.w / (halfGameWidth * 2.0f);
			}
			Vec2i viewportOffset = Vec2i((windowSize.w - viewportSize.w) / 2, (windowSize.h - viewportSize.h) / 2);

			glViewport(viewportOffset.x, viewportOffset.y, viewportSize.w, viewportSize.h);

			Mat4f proj = Mat4f::CreateOrthoRH(-halfGameWidth, halfGameWidth, -halfGameHeight, halfGameHeight, 0.0f, 1.0f);
			Mat4f model = Mat4f::Identity;
			viewProjection = proj * model;
			glMatrixMode(GL_MODELVIEW);
			glLoadMatrixf(&viewProjection.m[0]);

			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		}

		void OpenGLRenderer::EndFrame()
		{
		}
	}
}