// StE
// � Shlomi Steinberg, 2015

#pragma once

#include "buffer_object.h"
#include "VertexBufferDescriptor.h"

#include <type_traits>
#include <vector>

namespace StE {
namespace LLR {

class VertexBufferObjectGeneric {
public:
	virtual void bind() const = 0;
	virtual void unbind() const = 0;
	virtual const VertexBufferDescriptor *data_descriptor() const = 0;
};

template <typename T, class D, BufferUsage::buffer_usage U>
class vertex_buffer_attrib_binder {
private:
	template <typename, class, BufferUsage::buffer_usage>
	friend class VertexBufferObject;
	friend class VertexArrayObject;

	int binding_index;
	std::size_t offset, size;
	const VertexBufferObject<T, D, U> *vbo;

	vertex_buffer_attrib_binder(int index, const VertexBufferObject<T, D, U> *vbo, std::size_t offset, std::size_t size) :
		binding_index(index), vbo(vbo), offset(offset), size(size) {}
	vertex_buffer_attrib_binder(vertex_buffer_attrib_binder &&m) = default;
	vertex_buffer_attrib_binder& operator=(vertex_buffer_attrib_binder &&m) = default;
	vertex_buffer_attrib_binder(const vertex_buffer_attrib_binder &m) = delete;
	vertex_buffer_attrib_binder& operator=(const vertex_buffer_attrib_binder &m) = delete;

public:
	~vertex_buffer_attrib_binder() {}
};

template <typename Type, class Descriptor, BufferUsage::buffer_usage U = BufferUsage::BufferUsageNone>
class VertexBufferObject : public buffer_object<Type, U>, public VertexBufferObjectGeneric {
private:
	using buffer_object<Type, U>::bind;
	using buffer_object<Type, U>::unbind;

	static_assert(std::is_base_of<VertexBufferDescriptor, Descriptor>::value, "Descriptor must inherit from VertexBufferDescriptor");

private:
	Descriptor descriptor;

public:
	VertexBufferObject(VertexBufferObject &&m) = default;
	VertexBufferObject& operator=(VertexBufferObject &&m) = default;

	VertexBufferObject(std::size_t size) : buffer_object(size) {}
	VertexBufferObject(std::size_t size, const T *data) : buffer_object(size, data) {}
	VertexBufferObject(const std::vector<T> &data) : buffer_object(data.size(), &data[0]) {}

	virtual const VertexBufferDescriptor *data_descriptor() const override { return &descriptor; }

	vertex_buffer_attrib_binder<T, Descriptor, U> operator[](int index) { return vertex_buffer_attrib_binder<T, Descriptor, U>(index, this, 0, sizeof(T)); }
	vertex_buffer_attrib_binder<T, Descriptor, U> binder(int index, std::size_t offset) { return vertex_buffer_attrib_binder<T, Descriptor, U>(index, this, offset, sizeof(T)); }

	void bind() const final override { Binder::bind(id, GL_ARRAY_BUFFER); };
	void unbind() const final override { Binder::unbind(GL_ARRAY_BUFFER); };

	llr_resource_type resource_type() const override { return llr_resource_type::LLRVertexBufferObject; }
};

}
}
