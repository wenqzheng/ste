//	StE
// � Shlomi Steinberg 2015-2016

#pragma once

#include <stdafx.hpp>

#include <vulkan/vulkan.h>
#include <vk_logical_device.hpp>
#include <vk_descriptor_set.hpp>
#include <vk_descriptor_pool.hpp>

#include <allow_class_decay.hpp>

namespace StE {
namespace GL {

class vk_unique_descriptor_set : public allow_class_decay<vk_unique_descriptor_set, vk_descriptor_set> {
private:
	using binding_t = vk_descriptor_set_layout_binding;
	using binding_set_t = std::vector<vk_descriptor_set_layout_binding>;

private:
	vk_descriptor_pool pool;
	vk_descriptor_set_layout layout;
	vk_descriptor_set set;

public:
	vk_unique_descriptor_set(const vk_logical_device &device,
							 const binding_set_t &bindings)
		: pool(device, 1, bindings, false),
		layout(device, bindings),
		set(pool.allocate_descriptor_set({ layout }))
	{}

	~vk_unique_descriptor_set() noexcept {}

	vk_unique_descriptor_set(vk_unique_descriptor_set &&) = default;
	vk_unique_descriptor_set &operator=(vk_unique_descriptor_set &&) = default;
	vk_unique_descriptor_set(const vk_unique_descriptor_set &) = delete;
	vk_unique_descriptor_set &operator=(const vk_unique_descriptor_set &) = delete;

	auto& get() const { return set; }
	auto& get() { return set; }

	auto& get_layout() const { return layout; }
};

}
}
