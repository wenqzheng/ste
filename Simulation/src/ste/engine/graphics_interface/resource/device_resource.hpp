//	StE
// � Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <ste_context.hpp>
#include <device_resource_queue_ownership.hpp>
#include <ste_resource_traits.hpp>

#include <vk_resource.hpp>
#include <device_resource_memory_allocator.hpp>

#include <functional>

namespace StE {
namespace GL {

template <typename T, class allocation_policy>
class device_resource : ste_resource_deferred_create_trait {
	static_assert(std::is_base_of<vk_resource, T>::value,
				  "T must be a vk_resource derived type");

protected:
	struct ctor {};

public:
	using resource_t = T;
	using allocation_t = device_memory_heap::allocation_type;

private:
	T resource;
	allocation_t allocation;

private:
	void allocate_memory() {
		allocation = device_resource_memory_allocator<allocation_policy>()(ctx.device_memory_allocator(),
																		   resource);
	}

public:
	const ste_context &ctx;
	device_resource_queue_ownership queue_ownership;

protected:
	device_resource(ctor,
					const ste_context &ctx,
					const device_resource_queue_ownership::resource_queue_selector_t &selector,
					T &&resource)
		: resource(std::move(resource)),
		ctx(ctx),
		queue_ownership(ctx, selector)
	{
		allocate_memory();
	}
	device_resource(ctor,
					const ste_context &ctx,
					const device_resource_queue_ownership::queue_index_t &queue_index,
					T &&resource)
		: resource(std::move(resource)),
		ctx(ctx),
		queue_ownership(queue_index)
	{
		allocate_memory();
	}

public:
	template <typename ... Args>
	device_resource(const ste_context &ctx,
					const device_resource_queue_ownership::resource_queue_selector_t &initial_queue_selector,
					Args&&... args)
		: device_resource(ctor(),
						  ctx,
						  initial_queue_selector,
						  T(ctx.device().logical_device(), std::forward<Args>(args)...))
	{}
	template <typename ... Args>
	device_resource(const ste_context &ctx,
					const device_resource_queue_ownership::queue_index_t &initial_queue_index,
					Args&&... args)
		: device_resource(ctor(),
						  ctx,
						  initial_queue_index,
						  T(ctx.device().logical_device(), std::forward<Args>(args)...))
	{}
	virtual ~device_resource() noexcept {}

	device_resource(device_resource&&) = default;
	device_resource &operator=(device_resource&&) = default;

	auto& get_underlying_memory() { return *this->allocation.get_memory(); }
	auto& get_underlying_memory() const { return *this->allocation.get_memory(); }
	bool has_private_underlying_memory() const { return allocation.is_private_allocation(); }

	operator T&() { return get(); }
	operator const T&() const { return get(); }

	T& get() { return resource; }
	const T& get() const { return resource; }

	T* operator->() { return &get(); }
	const T* operator->() const { return &get(); }
	T& operator*() { return get(); }
	const T& operator*() const { return get(); }
};

}
}
