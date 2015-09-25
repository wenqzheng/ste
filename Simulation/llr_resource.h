// StE
// � Shlomi Steinberg, 2015

#pragma once

#include "llr_resource_type.h"

namespace StE {
namespace LLR {

class llr_resource_stub_allocator {
public:
	static bool is_valid(unsigned int id) { return !!id; }
	static int allocate() { return 0; }
	static void deallocate(unsigned int &id) { id = 0; }
};

class GenericResource {
protected:
	unsigned int id;

public:
	virtual llr_resource_type resource_type() const = 0;

	int get_resource_id() const { return id; }
	virtual bool is_valid() const = 0;
};

template <class A>
class llr_resource : virtual public GenericResource {
protected:
	using Allocator = A;

	llr_resource() { this->id = Allocator::allocate(); }

public:
	llr_resource(llr_resource &&m) = default;
	llr_resource(const llr_resource &c) = delete;
	llr_resource& operator=(llr_resource &&m) = default;
	llr_resource& operator=(const llr_resource &c) = delete;

	~llr_resource() { if (Allocator::is_valid(id)) Allocator::deallocate(id); }

	bool is_valid() const { return Allocator::is_valid(id); }
};

}
}
