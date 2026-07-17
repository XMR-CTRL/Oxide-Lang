
fn main() -> i64 {
  let m: map[str, i64] = map[str, i64];
  map_set(m, "banana", 3);
  map_set(m, "apple", 5);
  map_set(m, "cherry", 2);
  print(m);
  print(map_get(m, "apple"));
  print(map_contains(m, "pear"));
  print(map_contains(m, "banana"));
  print(len(m));
  map_set(m, "banana", 30);
  print(map_get(m, "banana"));
  print(len(m));


  let nums: map[i64, f64] = map[i64, f64];
  map_set(nums, 2, 0.5);
  map_set(nums, 10, 0.25);
  map_set(nums, 1, 0.125);
  print(nums);
  print(map_get(nums, 2));


  let keys = map_keys(m);
  for k in keys {
    print(k, "=", map_get(m, k));
  }
  return 0;
}
