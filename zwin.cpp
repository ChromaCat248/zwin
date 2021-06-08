// ZWin
// Mouse-driven Tiling Window Manager
// By ChromaCat248

#include <cstdlib>
#include <stdio.h>
#include <sys/time.h>
#include <X11/Xlib.h>
#include <unordered_map>
#include <memory>

using ::std::max;
using ::std::string;
using ::std::unique_ptr;

static int xerrorstart(Display *dpy, XErrorEvent *ee);
static int xerror(Display *dpy, XErrorEvent *ee);
bool wm_detected = false;
std::unordered_map<Window, Window> clients;
Display* display;
Window root;

int checkOtherWM(Display* display, XErrorEvent* errorEvent) {
	if (static_cast<int>(errorEvent->error_code) == BadAccess) {
		wm_detected = true;
	}
	
	return 0;
};

void frame(Window window, bool was_created_before_wm) {
	
	// visual properties of frame
	const unsigned int borderWidth = 1;
	const unsigned long borderColor = 0x888888;
	const unsigned long backgroundColor = 0xaaaaaa;
	
	// retrieve attributes of window to frame
	XWindowAttributes attrs;
	
	// only frame pre-existing windows if they are visible and don't set override_redirect
	if (was_created_before_wm &&
		(attrs.override_redirect || attrs.map_state != IsViewable)
	   ) {
		return;
	}
	
	// Create frame
	const Window frame = XCreateSimpleWindow(
		display,
		root,
		attrs.x,
		attrs.y,
		attrs.width,
		attrs.height,
		borderWidth,
		borderColor,
		backgroundColor
	);
	
	// Select events on frame
	XSelectInput(
		display,
		frame,
		SubstructureRedirectMask | SubstructureNotifyMask
	);
	
	// Add client to save set
	XAddToSaveSet(display, window);
	
	// Reparent client window
	XReparentWindow(
		display,
		window,
		frame,
		0, 0
	);
	
	// Map frame
	XMapWindow(display, frame);
	
	// Save frame
	clients[window] = frame;
	
	printf("Framed a window\n");
};

void unframe(Window window) {
	const Window frame = clients[window];
	
	XUnmapWindow(display, frame);
	
	XReparentWindow(
		display,
		window,
		root,
		0, 0
	);
	
	XRemoveFromSaveSet(display, frame);
	
	clients.erase(window);
	
	printf("Unframed a window\n");
};



void onConfigureRequest(const XConfigureRequestEvent& event) {
	XWindowChanges changes;
	changes.x = event.x;
	changes.y = event.y;
	changes.width = event.width;
	changes.height = event.height;
	changes.border_width = event.border_width;
	changes.sibling = event.above;
	changes.stack_mode = event.detail;
	
	XConfigureWindow(display, event.window, event.value_mask, &changes);
};

void onConfigureNotify(const XConfigureEvent& event) {
	// ignore
};

void onReparentNotify(const XReparentEvent& event) {
	// ignore
};

void onMapRequest(const XMapRequestEvent& event) {
	frame(event.window, false);
	XMapWindow(display, clients[event.window]);
	printf("Mapped a window\n");
};

void onMapNotify(const XMapEvent& event) {
	// ignore
};

void onUnmapNotify(const XUnmapEvent& event) {
	// ignore
};

void onDestroyNotify(const XDestroyWindowEvent& event) {
	// ignore
}



int main(int argc, const char** argv) {
	
	printf("ZWin: Starting ZWin\n");
	
	// Setup
	display = XOpenDisplay(nullptr);
	if (display == nullptr) {
		printf("ZWin: Failed to open X display\n");
		return 0;
	};
	XGrabServer(display);
	root = DefaultRootWindow(display);
	
	// Refuse to run if another window manager is running
	/*wm_detected = false;
	XSetErrorHandler(checkOtherWM);
	if (wm_detected = true) {
		printf("ZWin: Another window manager was detected on this display. ZWin cannot continue running.\n");
		return 0;
	};*/
	
	XSelectInput(display, root, SubstructureRedirectMask | SubstructureNotifyMask);
	XSync(display, false);
	
	// frame existing windows
	Window returned_root, returned_parent;
	Window* toplevel_windows;
	unsigned int num_toplevel_windows;
	XQueryTree(
		display,
		root,
		&returned_root,
		&returned_parent,
		&toplevel_windows,
		&num_toplevel_windows
	);
	
	printf("num_toplevel_windows = %d\n", num_toplevel_windows);
	for (unsigned int i = 0; i < num_toplevel_windows; i = i + 1) {
		printf("start of for loop\n");
		frame(toplevel_windows[i], true);
		printf("end of for loop\n");
	}
	
	XFree(toplevel_windows);
	XUngrabServer(display);
	
	// Event loop
	while (true) {
		
		// Get next event
		XEvent event;
		XNextEvent(display, &event);
		printf("ZWin: Received event: ");
		
		// Dispatch event
		switch (event.type) {
			case CreateNotify: //ignore
				printf("CreateNotify\n");
				break;
			case ConfigureRequest:
				printf("ConfigureRequest\n");
				onConfigureRequest(event.xconfigurerequest);
				break;
			case ConfigureNotify: //ignore
				printf("ConfigureNotify\n");
				onConfigureNotify(event.xconfigure);
				break;
			case ReparentNotify: //ignore
				printf("ReparentNotify\n");
				onReparentNotify(event.xreparent);
				break;
			case MapRequest:
				printf("MapRequest\n");
				onMapRequest(event.xmaprequest);
				break;
			case MapNotify: //ignore
				printf("MapNotify\n");
				onMapNotify(event.xmap);
				break;
			case UnmapNotify: //ignore
				printf("UnmapNotify\n");
				onUnmapNotify(event.xunmap);
				break;
			case DestroyNotify:
				printf("DestroyNotify\n");
				onDestroyNotify(event.xdestroywindow);
				break;
			default:
				printf("Unrecognized (ignoring)\n");
				break;
		};
	};
	
	// Exit out of window manager
	printf("ZWin: Exiting\n");
	XUngrabServer(display);
	XCloseDisplay(display);
	return 0;
	
};
