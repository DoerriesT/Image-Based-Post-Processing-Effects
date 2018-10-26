#pragma once
#include <glad\glad.h>
#include <memory>

struct TileData
{
	float m_x;
	float m_y;
	float m_neighbourMinusX;
	float m_neighbourMinusY;
	float m_neighbourPlusX;
	float m_neighbourPlusY;
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
	std::unique_ptr<TileData[]> m_tileData;
	GLuint m_VAO;
	GLuint m_VBO;
	GLuint m_EBO;
	size_t m_outerWidth;
	size_t m_innerWidth;
	size_t m_ringWidth;
	size_t m_numtiles;
	float m_tileSize;

	void assignNeighborSizes(size_t _x, size_t _y, TileData *_tileData);
	void generateTileData();
};