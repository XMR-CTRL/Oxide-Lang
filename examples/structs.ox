


struct Point {
  x: i64;
  y: i64;
}

struct Rect {
  tl: Point;
  br: Point;
}

fn make_point(x: i64, y: i64) -> Point {
  return Point { x: x, y: y };
}

fn area(r: Rect) -> i64 {
  return (r.br.x - r.tl.x) * (r.br.y - r.tl.y);
}

fn main() -> i64 {
  let mut p: Point = Point { x: 1, y: 2 };
  print("p=", p);
  p.x = p.x + 5;
  print("p.x after += 5=", p.x);
  print("p.y=", p.y);

  let q: Point = make_point(10, 20);
  print("q=", q);

  let r: Rect = Rect {
    tl: Point { x: 0, y: 0 },
    br: Point { x: 4, y: 5 }
  };
  print("rect area=", area(r));


  let mut box2: Rect = r;
  box2.br.y = 9;
  print("box2=", box2);
  print("area=", area(box2));
  return 0;
}
