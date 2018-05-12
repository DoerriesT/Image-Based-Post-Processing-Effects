#include <Engine.h>
#include "Application.h"

int main()
{
	App::Application app;
	Engine engine("Image-Based Post-Processing Effects", app);
	engine.start();
}