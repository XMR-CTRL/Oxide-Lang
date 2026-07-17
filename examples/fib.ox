fn fib(n: i64) -> i64 {
  if n < 2 {
    return n;
  }
  return fib(n - 1) + fib(n - 2);
}

fn main() -> i64 {
  let mut sum = 0;
  for let mut i = 0; i < 10; i = i + 1 {
    sum = sum + fib(i);
  }
  print("fib sum 0..9=", sum);
  return 0;
}
