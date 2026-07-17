


fn add_mul(a: i64, b: i64) -> i64 {
  return (a + b) * (a + b);
}


fn pick_max(xs: vec[i64]) -> i64 {
  let mut best = 0;
  for let mut i = 0; i < len(xs); i = i + 1 {
    if i == 0 || xs[i] > best { best = xs[i]; }
  }
  return best;
}
