use std::fmt;

pub const MAX: u32 = 100;
pub static NAME: &str = "tsag";
static mut COUNTER: i32 = 0;

pub mod inner {
    pub fn helper() {}
}

pub struct Point {
    pub x: f64,
    pub y: f64,
}

pub enum Status {
    Ok,
    Err(String),
}

pub trait Shape {
    fn area(&self) -> f64;
}

pub type Id = u64;

pub fn add(a: i32, b: i32) -> i32 {
    a + b
}

fn internal() {}

impl Point {
    pub fn new(x: f64, y: f64) -> Self {
        Point { x, y }
    }

    pub fn norm(&self) -> f64 {
        (self.x * self.x + self.y * self.y).sqrt()
    }
}

impl Shape for Point {
    fn area(&self) -> f64 {
        0.0
    }
}

macro_rules! my_macro {
    () => {};
}

fn main() {
    let p = Point::new(1.0, 2.0);
    println!("{}", p.norm());
}
