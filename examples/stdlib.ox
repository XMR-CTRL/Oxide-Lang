

fn main() -> i64 {
  print("abs(-7)=", abs(0 - 7));
  print("abs(42)=", abs(42));


  print("abs(-3.5)=", abs(0.0 - 3.5));
  let small: i16 = -91;
  print("abs(i16)=", abs(small));
  let u: u8 = 200;
  print("abs(u8)=", abs(u));

  print("imin(3, 9)=", imin(3, 9));
  print("imax(3, 9)=", imax(3, 9));

  let mut acc = 0;
  for let mut k = 0; k < 10; k = k + 1 {
    acc += imin(k, 10 - k);
  }
  print("sum of mins=", acc);


  let f: f64 = 2.0;
  print("sqrt(2)=", sqrt(f));
  print("fmax(1.5, 3.5)=", fmax(1.5, 3.5));
  print("fmin(1.5, 3.5)=", fmin(1.5, 3.5));


  let s: str = itos(12345);
  print("itos(12345)=", s);

  print("negation as string=", itos(abs(0 - 9)));
  print("'42' as int + 1=", stoi("42") + 1);
  print("'3.14' as double=", stod("3.14"));


  let n = 1729;
  let ns: str = itos(n);
  let digits_len = 4;


  print("round-trip n=", stoi(ns));
  return 0;
}
