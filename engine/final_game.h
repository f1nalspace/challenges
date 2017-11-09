#pragma once

#if FS_ENABLE_IMGUI
#	define IMGUI_DISABLE_OBSOLETE_FUNCTIONS 1
#	include <imgui/imgui.h>
#	include <imgui/imgui_internal.h>
#endif

#include "final_renderer.h"
#include "final_input.h"

using namespace fs::renderer;
using namespace fs::inputs;

namespace fs {
	namespace games {
		class BaseGame {
		protected:
			u32 initialWidth;
			u32 initialHeight;
			char *title;
			Renderer *renderer;
			bool exitRequested;
		public:
			BaseGame() :
				renderer(nullptr),
				exitRequested(false),
				initialWidth(1280),
				initialHeight(720),
				title(nullptr) {
			}
			virtual void Init() = 0;
			virtual void Release() = 0;
			virtual void HandleInput(const Input &input) = 0;
			virtual void Update(const Input &input) = 0;
			virtual void Render(const Input &input) = 0;
			virtual ~BaseGame() {
			}
			inline bool IsExitRequested() const {
				return exitRequested;
			}
			inline u32 GetInitialWidth() const {
				return initialWidth;
			}
			inline u32 GetInitialHeight() const {
				return initialHeight;
			}
			inline const char *GetTitle() const {
				return title;
			}
			inline void SetRenderer(Renderer *renderer) {
				this->renderer = renderer;
			}
		};

		extern void RunGame(BaseGame *game);
	};
};