//	StE
// © Shlomi Steinberg, 2015-2017

#pragma once

#include <stdafx.hpp>
#include <ste_context.hpp>

#include <material_lut_storage.hpp>
#include <atmospherics_lut_storage.hpp>

#include <fragment_graphics.hpp>

#include <task.hpp>
#include <cmd_draw.hpp>

namespace ste {
namespace graphics {

class deferred_composer : public gl::fragment_graphics<deferred_composer> {
	using Base = gl::fragment_graphics<deferred_composer>;

private:
	gl::task<gl::cmd_draw> draw_task;

	gl::rendering_system::storage_ptr<material_lut_storage> material_luts;
	gl::rendering_system::storage_ptr<atmospherics_lut_storage> atmospherics_luts;

public:
	deferred_composer(const gl::rendering_system &rs)
		: Base(rs,
			   gl::device_pipeline_graphics_configurations{},
			   "fullscreen_triangle.vert", "deferred_compose.frag")
	{
		draw_task.attach_pipeline(pipeline);
	}
	~deferred_composer() noexcept {}

	static const lib::string& name() { return "deferred_composer"; }

	static void setup_graphics_pipeline(const gl::rendering_system &rs,
										gl::pipeline_auditor_graphics &auditor) {
		gl::framebuffer_layout fb_layout;
		fb_layout[0] = gl::ignore_store(gl::format::r16g16b16a16_sfloat,
										gl::image_layout::shader_read_only_optimal);
		auditor.set_framebuffer_layout(fb_layout);
	}

	void attach_framebuffer(gl::framebuffer &fb) {
		pipeline.attach_framebuffer(fb);
	}
	const auto& get_framebuffer_layout() const {
		return pipeline.get_framebuffer_layout();
	}

	void record(gl::command_recorder &recorder) override final {
		recorder << draw_task(3, 1);
	}
};

}
}
