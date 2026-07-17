


struct Pair { x: i64, y: i64 }


const N = 10;
const PAIR_STRIDE = N * 2;
const GREETING = "hi, " + "oxide";


typedef Count = i64;
typedef Point = Pair;


let mut counter: Count = 0;

fn bump(n: Count) -> Count {
  counter = counter + n;
  return counter;
}


fn dist(p: Point) -> i64 {
  return p.x * p.x + p.y * p.y;
}

fn main() -> i64 {
  print("N =", N);
  print("stride =", PAIR_STRIDE);
  print(GREETING);
  print("counter start =", counter);

  bump(3); bump(7);
  print("counter after bumps =", counter);

  let p: Point = Pair { x: origin_x(), y: origin_y() };
  print("dist =", dist(p));
  return 0;
}


const OX = 4;
const OY = 5;
fn origin_x() -> i64 { return OX; }
fn origin_y() -> i64 { return OY; }
