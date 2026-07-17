

fn main() -> i64 {

  let nums = [10, 20, 30, 40, 50];
  let mut sum = 0;
  for x in nums {
    sum = sum + x;
  }
  print("sum of fixed array=", sum);


  let squares: vec[i64] = vec[i64];
  for let mut i = 0; i < 5; i = i + 1 {
    push(squares, i * i);
  }
  for v in squares {
    print(v);
  }


  for c in "rust" {
    print(c);
  }


  let names = ["alice", "bob", "carol", "dave"];
  for n in names {
    if n == "bob" { continue; }
    if n == "dave" { break; }
    print(n);
  }
  return 0;
}
