
fn main() -> i64 {
  let s: set[i64] = set[i64];
  set_insert(s, 5);
  set_insert(s, 2);
  set_insert(s, 9);
  set_insert(s, 2);
  set_insert(s, 1);
  print(s);
  print(len(s));
  print(set_contains(s, 5));
  print(set_contains(s, 7));
  set_remove(s, 2);
  print(s);
  print(len(s));


  let names: set[str] = set[str];
  set_insert(names, "zoe");
  set_insert(names, "amy");
  set_insert(names, "bob");
  print(names);


  for e in set_to_vec(s) {
    print(e);
  }
  return 0;
}
