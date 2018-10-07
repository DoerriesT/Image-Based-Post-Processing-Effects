#include "TileRing.h"
#include <cassert>

TileRing::TileRing(size_t _innerWidth, size_t _outerWidth, float _tileSize)
	:innerWidth(_innerWidth),
	outerWidth(_outerWidth),
	tileSize(_tileSize),
	ringWidth((_outerWidth - _innerWidth) / 2),
	numtiles(_outerWidth * _outerWidth - _innerWidth * _innerWidth)
{
	assert((outerWidth - innerWidth) % 2 == 0);
	generateTileData();
}

TileRing::~TileRing()
{
	glBindVertexArray(VAO);
	glDisableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glDeleteBuffers(1, &EBO);
	glDeleteBuffers(1, &VBO);

	glBindVertexArray(0);
	glDeleteVertexArrays(1, &VAO);
}

void TileRing::render() const
{
	glBindVertexArray(VAO);
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glPatchParameteri(GL_PATCH_VERTICES, 4);
	glDrawElementsInstanced(GL_PATCHES, TILE_DIMENSIONS * TILE_DIMENSIONS * 4, GL_UNSIGNED_INT, NULL, static_cast<GLsizei>(numtiles));
}

float TileRing::getTileSize() const
{
	return tileSize;
}

void TileRing::assignNeighborSizes(size_t _x, size_t _y, TileData * _tileData)
{
	_tileData->neighbourPlusX = 1.0f;
	_tileData->neighbourPlusY = 1.0f;
	_tileData->neighbourMinusX = 1.0f;
	_tileData->neighbourMinusY = 1.0f;

	// TBD: these aren't necessarily 2x different.  Depends on the relative tiles sizes supplied to ring ctors.
	const float innerNeighbourSize = 0.5f;
	const float outerNeighbourSize = 2.0f;

	// Inner edges abut tiles that are smaller.  (But not on the inner-most.)
	if (innerWidth > 0)
	{
		if (_y >= ringWidth && _y < outerWidth - ringWidth)
		{
			if (_x == ringWidth - 1)
			{
				_tileData->neighbourPlusX = innerNeighbourSize;
			}
			if (_x == outerWidth - ringWidth)
			{
				_tileData->neighbourMinusX = innerNeighbourSize;
			}
		}
		if (_x >= ringWidth && _x < outerWidth - ringWidth)
		{
			if (_y == ringWidth - 1)
			{
				_tileData->neighbourPlusY = innerNeighbourSize;
			}
			if (_y == outerWidth - ringWidth)
			{
				_tileData->neighbourMinusY = innerNeighbourSize;
			}
		}
	}

	// Outer edges abut tiles that are larger.  We could skip this on the outer-most ring.  But it will
	// make almost zero visual or perf difference.
	if (_x == 0)
	{
		_tileData->neighbourMinusX = outerNeighbourSize;
	}
	if (_y == 0)
	{
		_tileData->neighbourMinusY = outerNeighbourSize;
	}
	if (_x == outerWidth - 1)
	{
		_tileData->neighbourPlusX = outerNeighbourSize;
	}
	if (_y == outerWidth - 1)
	{
		_tileData->neighbourPlusY = outerNeighbourSize;
	}
}

void TileRing::generateTileData()
{
	size_t index = 0;
	tileData = std::make_unique<TileData[]>(numtiles);

	const float halfWidth = 0.5f * (float)outerWidth;
	for (size_t y = 0; y < outerWidth; ++y)
	{
		for (size_t x = 0; x < outerWidth; ++x)
		{
			if (x < ringWidth || y < ringWidth || x >= outerWidth - ringWidth || y >= outerWidth - ringWidth)
			{
				tileData[index].x = tileSize * ((float)x - halfWidth);
				tileData[index].y = tileSize * ((float)y - halfWidth);
				assignNeighborSizes(x, y, &tileData[index]);
				index++;
			}
		}
	}
	assert(index == numtiles);


	GLuint indices[TILE_DIMENSIONS * TILE_DIMENSIONS * 4];
	int currentIndex = 0;
	for (int row = 0; row < TILE_DIMENSIONS; ++row)
	{
		for (int column = 0; column < TILE_DIMENSIONS; ++column)
		{
			indices[currentIndex++] = (row * (TILE_DIMENSIONS + 1) + column);
			indices[currentIndex++] = (row * (TILE_DIMENSIONS + 1) + column) + 1;
			indices[currentIndex++] = ((row + 1) * (TILE_DIMENSIONS + 1) + column) + 1;
			indices[currentIndex++] = ((row + 1) * (TILE_DIMENSIONS + 1) + column);
		}
	}

	// create buffers/arrays
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, numtiles * sizeof(TileData), tileData.get(), GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	// patch position
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(TileData), (void*)0);
	glVertexAttribDivisor(0, 1);

	// vertex texture coord
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(TileData), (void*)offsetof(TileData, neighbourMinusX));
	glVertexAttribDivisor(1, 1);

	glBindVertexArray(0);
}
