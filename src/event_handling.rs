// ZWin's X11 event handler

use x11;
use x11::xlib::*;

pub fn on_create_notify (event: XCreateWindowEvent) {
	println!("ZWin: Recieved event #{} (created)", event.type_);
}

pub fn on_configure_request (event: XConfigureRequestEvent) {
	println!("ZWin: Recieved event #{} (configure request)", event.type_);
}

pub fn on_configure_notify (event: XConfigureEvent) {
	println!("ZWin: Recieved event #{} (configured)", event.type_);
}

pub fn on_reparent_notify (event: XReparentEvent) {
	println!("ZWin: Recieved event #{} (reparented)", event.type_);
}

pub fn on_map_request (event: XMapRequestEvent) {
	println!("ZWin: Recieved event #{} (map request)", event.type_);
}

pub fn on_map_notify (event: XMapEvent) {
	println!("ZWin: Recieved event #{} (mapped)", event.type_);
}

pub fn on_unmap_notify (event: XUnmapEvent) {
	println!("ZWin: Recieved event #{} (unmapped)", event.type_);
}

pub fn on_destroy_notify (event: XDestroyWindowEvent) {
	println!("ZWin: Recieved event #{} (destroyed)", event.type_);
}

pub fn on_button_press (event: XButtonEvent) {
	println!("ZWin: Recieved event #{} (button pressed)", event.type_);
}

pub fn on_button_release (event: XButtonEvent) {
	println!("ZWin: Recieved event #{} (button released)", event.type_);
}

pub fn on_motion_notify (event: XMotionEvent) {
	println!("ZWin: Recieved event #{} (motion)", event.type_);
}

pub fn on_property_notify (event: XPropertyEvent) {
	println!("ZWin: Recieved event #{} (property changed)", event.type_);
}
