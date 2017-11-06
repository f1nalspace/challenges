#include "pong.h"

#include <final_platform_layer.hpp>

namespace fs {
	namespace games {
		Pong::Pong() {
			title = "Pongy";
		}
		Pong::~Pong() {

		}
		void Pong::Init() {
			glClearColor(0.3f, 0.5f, 0.8f, 1.0f);
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		}
		void Pong::Release() {

		}
		void Pong::HandleInput(const Input &input) {

		}
		void Pong::Update(const Input &input) {

		}
		void Pong::Render() {
			renderer->Update(GAME_WIDTH * 0.5f, GAME_HEIGHT * 0.5f, GAME_ASPECT);

			glViewport(renderer->viewport.offset.x, renderer->viewport.offset.y, renderer->viewport.size.w, renderer->viewport.size.h);

			glClear(GL_COLOR_BUFFER_BIT);

			renderer->DrawRectangle(Vec2f(0, 0), Vec2f(GAME_WIDTH, GAME_HEIGHT) * 0.5f, Vec4f::White, false);
		}
	}
}