#include <iostream>
#include "GameEngine.h"

int main(int argc, char* argv[]) {
	GameEngine engine;
	engine.Initialize();
	engine.Run();

	return 0;
}