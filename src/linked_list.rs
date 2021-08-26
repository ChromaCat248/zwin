// Doubly-linked list
// by ChromaCat248

use std::mem::*;

struct Node<T> {
	value: T,
	next: Option<*mut Node<T>>,
	prev: Option<*mut Node<T>>,
}

impl<T> Node<T> {
	fn link_forth(&mut self, target: &mut Node<T>) {
		if (*self).next.is_some() {
			unsafe { (*(*self).next.unwrap()).prev = Some(target) };
			target.next = self.next;
		};
		if (*target).prev.is_some() {
			unsafe { (*(*target).prev.unwrap()).next = Some(self) };
			self.prev = target.prev;
		};

		(*self).next = Some(target);
		(*target).prev = Some(self);
	}

	fn link_back(&mut self, target: &mut Node<T>) {
		(*target).link_forth(self);
	}

	fn destroy(self) {
		unsafe {
			(*self.next.unwrap()).prev = self.prev;
			(*self.prev.unwrap()).next = self.next;
		}

		drop(self);
	}
}


pub struct List<T> {
	first:  *mut Node<T>,
	last:   *mut Node<T>,
	cursor: *mut Node<T>
}

impl<T> List<T> {
	pub fn init(mut self, first_value: T) {
		let first_node : *mut Node<T> = &mut Node {
			value: first_value,
			next: None,
			prev: None
		};

		self.first = first_node;
		self.last = first_node;
		self.cursor = first_node;
	}

	pub fn curse_forth(mut self) {
		if self.cursor != self.last {
			self.cursor = unsafe { &mut *(*self.cursor).next.unwrap() };
		}
	}

	pub fn curse_back(mut self) {
		if self.cursor != self.first {
			self.cursor = unsafe { &mut *(*self.cursor).prev.unwrap() };
		}
	}
}



// tester
fn main() {
	let mut test1 : Node<i32> = Node {
		value: 1,
		next: None,
		prev: None
	};
	let mut test2 : Node<i32> = Node {
		value: 2,
		next: None,
		prev: None
	};
	let mut test3 : Node<i32> = Node {
		value: 3,
		next: None,
		prev: None
	};
	let mut test4 : Node<i32> = Node {
		value: 4,
		next: None,
		prev: None
	};
	                              // 1
	test1.link_forth(&mut test2); // 1, 2
	test1.link_forth(&mut test3); // 1, 3, 2
	test1.link_back(&mut test4);  // 4, 1, 3, 2
	test3.destroy();              // 4, 1, 2

	println!("{}", unsafe {(*test1.prev.unwrap()).value});
	println!("{}", test1.value);
	println!("{}", unsafe {(*test1.next.unwrap()).value});
}
