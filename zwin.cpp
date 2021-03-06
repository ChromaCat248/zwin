// ZWin
// Mouse-driven tiling window manager
// by ChromaCat248

#include <cstdlib>
#include <string.h>
#include <stdio.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/cursorfont.h>
#include <map>

// config file
#include "config.h"

struct winInfo {
	Window* frame;
	Window* titlebar;
	GC gc;
	bool framed; // set to false on panel/background windows
	int edge; // 0: background, 1: top, 2: right, 3: bottom, 4: left
	bool hasTitlebar; // set to false on gtk windows
	bool tiled;
	bool show;
	const char* title;
};

struct tile {
	bool isRootTile;
	bool empty; // only for root tiles
	int screen; // only for root tiles
	int splits;
	std::map<unsigned int, tile*>* tiles;
	double sizePortion; // will be a number between 0 and 1 that represents the tile's share of its parent tile's size
	Window* content; // never used on split tiles
	tile* parentTile; // never used on root tiles
};

static int xerrorstart(Display *dpy, XErrorEvent *ee);
static int xerror(Display *dpy, XErrorEvent *ee);
bool wm_detected = false;
bool after_wm_detection = false;
std::map<Window, Window> frames;
std::map<Window, Window> titlebars;
std::map<Window, winInfo> clients;
std::map<unsigned int, tile*> rootTiles;
Display* display;
Window root;

// cursors
Cursor cursor_default;
Cursor cursor_drag;

// some variables relating to dragging
int lastClickX = 0;
int lastClickY = 0;
int dragOffsetX = 0;
int dragOffsetY = 0;
bool dragging = false; // not needed for the dragging process itself, but for effects related to dragging

// set to true when the wm is meant to exit
bool exitTrigger = false;

// window management functions

GC create_gc(Display* dpy, Window window, bool inverse_colors) {
	GC gc;
	unsigned long valuemask = 0;
	
	XGCValues values;
	unsigned int line_width = 2;
	int line_style = LineSolid;
	int cap_style = CapButt;
	int join_style = JoinBevel;
	int fill_style = FillSolid;
	int screen = DefaultScreen(dpy);
	
	gc = XCreateGC(dpy, window, valuemask, &values);
	
	if (inverse_colors) {
		XSetForeground(dpy, gc, WhitePixel(dpy, screen));
		XSetBackground(dpy, gc, BlackPixel(dpy, screen));
	} else {
		XSetForeground(dpy, gc, BlackPixel(dpy, screen));
		XSetBackground(dpy, gc, WhitePixel(dpy, screen));
	};
	
	XSetLineAttributes(display, gc, line_width, line_style, cap_style, join_style);
	XSetFillStyle(display, gc, fill_style);
	
	return gc;
};

Window getParent(Window window) {
	Window root_placeholder;
	Window parent;
	Window* children_placeholder;
	unsigned int nchildren_placeholder;
	XQueryTree(
		display,
		window,
		&root_placeholder,
		&parent,
		&children_placeholder,
		&nchildren_placeholder
	);
	
	return parent;
};

Window* getChildren(Window window) {
	Window root_placeholder;
	Window parent_placeholder;
	Window* children;
	unsigned int nchildren_placeholder;
	XQueryTree(
		display,
		window,
		&root_placeholder,
		&parent_placeholder,
		&children,
		&nchildren_placeholder
	);
	
	return children;
};

void createRootTile(int screen) {
	// create the root tile
	tile rt;
	rt.isRootTile = true;
	rt.screen = screen;
	rt.splits = 0;
	rt.sizePortion = 1;
	
	// append the new root tile to the list of root tiles
	unsigned int index = 0;
	while (rootTiles.count(index)) {
		index++;
	};
	rootTiles[index] = &rt;
};

void frame(Window window, bool was_created_before_wm) {
	
	// retrieve attributes of window to frame
	XWindowAttributes attrs;
	XGetWindowAttributes(display, window, &attrs);
	printf("%d\n", attrs.c_class);
	
	// only frame pre-existing windows if they are visible and don't set override_redirect
	if (was_created_before_wm && (attrs.override_redirect || attrs.map_state != IsViewable)) {
		return;
	}
	
	// create frame
	const Window frame = XCreateSimpleWindow(
		display,
		root,
		attrs.x,
		attrs.y,
		attrs.width,
		attrs.height + barHeight,
		borderWidth,
		borderColor,
		backgroundColor
	);
	
	
	// select events on frame
	XSelectInput(
		display,
		frame,
		SubstructureRedirectMask | SubstructureNotifyMask
	);
	
	XSelectInput(
		display,
		window,
		PropertyChangeMask
	);
	
	// create titlebar
	const Window bar = XCreateSimpleWindow(
		display,
		frame,
		attrs.x,
		attrs.y,
		attrs.width,
		barHeight,
		0,
		barColor,
		barColor
	);
	
	// add client to save set
	XAddToSaveSet(display, window);
	
	// reparent client window
	XReparentWindow(
		display,
		window,
		frame,
		0, barHeight
	);
	
	// map frame
	XMapWindow(display, frame);
	XMapWindow(display, bar);
	
	clients[window].gc = create_gc(display, bar, true);

	// tile window
	
	// save frame
	frames[window] = frame;
	clients[window].frame = &frames[window];
	
	// save titlebar
	titlebars[window] = bar;
	clients[window].titlebar = &titlebars[window];
	
	
	XGrabButton(
		display,
		Button1,
		None,
		bar,
		false,
		ButtonPressMask | ButtonReleaseMask | ButtonMotionMask,
		GrabModeAsync,
		GrabModeAsync,
		None,
		None);

	//printf("ZWin: Framed a window\n");
};

void unframe(Window window) {
	if (!frames.count(window)) {
		return;
	};	
	
	const Window frame = frames[window];
	
	XUnmapWindow(display, frame);
	
	XReparentWindow(
		display,
		window,
		root,
		0, 0
	);
	
	XRemoveFromSaveSet(display, frame);
	
	frames.erase(window);
	
	//printf("ZWin: Unframed a window\n");
};



void onCreateNotify(const XCreateWindowEvent& event) {
	
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
	
	XFetchName(display, event.window, (char**)&clients[event.window].title);
};

void onConfigureNotify(const XConfigureEvent& event) {
	// ignore
};

void onReparentNotify(const XReparentEvent& event) {
	// ignore
};

void onMapRequest(const XMapRequestEvent& event) {
	frame(event.window, false);
	XMapWindow(display, event.window);
};

void onMapNotify(const XMapEvent& event) {
	XFetchName(display, event.window, (char**)&clients[event.window].title);
	XUnmapWindow(display, titlebars[event.window]);
	XMapWindow(display, titlebars[event.window]);
};

void onUnmapNotify(const XUnmapEvent& event) {
	// if the window titles aren't refreshed they'll be wiped away by becoming unviewable
	// (e.g. disappearing behind a window, going off-screen)
	for (const auto [client, frame] : frames) {
		if (client != event.window) {
			Window bar = titlebars[client];
			XFetchName(display, client, (char**)&clients[client].title);
			XDrawString(
				display,
				bar,
				clients[client].gc,
				30, 17,
				clients[client].title,
				strlen(clients[client].title)
			);
		}
	};
	
	if (!frames.count(event.window)) {
		return;
	}
	if (event.event == root) {
		return;
	}
	unframe(event.window);
};

void onDestroyNotify(const XDestroyWindowEvent& event) {
	clients.erase(event.window);
}


void onButtonPress(const XButtonEvent& event) {
	XRaiseWindow(display, getParent(event.window));
	
	int rootx;
	int rooty;
	int winx;
	int winy;
	unsigned int m;
	Window frame = getParent(event.window);
	
	XQueryPointer(
		display,
		root,
		&root,
		&frame,
		&rootx,
		&rooty,
		&winx,
		&winy,
		&m
	);
	
	// 
	XWindowAttributes frame_attrs;
	XGetWindowAttributes(display, frame, &frame_attrs);
	winx = rootx - frame_attrs.x;
	winy = rooty - frame_attrs.y;
	
	lastClickX = rootx;
	lastClickY = rooty;
	dragOffsetX = winx;
	dragOffsetY = winy;
}

void onButtonRelease(const XButtonEvent& event) {
	
	// register titlebar button presses if not dragging
	if (dragging == false) {
		//printf("click\n");
	} else {
		//printf("drag\n");
	}
	
	dragging = false;
	XDefineCursor(display, root, cursor_default);
}

void onMotionNotify(const XMotionEvent& event) {
	dragging = true;
	XDefineCursor(display, root, cursor_drag);
	
	int rootx;
	int rooty;
	int winx;
	int winy;
	unsigned int m;
	Window window = event.window;
	
	XQueryPointer(
		display,
		root,
		&root,
		&window,
		&rootx,
		&rooty,
		&winx,
		&winy,
		&m
	);
	
	Window frame = getParent(event.window);
	
	int dragx = rootx - dragOffsetX;
	int dragy = rooty - dragOffsetY;
	XMoveWindow(
		display,
		frame,
		dragx, dragy
	);
	
	for (const auto [client, frame] : frames) {
		Window bar = titlebars[client];
		XDrawString(
			display,
			bar,
			clients[client].gc,
			30, 17,
			clients[client].title,
			strlen(clients[client].title)
		);
		
	};
};

void onPropertyNotify(const XPropertyEvent& event) {
	XFetchName(display, event.window, (char**)&clients[event.window].title);
	XUnmapWindow(display, titlebars[event.window]);
	XMapWindow(display, titlebars[event.window]);
};


int onXError(Display* display, XErrorEvent* e) {
	if (e->error_code == BadAccess) {
		wm_detected = true;
		return 0;
	};
	
	const int MAX_ERROR_TEXT_LENGTH = 1024;
	char error_text[MAX_ERROR_TEXT_LENGTH];
	XGetErrorText(display, e->error_code, error_text, sizeof(error_text));
	
	printf("ZWin: Received X error\n");
	printf("	Request: %d\n", int(e->request_code));
	printf("	Error code: %d\n", int(e->error_code));
	printf("	Error text: %s\n", error_text);
	printf("	Resource ID: %lu\n", e->resourceid);
	
	return 0;
}

int main(int argc, const char** argv) {
	
	printf("ZWin: Starting ZWin\n");
	
	// open display
	display = XOpenDisplay(0x0);
	if (display == 0x0) {
		printf("ZWin: Failed to open X display\n");
		return 0;
	};
	
	// set error handler
	XSetErrorHandler(onXError);
	
	root = DefaultRootWindow(display);
	XSelectInput(display, root, SubstructureRedirectMask | SubstructureNotifyMask);
	
	if (wm_detected == true) {
		printf("ZWin: Cannot run alongside another window manager\n");
		return 0;
	};
	after_wm_detection = true;
	
	XSync(display, false);
	
	XGrabServer(display);
	
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
	
	for (unsigned int i = 0; i < num_toplevel_windows; i = i + 1) {
		frame(toplevel_windows[i], true);
	}
	
	XFree(toplevel_windows);
	XUngrabServer(display);
	
	cursor_default = XCreateFontCursor(display, XC_arrow);
	cursor_drag = XCreateFontCursor(display, XC_fleur);
	
	// set cursor
	XDefineCursor(display, root, cursor_default);
	
	// root tile
	createRootTile(0);

    // rc
    int rcOut = rc();
    if(rcOut) {
        printf("ZWin: RC returned error %d\n", rcOut);
        if (exitOnRCError) {
            return 0;
        };
    };
	
	// event loop
	while (exitTrigger == false) {
		
		// get next event
		XEvent event;
		XNextEvent(display, &event);
		
		// dispatch event
		switch (event.type) {
			case CreateNotify: //ignore
				printf("CreateNotify\n");
				onCreateNotify(event.xcreatewindow);
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
			case UnmapNotify:
				printf("UnmapNotify\n");
				onUnmapNotify(event.xunmap);
				break;
			case DestroyNotify:
				printf("DestroyNotify\n");
				onDestroyNotify(event.xdestroywindow);
				break;
			case ButtonPress:
				printf("ButtonPress\n");
				onButtonPress(event.xbutton);
				break;
			case ButtonRelease:
				printf("ButtonRelease\n");
				onButtonRelease(event.xbutton);
				break;
			case MotionNotify:
				printf("MotionNotify\n");
				onMotionNotify(event.xmotion);
				break;
			case PropertyNotify:
				printf("PropertyNotify");
				onPropertyNotify(event.xproperty);
				break;
			default:
				printf("ZWin: Ignoring unrecognized event (type: %d)\n", event.type);
				break;
		};
	};
	
	// exit out of window manager
	XCloseDisplay(display);
	printf("ZWin: Exiting, have a nice day\n");
	return 0;
	
};
