//	StE
// � Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <camera_projection_model.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace ste {
namespace graphics {

/**
*	@brief	Perspective projection model
*/
template <typename T>
class camera_projection_perspective : public camera_projection_model<T> {
private:
	T fovy;
	T aspect;
	metre near_clip_plane;
	metre far_clip_plane;

	glm::tmat4x4<T> proj_mat;

public:
	/*
	 *	@param	fovy		Field-of-view in y axis, in radians.
	 *	@param	aspect		Aspect ratio of x to y
	 *	@param	near_clip_plane	Distance to the near clip plane
	 *	@param	far_clip_plane	Distance to the far clip plane
	 */
	camera_projection_perspective(T fovy, 
								  T aspect, 
								  metre near_clip_plane,
								  metre far_clip_plane)
		: fovy(fovy), aspect(aspect), near_clip_plane(near_clip_plane), far_clip_plane(far_clip_plane),
		proj_mat(glm::perspectiveRH(fovy, aspect, near_clip_plane, far_clip_plane))
	{
		assert(aspect > 0);
		assert(fovy > 0);
		assert(near_clip_plane > 0);
		assert(far_clip_plane > 0);
		assert(far_clip_plane > near_clip_plane);
	}
	~camera_projection_perspective() noexcept {}

	camera_projection_perspective(camera_projection_perspective&&) = default;
	camera_projection_perspective &operator=(camera_projection_perspective&&) = default;
	camera_projection_perspective(const camera_projection_perspective&) = default;
	camera_projection_perspective &operator=(const camera_projection_perspective&) = default;

	/*
	*	@brief	Projects a eye space position to homogeneous clip coordinates.
	*/
	glm::tvec4<T> project(metre_vec4 v) const final override {
		return proj_mat * v.v();
	}

	/*
	*	@brief	Projects a z value to a depth value.
	*/
	metre project_depth(metre z) const final override {
		auto u = project({ 0_m, 0_m, z, 1_m });
		return u.z / static_cast<T>(u.w);
	}

	/*
	*	@brief	Unprojects a depth value to the z value
	*/
	metre unproject_depth(metre d) const final override {
		auto a = proj_mat[3][2] - proj_mat[3][3] * static_cast<T>(d);
		auto b = proj_mat[2][3] * static_cast<T>(d) - proj_mat[2][2];
		return metre(a / b);
	}

	/*
	*	@brief	Unprojects a screen position, given with depth value and normalized screen coordinates, into eye space.
	*/
	metre_vec3 unproject_screen_position(metre depth, const glm::tvec2<T> &norm_frag_coords) const final override {
		const auto z = unproject_depth(depth);
		return unproject_screen_position_with_z(z, norm_frag_coords);
	}

	/*
	*	@brief	Unprojects a screen position, given with a eye space z value and normalized screen coordinates, into eye space.
	*/
	metre_vec3 unproject_screen_position_with_z(metre z, const glm::tvec2<T> &norm_frag_coords) const final override {
		auto t = norm_frag_coords * static_cast<T>(2);
		t -= glm::tvec2<T>(1);

		const auto &P = proj_mat;

		auto a = (P[1][1] * P[0][0]);
		auto b = P[2][3] * static_cast<T>(z) + P[3][3];
		auto tx = t.x * b;
		auto ty = t.y * b;

		auto x = (P[1][0] * (ty - P[3][1]) / a + (tx - P[3][0]) / P[0][0]) / (static_cast<T>(1) - P[1][0] * P[0][1] / a);
		auto y = (ty - P[0][1] * x - P[3][1]) / P[1][1];

		return { metre(x), metre(y), z };
	}

	/*
	*	@brief	Creates a projection matrix
	*/
	glm::tmat4x4<T> projection_matrix() const final override {
		return proj_mat;
	}

	auto get_fovy() const { return fovy; }
	auto get_aspect() const { return aspect; }
	auto get_near_clip_plane() const { return near_clip_plane; }
	auto get_far_clip_plane() const { return far_clip_plane; }
};

}
}
