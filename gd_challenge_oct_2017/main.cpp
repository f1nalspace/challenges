#define FPL_IMPLEMENTATION
#include <final_platform_layer.hpp>
#include "game.h"

int main(int argc, char **args) {
	fs::games::BaseGame *game = new fs::games::mygame::Game();
	fs::games::RunGame(game);
	delete game;
}