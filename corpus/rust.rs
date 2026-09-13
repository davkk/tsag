//! Variate Rust corpus: every item kind + visibility/generic/attr shapes.
use std::collections::HashMap;
use std::fmt::{self, Display};
use crate::inner::{helper, SHADOW as _};
pub use std::io::Result as IoResult;
use super::sibling::*;

pub const MAX: u32 = 100;
pub const MSG: &str = "hi";
const PRIVATE_CONST: i32 = -1;
pub(crate) const CRATE_CONST: u8 = 2;

pub static NAME: &str = "tsag";
pub static mut COUNTER: i32 = 0;
static INTERNAL: u64 = 9;
pub(crate) static CRATE_STATIC: bool = true;

pub mod inner {
    pub const SHADOW: u32 = 1;
    pub fn helper() -> u32 {
        1
    }
    fn private_fn() {}
    pub(crate) mod nested {
        pub fn deep() {}
    }
}

mod plain_mod {
    pub fn f() {}
}

pub fn add(a: i32, b: i32) -> i32 {
    a + b
}

fn internal() {}

pub async fn fetch(url: &str) -> Result<String, std::io::Error> {
    Ok(url.to_owned())
}

pub unsafe fn dangerous(x: *mut u8) {}
pub const fn const_add(a: u32, b: u32) -> u32 {
    a + b
}
extern "C" fn c_callback(v: i32) {}
pub fn generic_id<T>(x: T) -> T {
    x
}
pub fn with_lifetime<'a>(x: &'a str) -> &'a str {
    x
}
pub fn with_where<T>(x: T) -> T
where
    T: Clone + fmt::Debug,
{
    x
}
pub fn impl_trait_arg(x: impl Display) -> String {
    x.to_string()
}
pub fn diverging(msg: &str) -> ! {
    panic!("{}", msg)
}

pub struct Point {
    pub x: f64,
    pub y: f64,
    hidden: bool,
}

pub struct Unit;
pub struct Tuple(pub u32, String);
pub struct Generic<T, U = String> {
    pub first: T,
    pub second: U,
}

#[derive(Debug, Clone, PartialEq)]
#[repr(C)]
pub struct Attr {
    pub v: u32,
}

pub enum Status {
    Ok,
    Err(String),
    Moved { x: i32, y: i32 },
    Code(u16) = 100,
}

pub enum GenericE<T> {
    Some(T),
    None,
}

pub trait Shape {
    type Output;
    const SIDES: u32;
    fn area(&self) -> f64;
    fn name(&self) -> String {
        "shape".into()
    }
    fn generic_m<T: Display>(&self, x: T) -> String;
}

pub unsafe trait UnsafeM {}

pub type Id = u64;
pub type Map<K, V = String> = HashMap<K, V>;
pub type Cb = fn(u32) -> u32;
pub(crate) type CrateAlias<T> = Vec<T>;

impl Point {
    pub const ORIGIN: Point = Point { x: 0.0, y: 0.0, hidden: false };
    pub fn new(x: f64, y: f64) -> Self {
        Point { x, y, hidden: false }
    }
    pub fn norm(&self) -> f64 {
        (self.x * self.x + self.y * self.y).sqrt()
    }
    pub fn set(&mut self, x: f64, y: f64) {}
    fn reset(&mut self) {}
    pub async fn refresh(&self) {}
    pub unsafe fn raw(&self) -> *const Point {
        self
    }
}

impl<T> Generic<T> {
    pub fn get(&self) -> &T {
        &self.first
    }
}

impl Shape for Point {
    type Output = f64;
    const SIDES: u32 = 0;
    fn area(&self) -> f64 {
        0.0
    }
    fn generic_m<T: Display>(&self, x: T) -> String {
        x.to_string()
    }
}

impl fmt::Display for Point {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, "({}, {})", self.x, self.y)
    }
}

#[macro_export]
macro_rules! my_macro {
    () => {};
    ($x:expr) => {
        $x
    };
    ($x:expr, $($rest:tt)*) => {};
}

macro_rules! local_macro {
    ($t:ty) => {};
}

macro local_style {}

pub fn uses_macro() -> u32 {
    my_macro!();
    let v = vec![1, 2, 3];
    let m: HashMap<i32, i32> = HashMap::new();
    let clos = |a: i32, b: i32| a + b; // closure must NOT tag
    fn nested_fn() -> u32 {
        1
    }
    nested_fn() + v.len() as u32 + clos(1, 2)
}

#[cfg(test)]
mod tests {
    use super::*;
    #[test]
    fn it_adds() {
        assert_eq!(add(1, 2), 3);
    }
}

fn main() {
    let p = Point::new(1.0, 2.0);
    println!("{} {}", p.norm(), add(1, 2));
}
