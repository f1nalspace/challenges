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

		void OpenGLRenderer::BeginFrame()
		{
			glViewport(viewport.offset.x, viewport.offset.y, viewport.size.w, viewport.size.h);

			glMatrixMode(GL_MODELVIEW);
			glLoadMatrixf(&viewProjection.m[0]);

			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		}

		void OpenGLRenderer::EndFrame()
		{
		}

		void OpenGLRenderer::Update(const f32 halfGameWidth, const f32 halfGameHeight, const f32 aspectRatio)
		{
			// Calculate a letterboxed viewport offset and size
			viewSize = Vec2f(halfGameWidth, halfGameHeight) * 2.0f;
			viewScale = (f32)windowSize.w / (halfGameWidth * 2.0f);
			Vec2i viewportSize = Vec2i(windowSize.w, (u32)(windowSize.w / aspectRatio));
			if (viewportSize.h > windowSize.h) {
				viewportSize.h = windowSize.h;
				viewportSize.w = (u32)(viewportSize.h * aspectRatio);
				viewScale = (f32)viewportSize.w / (halfGameWidth * 2.0f);
			}
			Vec2i viewportOffset = Vec2i((windowSize.w - viewportSize.w) / 2, (windowSize.h - viewportSize.h) / 2);
			viewport.size = viewportSize;
			viewport.offset = viewportOffset;

			// Update view projection
			Mat4f proj = Mat4f::CreateOrthoRH(-halfGameWidth, halfGameWidth, -halfGameHeight, halfGameHeight, 0.0f, 1.0f);
			Mat4f model = Mat4f::Identity;
			viewProjection = proj * model;
		}

		Vec2f OpenGLRenderer::Unproject(const Vec2i &windowPos) {
			Vec2f result = Vec2f();
			if (viewScale > 0) {
				result.x = (f32)((windowPos.x - viewport.offset.x) / viewScale) - viewSize.w * 0.5f;
				result.y = (f32)((windowPos.y - viewport.offset.y) / viewScale) - viewSize.h * 0.5f;
			}
			return(result);
		}
		void OpenGLRenderer::DrawRectangle(const Vec2f & pos, const Vec2f & ext, const Vec4f &color, const bool isFilled)
		{
			Mat4f translation = Mat4f::CreateTranslation(pos);
			Mat4f mvp = viewProjection * translation;
			glLoadMatrixf(&mvp.m[0]);

			glColor4fv(&color.elements[0]);

			glBegin(isFilled ? GL_QUADS : GL_LINE_LOOP);
			glVertex2f(ext.w, ext.h);
			glVertex2f(-ext.w, ext.h);
			glVertex2f(-ext.w, -ext.h);
			glVertex2f(ext.w, -ext.h);
			glEnd();
		}

		void OpenGLRenderer::DrawSprite(const Vec2f &pos, const Vec2f &ext, const Vec4f &color, const Texture &texture, const Vec2f &uvMin, const Vec2f &uvMax) {
			Mat4f translation = Mat4f::CreateTranslation(pos);
			Mat4f mvp = viewProjection * translation;
			glLoadMatrixf(&mvp.m[0]);

			GLuint texHandle = utils::PointerToValue<GLuint>(texture.handle);
			glEnable(GL_TEXTURE_2D);
			glBindTexture(GL_TEXTURE_2D, texHandle);

			glColor4fv(&color.elements[0]);
			glBegin(GL_QUADS);
			glTexCoord2f(uvMax.x, uvMax.y); glVertex2f(ext.w, ext.h);
			glTexCoord2f(uvMin.x, uvMax.y); glVertex2f(-ext.w, ext.h);
			glTexCoord2f(uvMin.x, uvMin.y); glVertex2f(-ext.w, -ext.h);
			glTexCoord2f(uvMax.x, uvMin.y); glVertex2f(ext.w, -ext.h);
			glEnd();;

			glBindTexture(GL_TEXTURE_2D, 0);
		}
	}
}