// StE
// � Shlomi Steinberg, 2015-2017

#pragma once

#include <stdafx.hpp>

#include <format.hpp>
#include <format_type_traits.hpp>

#include <image_type.hpp>
#include <image_type_traits.hpp>

#include <memory>

namespace ste {
namespace resource {

namespace _detail {

template<typename block_type, bool is_const>
class surface_storage {
private:
	std::shared_ptr<block_type> data;
	std::size_t offset;

private:
	surface_storage(std::shared_ptr<block_type> data,
					std::size_t offset) : data(data), offset(offset) {}

public:
	surface_storage(std::size_t blocks)
		: data(new block_type[blocks],
			   std::default_delete<block_type[]>()),
		offset(0)
	{}

	template <bool b = is_const, typename = typename std::enable_if_t<b>>
	surface_storage(surface_storage<block_type, false> &&o) noexcept : data(std::move(o.data)), offset(o.offset) {}

	surface_storage(surface_storage&&) = default;
	surface_storage(const surface_storage&) = default;
	surface_storage &operator=(surface_storage&&) = default;
	surface_storage &operator=(const surface_storage&) = default;

	auto view(std::size_t offset) const { return surface_storage(data, offset); }

	template <bool b = is_const, typename = typename std::enable_if_t<!b>>
	block_type* get() { return data.get() + offset; }
	const block_type* get() const { return data.get() + offset; }
};

template<gl::format format, gl::image_type image_type>
class surface_base {
public:
	using traits = gl::format_traits<format>;
	static constexpr auto dimensions = gl::image_dimensions_v<image_type>;
	using extent_type = typename gl::image_extent_type<dimensions>::type;

private:
	extent_type _extent;
	std::size_t _levels;
	std::size_t _layers;

protected:
	surface_base(const extent_type &extent,
				 std::size_t levels,
				 std::size_t layers)
		: _extent(extent), _levels(levels), _layers(layers)
	{
		assert(layers > 0);
		assert(levels > 0);
	}

public:
	auto extent(std::size_t level = 0) const {
		return glm::max(extent_type(static_cast<typename extent_type::value_type>(1)),
						_extent >> static_cast<typename extent_type::value_type>(level));
	}
	auto levels() const {
		return _levels;
	}
	auto layers() const {
		return _layers;
	}

	auto block_extent() const {
		return extent_type(static_cast<typename extent_type::value_type>(1));
	}

	auto blocks(std::size_t level) const {
		assert(level < levels());

		std::size_t b = 1;
		for (std::remove_cv_t<decltype(dimensions)> i = 0; i < dimensions; ++i)
			b *= extent(level)[i];

		return b;
	}

	auto blocks_layer() const {
		std::size_t b = 0;
		for (std::size_t l = 0; l < levels(); ++l)
			b += blocks(l);

		return b;
	}

	auto offset_blocks(std::size_t layer, std::size_t level) const {
		std::size_t level_offset = 0;
		for (std::size_t l = 0; l < level; ++l)
			level_offset += blocks(l) * block_bytes();

		return blocks_layer() * layer + level_offset;
	}

	auto block_bytes() const {
		return sizeof(traits::element_type);
	}

	auto bytes() const {
		return block_bytes() * blocks_layer() * layers();
	}
};

}

}
}
