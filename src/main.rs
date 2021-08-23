// ZWin
// Mouse-driven tiling window manager
// by ChromaCat248

#[path = "../config.rs"] mod config;

use x11;
use x11::xlib::*;

static mut DISPLAY : *mut Display = 0 as *mut _XDisplay;
static mut ROOTWIN : Window = 0 as Window;

fn on_x_error(_display: *mut Display, _event : *const XErrorEvent) {
	println!("ZWin: Recieved X error");
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

		XSelectInput(DISPLAY, ROOTWIN, SubstructureRedirectMask | SubstructureNotifyMask);

		// code to detect a running window manager should go here

		XSync(DISPLAY, 0);

		// frame existing windows
		XGrabServer(DISPLAY);
		XUngrabServer(DISPLAY);

	}

	// run startup function
	let rc_ret:i32 = config::rc();
	if rc_ret != 0 {
		println!("ZWin: RC encountered error {}", rc_ret);
	}

	// event loop
	loop {
		let mut event : *mut XEvent = 0 as *mut XEvent;
		unsafe {
			XNextEvent(DISPLAY, event);
			/*match (*event).type_ {
				CreateNotify => {
					println!("sdfg");
				}
				_ => {
					println!("sdfgsdfg");
				}
			}*/
		}
		println!("dsfgsdfg");
	}
}
