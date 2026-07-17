
enum Color { Red, Green, Blue }
enum Shape { Circle, Square, Triangle }


fn name(c: Color) -> i64 {
  match c {
    Red   => { print("red");   return 1; }
    Green => { print("green"); return 2; }
    Blue  => { print("blue");  return 3; }
  }
  return 0;
}


fn classify(s: Shape) -> i64 {
  match s {
    Circle => { print("circle");   return 0; }
    Square => { print("square");   return 1; }
    _      => { print("triangle"); return 2; }
  }
}

fn main() -> i64 {
  let c: Color = Green;
  print(c);
  print(name(c) + 100);
  print(name(Red));
  classify(Square);
  classify(Triangle);
  print(c == Green, c == Red);
  print(c as i64);
  let c2: Color = 0;
  print(c2);
  return 0;
}
