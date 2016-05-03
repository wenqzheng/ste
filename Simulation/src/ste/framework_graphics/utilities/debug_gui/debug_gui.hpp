// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include "stdafx.hpp"
#include "gpu_dispatchable.hpp"

#include "StEngineControl.hpp"
#include "profiler.hpp"

#include "signal.hpp"

#include <memory>

namespace StE {
namespace Graphics {

class debug_gui : public gpu_dispatchable {
	using Base = gpu_dispatchable;

private:
	using hid_pointer_button_signal_connection_type = StEngineControl::hid_pointer_button_signal_type::connection_type;
	using hid_scroll_signal_connection_type = StEngineControl::hid_scroll_signal_type::connection_type;
	using hid_keyboard_signal_connection_type = StEngineControl::hid_keyboard_signal_type::connection_type;

private:
	const StEngineControl &ctx;
	profiler *prof;

private:
	std::shared_ptr<hid_pointer_button_signal_connection_type> hid_pointer_button_signal;
	std::shared_ptr<hid_scroll_signal_connection_type> hid_scroll_signal;
	std::shared_ptr<hid_keyboard_signal_connection_type> hid_keyboard_signal;

public:
	debug_gui(const StEngineControl &ctx, profiler *prof = nullptr);
	~debug_gui() noexcept;

protected:
	void set_context_state() const override final {}
	void dispatch() const override final;
};

}
}
