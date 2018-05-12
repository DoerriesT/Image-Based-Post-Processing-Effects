#include <Engine.h>
#include "Application.h"

int main()
{
	Application app;
	Engine engine("Image-Based Post-Processing Effects", app);
	engine.start();
}