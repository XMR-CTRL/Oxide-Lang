


fn id<T>(x: T) -> T { return x; }


fn max<T>(a: T, b: T) -> T {
  if a > b { return a; }
  return b;
}


struct Pair<A, B> { a: A; b: B; }

fn first<A, B>(p: Pair<A, B>) -> A { return p.a; }
fn second<A, B>(p: Pair<A, B>) -> B { return p.b; }


fn make_pair<A, B>(a: A, b: B) -> Pair<A, B> {
  return Pair<A, B> { a: a, b: b };
}

fn main() -> i64 {
  print(id<i64>(7));
  print(id(42));

  print(max<i64>(3, 9));
  print(max(11, 4));

  let p = make_pair<i64, str>(1, "hello");
  print(first<i64, str>(p));
  print(second<i64, str>(p));


  let q = Pair<str, str> { a: "x", b: "y" };
  print(q);
  return 0;
}
