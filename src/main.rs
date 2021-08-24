// ZWin
// Mouse-driven tiling window manager
// by ChromaCat248

#![allow(non_upper_case_globals)]

#[path = "../config.rs"] mod config;
mod event_handling;

use x11;
use x11::xlib::*;
use std::mem::MaybeUninit;
use core::ffi::c_void;

static mut DISPLAY : *mut Display = 0 as *mut _XDisplay;
static mut ROOTWIN : Window = 0 as Window;

// cursors
static mut CURSOR_DEFAULT : Cursor = 0 as u64;
static mut CURSOR_DRAG : Cursor = 0 as u64;

// error handler
unsafe extern "C" fn on_x_error(_display: *mut Display, _event : *mut XErrorEvent) -> i32 {
	println!("ZWin: Recieved X error");
	return 0;
}

fn main() {
	println!("ZWin: Starting");

	unsafe {
		DISPLAY = XOpenDisplay(0x0 as *const i8);
		if DISPLAY == 0 as *mut _XDisplay { {
			println!("ZWin: Failed to open X display");
			return;
		} }
		ROOTWIN = XDefaultRootWindow(DISPLAY) as u64;

		XSelectInput(DISPLAY, ROOTWIN,
						SubstructureRedirectMask |
						SubstructureNotifyMask);

		// set error handler
		XSetErrorHandler(Some(on_x_error));

		// code to detect a running window manager should go here

		XSync(DISPLAY, 0);

		// frame existing windows
		XGrabServer(DISPLAY);

		let mut returned_root : Window = 0;
		let mut returned_parent : Window = 0;
		let mut toplevel_windows : *mut Window = &mut 0;
		let mut num_toplevel_windows : u32 = 0;

		XQueryTree(
			DISPLAY,
			ROOTWIN,
			&mut returned_root as *mut Window,
			&mut returned_parent as *mut Window,
			&mut toplevel_windows as *mut *mut Window,
			&mut num_toplevel_windows as *mut u32
		);
		// I will keep an eye out for issues regarding this because it seems janky,
		// please open an issue if ZWin crashes when windows are open on startup.

		XFree(toplevel_windows as *mut c_void);
		XUngrabServer(DISPLAY);

		// set cursor
		CURSOR_DEFAULT = XCreateFontCursor(DISPLAY, 2 /*XC_arrow*/);
		CURSOR_DRAG = XCreateFontCursor(DISPLAY, 52/*XC_fleur*/);

		XDefineCursor(DISPLAY, ROOTWIN, CURSOR_DEFAULT);
	}

	// run startup function
	let rc_ret:i32 = config::rc();
	if rc_ret != 0 {
		println!("ZWin: RC encountered error {}", rc_ret);
	}

	// event loop
	loop { unsafe {
		// get next event
		let mut event_uninit = MaybeUninit::uninit();
		XNextEvent(DISPLAY, event_uninit.as_mut_ptr());

		event_uninit.assume_init();

		let event : XEvent = *event_uninit.as_mut_ptr();

		// dispatch event
		match event.type_ {
			CreateNotify => {
				event_handling::on_create_notify(event.create_window);
			}
			ConfigureRequest => {
				event_handling::on_configure_request(event.configure_request);
			}
			ConfigureNotify => {
				event_handling::on_configure_notify(event.configure);
			}
			ReparentNotify => {
				event_handling::on_reparent_notify(event.reparent);
			}
			MapRequest => {
				event_handling::on_map_request(event.map_request);
			}
			MapNotify => {
				event_handling::on_map_notify(event.map);
			}
			UnmapNotify => {
				event_handling::on_unmap_notify(event.unmap);
			}
			DestroyNotify => {
				event_handling::on_destroy_notify(event.destroy_window);
			}
			ButtonPress => {
				event_handling::on_button_press(event.button);
			}
			ButtonRelease => {
				event_handling::on_button_release(event.button);
			}
			MotionNotify => {
				event_handling::on_motion_notify(event.motion);
			}
			PropertyNotify => {
				event_handling::on_property_notify(event.property);
			}

			// unrecognized event
			_ => {
				println!("ZWin: Ignoring unrecognized event #{}", event.type_);
			}
		}
	} }
}
