#pragma once
#include <glad\glad.h>
#include <memory>

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
	explicit TileRing(size_t _innerWidth, size_t _outerWidth, float _tileSize);
	~TileRing();
	TileRing(const TileRing&) = delete;
	TileRing& operator=(const TileRing&) = delete;
	void render() const;
	float getTileSize() const;

private:
	std::unique_ptr<TileData[]> tileData;
	GLuint VAO;
	GLuint VBO;
	GLuint EBO;
	size_t outerWidth;
	size_t innerWidth;
	size_t ringWidth;
	size_t numtiles;
	float tileSize;

	void assignNeighborSizes(size_t _x, size_t _y, TileData *_tileData);
	void generateTileData();
};