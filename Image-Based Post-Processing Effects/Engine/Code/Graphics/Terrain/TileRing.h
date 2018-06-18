#pragma once
#include <glad\glad.h>

struct TileData
{
	float x;
	float y;
	float neighbourMinusX;
	float neighbourMinusY;
	float neighbourPlusX;
	float neighbourPlusY;
};

// Reminder: there are two constants in terrain.vert which depend on this value
const unsigned int TILE_DIMENSIONS = 8;

class TileRing
{
public:
	explicit TileRing(int _innerWidth, int _outerWidth, float _tileSize);
	~TileRing();
	TileRing(const TileRing&) = delete;
	TileRing& operator=(const TileRing&) = delete;
	void render() const;
	float getTileSize() const;

private:
	int outerWidth;
	int innerWidth;
	int ringWidth;
	int numtiles;
	float tileSize;
	TileData *tileData;
	GLuint VAO;
	GLuint VBO;
	GLuint EBO;

	void assignNeighborSizes(int _x, int _y, TileData *_tileData);
	void generateTileData();
};