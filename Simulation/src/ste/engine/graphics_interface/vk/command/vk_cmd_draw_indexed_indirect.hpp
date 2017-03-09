//	StE
// � Shlomi Steinberg 2015-2016

#pragma once

#include <vulkan/vulkan.h>
#include <vk_command.hpp>
#include <vk_buffer_base.hpp>

namespace StE {
namespace GL {

class vk_cmd_draw_indexed_indirect : public vk_command {
private:
	const vk_buffer_base &buffer;
	std::uint32_t offset;
	std::uint32_t draw_count;
	std::uint32_t stride;

public:
	vk_cmd_draw_indexed_indirect(const vk_buffer_base &buffer,
						 std::uint32_t offset,
						 std::uint32_t draw_count,
						 std::uint32_t stride)
		: buffer(buffer),
		offset(offset),
		draw_count(draw_count),
		stride(stride)
	{}
	virtual ~vk_cmd_draw_indexed_indirect() noexcept {}

private:
	void operator()(const vk_command_buffer &command_buffer) const override final {
		vkCmdDrawIndexedIndirect(command_buffer, buffer, offset, draw_count, stride);
	}
};

}
}
