


fn kmain(a: i64, b: i64) -> i64 {
  let s = a + b;
  let p = a * b;
  let t = s * s - p;
  if t > 0 {
    return t;
  }
  return 0 - t;
}


fn fib(n: i64) -> i64 {
  if n < 2 { return n; }
  let mut a: i64 = 0;
  let mut b: i64 = 1;
  let mut i: i64 = 0;
  while i < n {
    let c = a + b;
    a = b;
    b = c;
    i = i + 1;
  }
  return b;
}


fn entry_check() -> i64 {
  return fib(10) + kmain(3, 4);
}
