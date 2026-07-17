

fn main() -> i64 {
  let s = "hello";
  print(s[0], s[4]);
  let upper = s as str;
  print(upper[0]);

  let g = "hello" + " world";
  print(g);

  print(len(s), len(g));
  print(s == "hello");
  print(s == "world");
  print(s + "!");


  let ch = 'A';
  print(ch);
  print(ch + 1);
  let next = 'Z' as char;
  print(next);
  return 0;
}
