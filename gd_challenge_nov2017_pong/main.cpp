#define FPL_IMPLEMENTATION
#include <final_platform_layer.hpp>

#include "pong.h"

int main(int argc, char **args) {
	fs::games::BaseGame *game = new fs::games::Pong();
	fs::games::RunGame(game);
	delete game;
}