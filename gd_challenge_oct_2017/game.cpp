#include "game.h"

#include <GL\glew.h>

using namespace finalspace::renderer;

namespace finalspace {
	namespace games {

		constexpr f32 GameAspect = 16.0f / 9.0f;
		constexpr f32 GameWidth = 20.0f;
		constexpr f32 GameHeight = GameWidth / GameAspect;
		constexpr f32 HalfGameWidth = GameWidth * 0.5f;
		constexpr f32 HalfGameHeight = GameHeight * 0.5f;

		void Game::Init() {
			glClearColor(0.2f, 0.3f, 0.8f, 1.0f);
		}

		void Game::Render(RenderState &render) {
			glViewport(0, 0, render.windowSize.w, render.windowSize.h);

			glMatrixMode(GL_PROJECTION);
			glLoadIdentity();
			glOrtho(-HalfGameWidth, HalfGameWidth, -HalfGameHeight, HalfGameHeight, 0.0f, 1.0f);
			glMatrixMode(GL_MODELVIEW);
			glLoadIdentity();

			glClear(GL_COLOR_BUFFER_BIT);

			glColor3f(1.0f, 1.0f, 1.0f);
			glPushMatrix();
			glTranslatef(position.x, position.y, 0.0f);
			glBegin(GL_QUADS);
			glVertex2f(0.5f, 0.5f);
			glVertex2f(-0.5f, 0.5f);
			glVertex2f(-0.5f, -0.5f);
			glVertex2f(0.5f, -0.5f);
			glEnd();
			glPopMatrix();
		}

		void Game::Update(const Input &input) {
			const Controller &playerController = input.controller[input.playerControllerIndex];
			Vec2f accel = Vec2f();
			f32 speed = 0.5f;
			if (playerController.moveLeft.isDown) {
				accel.x = -1.0f * speed;
			} else if (playerController.moveRight.isDown) {
				accel.x = 1.0f * speed;
			}
			if (playerController.moveUp.isDown) {
				accel.y = 1.0f * speed;
			} else if (playerController.moveDown.isDown) {
				accel.y = -1.0f * speed;
			}
			if (accel.x != 0 || accel.y != 0) {
				accel = Normalize(accel);
			}
			position += input.deltaTime * accel;
		}

	}
}