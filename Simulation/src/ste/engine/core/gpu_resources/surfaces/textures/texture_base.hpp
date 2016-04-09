// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include "stdafx.hpp"
#include "gl_utils.hpp"

#include "Log.hpp"
#include "gl_current_context.hpp"

#include "bindable_resource.hpp"
#include "layout_binding.hpp"

#include "RenderTarget.hpp"
#include "Sampler.hpp"

#include "texture_handle.hpp"
#include "texture_enums.hpp"
#include "texture_traits.hpp"
#include "texture_allocator.hpp"
#include "texture_cubemap_face.hpp"

#include "surface_constants.hpp"

#include <type_traits>
#include <gli/gli.hpp>

namespace StE {
namespace Core {

class texture_layout_binding_type {};
using texture_layout_binding = layout_binding<texture_layout_binding_type>;
texture_layout_binding inline operator "" _tex_unit(unsigned long long int i) { return texture_layout_binding(i); }

template <core_resource_type type>
class TextureBinder {
private:
	static constexpr GLenum gl_type() { return gl_utils::translate_type(type); }

public:
	static void bind(GenericResource::type id, const texture_layout_binding &sampler) {
		gl_current_context::get()->bind_texture_unit(sampler.binding_index(), id);
	}
	static void unbind(const texture_layout_binding &sampler) {
		gl_current_context::get()->bind_texture_unit(sampler.binding_index(), 0);
	}
};

template <core_resource_type type>
class texture : virtual public bindable_resource<texture_immutable_storage_allocator<type>, TextureBinder<type>, texture_layout_binding>,
				virtual public shader_layout_bindable_resource<texture_layout_binding_type> {
private:
	using Base = bindable_resource<texture_immutable_storage_allocator<type>, TextureBinder<type>, texture_layout_binding>;

public:
	using size_type = typename texture_size_type<texture_dimensions<type>::dimensions>::type;
	using image_size_type = typename texture_size_type<texture_dimensions<type>::dimensions>::type;
	static constexpr core_resource_type T = type;

protected:
	static constexpr GLenum gl_type() { return gl_utils::translate_type(type); }

	int levels, samples;
	typename texture_size_type<texture_dimensions<type>::dimensions>::type size;
	gli::format format;
	gli::swizzles swizzle{ swizzles_rgba };

protected:
	auto get_image_container_size() const {
		glm::ivec2 s(1);
		for (int i = 0; i < std::min(2, dimensions()); ++i) s[i] = size[i];
		return s;
	}
	int get_image_container_dimensions() const { return dimensions() > 2 ? size[2] : get_layers(); }

	bool allocate_tex_storage(const size_type &size, const gli::format &gli_format, const gli::swizzles &swizzle, int levels, int samples, bool sparse, int page_size_idx = 0) {
		gli::gl::format const glformat = gl_utils::translate_format(gli_format, swizzle);

		auto id = Base::get_resource_id();
		if (sparse) {
			glTextureParameteri(id, GL_TEXTURE_SPARSE_ARB, true);
			glTextureParameteri(id, GL_VIRTUAL_PAGE_SIZE_INDEX_ARB, page_size_idx);
		}
		glTextureParameteri(id, GL_TEXTURE_BASE_LEVEL, 0);
		glTextureParameteri(id, GL_TEXTURE_MAX_LEVEL, levels - 1);
		glTextureParameteri(id, GL_TEXTURE_SWIZZLE_R, glformat.Swizzles[0]);
		glTextureParameteri(id, GL_TEXTURE_SWIZZLE_G, glformat.Swizzles[1]);
		glTextureParameteri(id, GL_TEXTURE_SWIZZLE_B, glformat.Swizzles[2]);
		glTextureParameteri(id, GL_TEXTURE_SWIZZLE_A, glformat.Swizzles[3]);

		this->format = gli_format;
		this->swizzle = swizzle;
		this->size = size;
		this->levels = levels;

		Base::allocator.allocate_storage(Base::get_resource_id(), levels, samples, glformat, size, get_storage_size());

		return true;
	}

	texture() {}

public:
	texture(texture &&m) = default;
	texture& operator=(texture &&m) = default;
	texture(const texture &m) = delete;
	texture& operator=(const texture &m) = delete;

	virtual ~texture() noexcept {}

	void bind() const { bind(LayoutLocationType(0)); }
	void unbind() const { unbind(LayoutLocationType(0)); }
	void bind(const LayoutLocationType &sampler) const final override { Base::bind(sampler); };
	void unbind(const LayoutLocationType &sampler) const final override { Base::unbind(sampler); };

	constexpr int dimensions() const { return texture_dimensions<type>::dimensions; }
	constexpr bool is_array_texture() const { return texture_is_array<type>::value; }
	constexpr bool is_multisampled() const { return texture_is_multisampled<type>::value; }

	void clear(void *data, int level = 0) {
		gli::gl::format glformat = gl_utils::translate_format(format, swizzle);
		glClearTexImage(Base::get_resource_id(), level, glformat.External, glformat.Type, data);
	}
	void clear(void *data, glm::i32vec3 offset, glm::u32vec3 size, int level = 0) {
		gli::gl::format glformat = gl_utils::translate_format(format, swizzle);
		glClearTexSubImage(Base::get_resource_id(), level, offset.x, offset.y, offset.z, size.x, size.y, size.z, glformat.External, glformat.Type, data);
	}

	auto get_size() const { return size; }
	auto get_image_size(int level) const {
		image_size_type ret;
		for (int i = 0; i < texture_layer_dimensions<type>::dimensions; ++i)
			ret[i] = size[i] >> level;
		return ret;
	}
	auto get_image_size() const { return get_image_size(0); }
	gli::format get_format() const { return format; }
	int get_layers() const { return texture_is_array<type>::value ? this->size[texture_dimensions<type>::dimensions - 1] : 1; }
	bool is_compressed() const { return gli::is_compressed(format); }

	std::size_t get_storage_size(int level) const {
		std::size_t b = gli::block_size(format);
		auto block_extend = gli::block_extent(format);
		int i;
		for (i = 0; i < texture_layer_dimensions<type>::dimensions; ++i) b *= std::max<decltype(size.x)>(1u, size[i] >> static_cast<decltype(size.x)>(level));
		for (; i < dimensions(); ++i) b *= size[i] / block_extend[i];
		return b;
	}
	std::size_t get_storage_size() const {
		int t = 0;
		for (int l = 0; l < levels; ++l) t += get_storage_size(l);
		return t;
	}

	auto get_texture_handle() const {
		return texture_handle(glGetTextureHandleARB(Base::get_resource_id()));
	}
	auto get_texture_handle(const Sampler &sam) const {
		GenericResource::type sam_id = sam.get_resource_id();
		return texture_handle(glGetTextureSamplerHandleARB(Base::get_resource_id(), sam_id));
	}

	core_resource_type resource_type() const override { return type; }
};

template <core_resource_type type>
class texture_multisampled : public texture<type> {
private:
	using Base = texture<type>;

protected:
	texture_multisampled(gli::format format, const typename Base::size_type &size, int samples, const gli::swizzles &swizzle = swizzles_rgba) {
		allocate_tex_storage(size, format, swizzle, 1, samples, false);
	}

public:
	texture_multisampled(texture_multisampled &&m) = default;
	texture_multisampled& operator=(texture_multisampled &&m) = default;

	virtual ~texture_multisampled() noexcept {}

	int get_samples() const { return samples; }
};

template <core_resource_type type>
class texture_pixel_transferable : public texture<type> {
private:
	using Base = texture<type>;

protected:
	using Base::texture;

	texture_pixel_transferable() {}

	texture_pixel_transferable(texture_pixel_transferable &&m) = default;
	texture_pixel_transferable& operator=(texture_pixel_transferable &&m) = default;
	texture_pixel_transferable(const texture_pixel_transferable &m) = delete;
	texture_pixel_transferable& operator=(const texture_pixel_transferable &m) = delete;

public:
	virtual ~texture_pixel_transferable() noexcept {}

	virtual void upload_level(const void *data, int level = 0, int layer = 0, CubeMapFace face = CubeMapFace::CubeMapFaceNone, int data_size = 0) = 0;

	virtual void download_level(void *data,
								std::size_t size,
								int level) const {
		auto gl_format = gl_utils::translate_format(Base::format, Base::swizzle);
		glGetTextureImage(Base::get_resource_id(), level, gl_format.External, gl_format.Type, size, data);
	}
	virtual void download_level(void *data,
								std::size_t size,
								int level,
								const gli::format &format,
								const gli::swizzles &swizzle = swizzles_rgba,
								bool compressed = false) const {
		auto gl_format = gl_utils::translate_format(format, swizzle);
		if (compressed)
			glGetCompressedTextureImage(Base::get_resource_id(), level, size, data);
		else
			glGetTextureImage(Base::get_resource_id(), level, gl_format.External, gl_format.Type, size, data);
	}
};

template <core_resource_type type>
class texture_mipmapped : public texture_pixel_transferable<type> {
private:
	using Base = texture_pixel_transferable<type>;

protected:
	texture_mipmapped() {}
	texture_mipmapped(gli::format format, const typename Base::size_type &size, int levels, const gli::swizzles &swizzle = swizzles_rgba) : Base() {
		allocate_tex_storage(size, format, swizzle, levels, 1, false);
	}

public:
	texture_mipmapped(texture_mipmapped &&m) = default;
	texture_mipmapped& operator=(texture_mipmapped &&m) = default;

	virtual ~texture_mipmapped() noexcept {}

	int get_levels() const { return levels; }

	void generate_mipmaps() {
		Base::bind();
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(Base::gl_type(), Base::get_resource_id());
		glGenerateMipmap(Base::gl_type());
	}

	static int calculate_mipmap_max_level(const typename texture_size_type<texture_layer_dimensions<type>::dimensions>::type &size) {
		int levels = 0;
		for (int i = 0; i < texture_layer_dimensions<type>::dimensions; ++i) {
			int l;
			auto x = size[i];
			for (l = 0; x >> l > 1; ++l);
			levels = std::max(l, levels);
		}
		return levels;
	}
};

}
}
