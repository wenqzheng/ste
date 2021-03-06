// StE
// � Shlomi Steinberg, 2015-2017

#pragma once

#include <human_vision_properties.hpp>

#include <storage.hpp>
#include <ste_context.hpp>
#include <ste_resource.hpp>

#include <hdr_dof_bokeh_parameters.hpp>

#include <texture.hpp>
#include <array.hpp>
#include <std430.hpp>

#include <surface.hpp>
#include <surface_factory.hpp>

namespace ste {
namespace graphics {

class hdr_dof_postprocess_storage : public gl::storage<hdr_dof_postprocess_storage> {
	friend class hdr_dof_postprocess;

private:
	static hdr_bokeh_parameters parameters_initial;
	static constexpr float vision_properties_max_lum = 10.f;
	static constexpr std::uint32_t bins = 1024;

	ste_resource<gl::texture<gl::image_type::image_2d>> create_hdr_vision_properties_texture(const ste_context &ctx) {
		static constexpr auto format = gl::format::r32g32_sfloat;

		auto hdr_human_vision_properties_data = resource::surface_2d<format>(glm::tvec2<std::size_t>{ 4096, 1 });
		auto level = hdr_human_vision_properties_data[0_mip];
		{
			for (std::uint32_t i = 0; i < hdr_human_vision_properties_data.extent().x; ++i) {
				const float x = (static_cast<float>(i) + .5f) / static_cast<float>(hdr_human_vision_properties_data.extent().x);
				const float l = glm::mix(ste::graphics::human_vision_properties::min_luminance,
										 vision_properties_max_lum,
										 x);
				level.at({ i, 0 }).r() = ste::graphics::human_vision_properties::scotopic_vision(l);
				level.at({ i, 0 }).g() = ste::graphics::human_vision_properties::mesopic_vision(l);
			}
		}

		auto image = resource::surface_factory::image_from_surface_2d<format>(ctx,
																			  std::move(hdr_human_vision_properties_data),
																			  gl::image_usage::sampled,
																			  gl::image_layout::shader_read_only_optimal,
																			  "HDR vision properties texture",
																			  false);
		return ste_resource<gl::texture<gl::image_type::image_2d>>(ctx, std::move(image));
	}

public:
	ste_resource<gl::texture<gl::image_type::image_2d>> hdr_vision_properties_texture;

	gl::array<hdr_bokeh_parameters> hdr_bokeh_param_buffer;
	gl::array<hdr_bokeh_parameters> hdr_bokeh_param_buffer_prev;
	gl::array<gl::std430<std::uint32_t>> histogram;
	gl::array<gl::std430<std::uint32_t>> histogram_sums;

public:
	hdr_dof_postprocess_storage(const ste_context &ctx) 
		: hdr_vision_properties_texture(create_hdr_vision_properties_texture(ctx)),
		// Buffers
		hdr_bokeh_param_buffer(ctx, 
							   1, 
							   { parameters_initial }, 
							   gl::buffer_usage::storage_buffer,
							   "hdr_bokeh_param_buffer"),
		hdr_bokeh_param_buffer_prev(ctx, 
									1, 
									{ parameters_initial }, 
									gl::buffer_usage::storage_buffer,
									"hdr_bokeh_param_buffer_prev"),
		histogram(ctx, 
				  bins,
				  gl::buffer_usage::storage_buffer,
				  "histogram"),
		histogram_sums(ctx, 
					   bins,
					   gl::buffer_usage::storage_buffer,
					   "histogram_sums")
	{}
	~hdr_dof_postprocess_storage() noexcept {}
};

}
}
