


struct Point {
  x: i64;
  y: i64;
}

impl Point {

  fn new(x: i64, y: i64) -> Point {
    return Point { x: x, y: y };
  }

  fn mag(self) -> i64 {
    return self.x * self.x + self.y * self.y;
  }


  fn translate(&self, dx: i64, dy: i64) {
    self.x = self.x + dx;
    self.y = self.y + dy;
  }
  fn to(&self, other: Point) -> Point {
    return Point {
      x: other.x - self.x,
      y: other.y - self.y,
    };
  }
}

fn main() -> i64 {
  let mut p: Point = Point::new(3, 4);
  print("p =", p);
  print("mag =", p.mag());

  p.translate(10, 20);
  print("translated p =", p);

  let q: Point = Point::new(1, 1);
  print("p to q =", p.to(q));


  print("origin mag =", Point::new(5, 12).mag());
  return 0;
}
