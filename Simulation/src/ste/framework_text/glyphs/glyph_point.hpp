// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include <stdafx.hpp>

#include <vertex_attributes_from_tuple.hpp>

namespace StE {
namespace Text {

struct glyph_point {
	glm::vec2 pos; float glyph, size;
	glm::vec4 color;
	glm::vec4 stroke_color;
	float stroke_width;
	float weight;

	using descriptor = GL::vertex_attributes_from_tuple<glm::vec4, glm::vec4, glm::vec4, float, float>::descriptor;

	glyph_point() = default;
};

}
}
