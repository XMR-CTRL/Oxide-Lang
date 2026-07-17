

fn factorial(n: i64) -> i64 {
  if n <= 1 {
    return 1;
  }
  return n * factorial(n - 1);
}

fn classify(n: i64) -> i64 {
  if n < 0 {
    return 0 - 1;
  } else if n == 0 {
    return 0;
  } else if n < 10 {
    return 1;
  } else {
    return 2;
  }
}

fn main() -> i64 {
  let mut i = 1;
  while i <= 6 {
    print("factorial", i, "=", factorial(i));
    i += 1;
  }

  let mut acc = 0;
  for let mut k = 0; k < 50; k = k + 1 {
    acc += classify(k);
  }
  print("classify sum 0..49=", acc);

  let flag = true;
  if flag {
    print("booleans work");
  }
  return 0;
}
