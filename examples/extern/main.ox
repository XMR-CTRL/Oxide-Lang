


extern fn add_mul(a: i64, b: i64) -> i64;
extern fn pick_max(xs: vec[i64]) -> i64;

fn main() -> i64 {
  let am = add_mul(3, 4);
  print("add_mul(3,4) =", am);

  let nums = vec[i64];
  push(nums, 5);
  push(nums, 12);
  push(nums, 9);
  push(nums, 21);
  push(nums, 7);
  let mx = pick_max(nums);
  print("max =", mx);
  return 0;
}
