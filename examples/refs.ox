


struct Point { x: i64, y: i64 }


fn move_by(p: &Point, dx: i64, dy: i64) -> i64 {
  p.x = p.x + dx;
  p.y = p.y + dy;
  return 0;
}


fn dist_squared(p: &Point) -> i64 {
  return p.x * p.x + p.y * p.y;
}


fn zero(arr: &[i64; 5], i: i64) -> i64 {
  arr[i] = 0;
  return arr[i];
}

fn main() -> i64 {
  let mut pt = Point { x: 3, y: 4 };
  print(dist_squared(&pt));
  move_by(&pt, 10, 20);
  print(pt);
  print(dist_squared(&pt));


  let nums = [10, 20, 30, 40, 50];
  let idx = 2;
  print("element refs:");
  let r = &nums;
  print(r[0]);

  let val = zero(&nums, idx);
  print("after zero dyn:", nums);
  return 0;
}
