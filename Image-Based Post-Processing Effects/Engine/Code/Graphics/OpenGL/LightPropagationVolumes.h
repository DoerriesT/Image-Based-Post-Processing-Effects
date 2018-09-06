#pragma once
#include <glad/glad.h>

class LightPropagationVolumes
{
public:
	void init();
	void prepare();

private:
	GLuint rsmFbo;
	GLuint rsmDepth;
	GLuint rsmFlux;
	GLuint rsmNormal;

	GLuint propagationFbo;
	GLuint propagationVolumeRed;
	GLuint propagationVolumeGreen;
	GLuint propagationVolumeBlue;
};