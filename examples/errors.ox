


fn add(a: i64, b: i64) -> i64 {
  return a + b;
}

fn broken() -> i64 {
  let x = goofball;
  let mut y = 10;
  y = y + missing_fn();
  return x + y;
}

fn bad_types() -> i64 {
  let z: f64 = "not a float";
  let w: i64 = true;
  if 42 {
    return w;
  }
  return 0;
}

fn main() -> i64 {
  return add(1, 2);
}
