// StE
// � Shlomi Steinberg, 2015-2017

#pragma once

#include <stdafx.hpp>

#include <rendering_presentation_system.hpp>
#include <presentation_engine.hpp>
#include <pipeline_external_binding_set_collection.hpp>

#include <camera.hpp>
#include <scene.hpp>

#include <ste_context.hpp>
#include <ste_resource.hpp>

#include <atmospherics_buffer.hpp>
#include <transforms_ring_buffers.hpp>
#include <deferred_composer.hpp>
#include <linked_light_lists.hpp>
#include <shadowmap_storage.hpp>
#include <volumetric_scattering_storage.hpp>
#include <fxaa_postprocess.hpp>
#include <hdr_dof_postprocess.hpp>
#include <scene_prepopulate_depth_fragment.hpp>
#include <scene_geo_cull_fragment.hpp>
#include <linked_light_lists_gen_dispatch.hpp>
#include <light_preprocessor.hpp>
#include <shadowmap_projector.hpp>
#include <directional_shadowmap_projector.hpp>
#include <volumetric_scattering_scatter_dispatch.hpp>
#include <gbuffer_downsample_depth_fragment.hpp>

#include <signal.hpp>

namespace ste {
namespace graphics {

class primary_renderer : gl::rendering_presentation_system {
	using Base = gl::rendering_presentation_system;

private:
	std::reference_wrapper<gl::presentation_engine> presentation;
	const camera<float> *cam;
	scene *s;

	gl::ste_device::queues_and_surface_recreate_signal_type::connection_type resize_signal_connection;

private:
	transforms_ring_buffers transform_buffers;
	atmospherics_buffer atmospheric_buffer;

	ste_resource<linked_light_lists> lll_storage;
	ste_resource<shadowmap_storage> shadows_storage;
	ste_resource<volumetric_scattering_storage> vol_scat_storage;

	gl::pipeline_external_binding_set_collection common_binding_set_collection;

private:
	ste_resource<deferred_composer> composer;
	ste_resource<fxaa_postprocess> fxaa;
	ste_resource<hdr_dof_postprocess> hdr;

	ste_resource<gbuffer_downsample_depth_fragment> downsample_depth;
	ste_resource<scene_prepopulate_depth_front_face_fragment> prepopulate_depth_dispatch;
	ste_resource<scene_prepopulate_depth_back_face_fragment> prepopulate_backface_depth_dispatch;
	ste_resource<scene_geo_cull_fragment> scene_geo_cull;

	ste_resource<linked_light_lists_gen_dispatch> lll_gen_dispatch;
	ste_resource<light_preprocessor> light_preprocess;

	ste_resource<shadowmap_projector> shadows_projector;
	ste_resource<directional_shadowmap_projector> directional_shadows_projector;

	ste_resource<volumetric_scattering_scatter_dispatch> vol_scat_scatter;

private:
	static gl::framebuffer_layout create_fb_layout(const ste_context &ctx);
	static gl::pipeline_external_binding_set_collection create_common_binding_set_collection(const ste_context &ctx,
																							 const scene *s);

	/**
	*	@brief		Updates common descriptor set's bindings
	*/
	void update();

public:
	primary_renderer(const ste_context &ctx,
					 gl::presentation_engine &presentation,
					 const camera<float> *cam,
					 scene *s,
					 const atmospherics_properties<double> &atmospherics_prop);
	~primary_renderer() noexcept {}

	const gl::pipeline_external_binding_set_collection* external_binding_sets() const override final {
		return &common_binding_set_collection;
	}

	/*
	 *	@brief		Updates atmospheric properties.
	 */
	void update_atmospherics_properties(const atmospherics_properties<double> &atmospherics_prop) { atmospheric_buffer.update_data(atmospherics_prop); }

	/**
	*	@brief	Set the camera aperture parameter. Those parameters affect the depth of field of the resulting image.
	*
	* 	@param diameter		Lens diameter in world units. Defaults to human eye pupil diameter which ranges from 2e-3 to 8e-3.
	*	@param focal_length	Focal length world units. Defaults to human eye focal length, about 23e-3.
	*/
	void set_aperture_parameters(float diameter, float focal_length) { hdr.get().set_aperture_parameters(diameter, focal_length); }

	/**
	*	@brief		Performs rendering and presentation.
	*/
	void present() override final;
};

}
}
