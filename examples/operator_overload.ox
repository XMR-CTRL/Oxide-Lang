


struct Vec2 {
  x: i64;
  y: i64;
}

impl Vec2 {
  fn new(x: i64, y: i64) -> Vec2 {
    return Vec2 { x: x, y: y };
  }
  fn __add(self, o: Vec2) -> Vec2 {
    return Vec2 { x: self.x + o.x, y: self.y + o.y };
  }
  fn __sub(self, o: Vec2) -> Vec2 {
    return Vec2 { x: self.x - o.x, y: self.y - o.y };
  }
  fn __mul(self, k: i64) -> Vec2 {
    return Vec2 { x: self.x * k, y: self.y * k };
  }
  fn __neg(self) -> Vec2 {
    return Vec2 { x: 0 - self.x, y: 0 - self.y };
  }
  fn __eq(self, o: Vec2) -> bool {
    return self.x == o.x && self.y == o.y;
  }
  fn __lt(self, o: Vec2) -> bool {
    return self.x * self.x + self.y * self.y < o.x * o.x + o.y * o.y;
  }
  fn __index(self, i: i64) -> i64 {
    if i == 0 { return self.x; }
    return self.y;
  }
  fn __iadd(&self, o: Vec2) {
    self.x = self.x + o.x;
    self.y = self.y + o.y;
  }
}

fn main() -> i64 {
  let a = Vec2::new(1, 2);
  let b = Vec2::new(3, 4);

  print(a + b);
  print(a - b);
  print(a * 5);
  print(-b);

  print(a + b == Vec2::new(4, 6));
  print(a < b);
  print(a != b);

  print(a[0], a[1]);

  let mut m = Vec2::new(10, 20);
  m += Vec2::new(1, 1);
  print(m);


  print(-(a + b) + Vec2::new(0, 0));

  if a < b && a != b { print("a is the smaller one"); }
  return 0;
}
