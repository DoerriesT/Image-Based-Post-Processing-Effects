#include "TileRing.h"
#include <cassert>

TileRing::TileRing(size_t _innerWidth, size_t _outerWidth, float _tileSize)
	:m_innerWidth(_innerWidth),
	m_outerWidth(_outerWidth),
	m_tileSize(_tileSize),
	m_ringWidth((_outerWidth - _innerWidth) / 2),
	m_numtiles(_outerWidth * _outerWidth - _innerWidth * _innerWidth)
{
	assert((m_outerWidth - m_innerWidth) % 2 == 0);
	generateTileData();
}

TileRing::~TileRing()
{
	glBindVertexArray(m_VAO);
	glDisableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glDeleteBuffers(1, &m_EBO);
	glDeleteBuffers(1, &m_VBO);

	glBindVertexArray(0);
	glDeleteVertexArrays(1, &m_VAO);
}

void TileRing::render() const
{
	glBindVertexArray(m_VAO);
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glPatchParameteri(GL_PATCH_VERTICES, 4);
	glDrawElementsInstanced(GL_PATCHES, TILE_DIMENSIONS * TILE_DIMENSIONS * 4, GL_UNSIGNED_INT, NULL, static_cast<GLsizei>(m_numtiles));
}

float TileRing::getTileSize() const
{
	return m_tileSize;
}

void TileRing::assignNeighborSizes(size_t _x, size_t _y, TileData * _tileData)
{
	_tileData->m_neighbourPlusX = 1.0f;
	_tileData->m_neighbourPlusY = 1.0f;
	_tileData->m_neighbourMinusX = 1.0f;
	_tileData->m_neighbourMinusY = 1.0f;

	// TBD: these aren't necessarily 2x different.  Depends on the relative tiles sizes supplied to ring ctors.
	const float innerNeighbourSize = 0.5f;
	const float outerNeighbourSize = 2.0f;

	// Inner edges abut tiles that are smaller.  (But not on the inner-most.)
	if (m_innerWidth > 0)
	{
		if (_y >= m_ringWidth && _y < m_outerWidth - m_ringWidth)
		{
			if (_x == m_ringWidth - 1)
			{
				_tileData->m_neighbourPlusX = innerNeighbourSize;
			}
			if (_x == m_outerWidth - m_ringWidth)
			{
				_tileData->m_neighbourMinusX = innerNeighbourSize;
			}
		}
		if (_x >= m_ringWidth && _x < m_outerWidth - m_ringWidth)
		{
			if (_y == m_ringWidth - 1)
			{
				_tileData->m_neighbourPlusY = innerNeighbourSize;
			}
			if (_y == m_outerWidth - m_ringWidth)
			{
				_tileData->m_neighbourMinusY = innerNeighbourSize;
			}
		}
	}

	// Outer edges abut tiles that are larger.  We could skip this on the outer-most ring.  But it will
	// make almost zero visual or perf difference.
	if (_x == 0)
	{
		_tileData->m_neighbourMinusX = outerNeighbourSize;
	}
	if (_y == 0)
	{
		_tileData->m_neighbourMinusY = outerNeighbourSize;
	}
	if (_x == m_outerWidth - 1)
	{
		_tileData->m_neighbourPlusX = outerNeighbourSize;
	}
	if (_y == m_outerWidth - 1)
	{
		_tileData->m_neighbourPlusY = outerNeighbourSize;
	}
}

void TileRing::generateTileData()
{
	size_t index = 0;
	m_tileData = std::make_unique<TileData[]>(m_numtiles);

	const float halfWidth = 0.5f * (float)m_outerWidth;
	for (size_t y = 0; y < m_outerWidth; ++y)
	{
		for (size_t x = 0; x < m_outerWidth; ++x)
		{
			if (x < m_ringWidth || y < m_ringWidth || x >= m_outerWidth - m_ringWidth || y >= m_outerWidth - m_ringWidth)
			{
				m_tileData[index].m_x = m_tileSize * ((float)x - halfWidth);
				m_tileData[index].m_y = m_tileSize * ((float)y - halfWidth);
				assignNeighborSizes(x, y, &m_tileData[index]);
				index++;
			}
		}
	}
	assert(index == m_numtiles);


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
	glGenVertexArrays(1, &m_VAO);
	glGenBuffers(1, &m_VBO);
	glGenBuffers(1, &m_EBO);
	glBindVertexArray(m_VAO);
	glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
	glBufferData(GL_ARRAY_BUFFER, m_numtiles * sizeof(TileData), m_tileData.get(), GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	// patch position
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(TileData), (void*)0);
	glVertexAttribDivisor(0, 1);

	// vertex texture coord
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(TileData), (void*)offsetof(TileData, m_neighbourMinusX));
	glVertexAttribDivisor(1, 1);

	glBindVertexArray(0);
}
