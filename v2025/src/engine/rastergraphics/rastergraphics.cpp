//
// Created by mgrus on 21.02.2025.
//

#include "rastergraphics.h"

#include <span>
#include <engine/graphics/Texture.h>

PixelBuffer::PixelBuffer(Texture *texture) {
    dimension = { texture->width(), texture->height() };
	auto p8 = texture->pixels();
	pixels = std::span(p8, p8 + (uint32_t)(dimension.x * dimension.y * 4));
	format = PixelFormat::RGBA8;
}

void PixelBuffer::clear(glm::ivec4 color) {
#ifdef _PROFILING
	LARGE_INTEGER start;
	QueryPerformanceCounter(&start);
#endif

	uint32_t pattern = color.a << 24 | color.b << 16 | color.g << 8 | color.r;
	size_t numInts = pixels.size() / sizeof(uint32_t);
	std::fill(reinterpret_cast<uint32_t*>(pixels.data()),
			  reinterpret_cast<uint32_t*>(pixels.data()) + numInts,
			  pattern);

#ifdef _PROFILING
	LARGE_INTEGER end;
	QueryPerformanceCounter(&end);
	uint64_t elapsed = end.QuadPart - start.QuadPart;
	float time_in_secs = (float)elapsed / (float)performance_frequency;
	printf("time: %f\n", time_in_secs);
#endif


}

void PixelBuffer::drawPoint(glm::vec2 position, glm::vec4 color) {
	int memoryCell = (position.y * dimension.x * 4) + (position.x * 4);
	pixels[memoryCell] = color.r;
	pixels[memoryCell + 1] = color.g;
	pixels[memoryCell + 2] = color.b;
	pixels[memoryCell + 3] = color.a;

}

void PixelBuffer::drawLine(glm::ivec2 p1, glm::ivec2 p2, glm::vec4 color) {


	int dx = p2.x - p1.x;
	int dy = p2.y - p1.y;

	if (abs(dx) > abs(dy)) {
		if (dx < 0) {
			auto temp = p1;
			p1 = p2;
			p2 = temp;
		}
		drawXDominant(p1, p2, color);
	} else {
		if (dy < 0) {
			auto temp = p1;
			p1 = p2;
			p2 = temp;
		}
		drawYDominant(p1, p2, color);
	}


}

void PixelBuffer::drawRect(glm::ivec2 topLeft, glm::ivec2 bottomRight, glm::vec4 color) {
	drawLine(topLeft, {bottomRight.x, topLeft.y}, color);
	drawLine(topLeft, {topLeft.x, bottomRight.y}, color);
	drawLine({topLeft.x, bottomRight.y}, bottomRight, color);
	drawLine(bottomRight, {bottomRight.x, topLeft.y}, color);
}

void PixelBuffer::drawXDominant(glm::ivec2 p1, glm::ivec2 p2, glm::ivec4 color) {
	// We start with y from p1.
	float y = p1.y;
	float dx = p2.x - p1.x;
	float step = (p2.y - p1.y) / dx;

	// Note: it seems the 32 bit operation is not faster,
	// so we stick with 8 bits for now.
	// uint32_t* pdata32 = (uint32_t*)pixels.data();
	// uint32_t col32 = color.a << 24 | color.r << 16 | color.g << 8 | color.b;

	for (int x = p1.x; x <= p2.x; x++) {
		// int memcell32 = (y * dimension.x ) + (x );
		// pdata32[memcell32] = col32;
		int memoryCell = ((int)y * dimension.x * 4) + (x * 4);
		pixels[memoryCell] = color.r;
		pixels[memoryCell + 1] = color.g;
		pixels[memoryCell + 2] = color.b;
		pixels[memoryCell + 3] = color.a;
		// y += step;
	}

}

void PixelBuffer::drawYDominant(glm::ivec2 p1, glm::ivec2 p2, glm::ivec4 color) {
	// We start with x from p1.
	float x = p1.x;
	float dy = p2.y - p1.y;
	float step = (p2.x - p1.x) / dy;
	for (int y = p1.y; y <= p2.y; y++) {
		int memoryCell = (y * dimension.x * 4) + ((int)x * 4);
		pixels[memoryCell] = color.r;
		pixels[memoryCell + 1] = color.g;
		pixels[memoryCell + 2] = color.b;
		pixels[memoryCell + 3] = color.a;
		x += step;
	}
}