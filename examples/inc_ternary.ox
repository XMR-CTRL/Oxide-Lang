


struct P { x: i64; y: i64; }

fn main() -> i64 {


  let n: i64 = 7;
  let parity = (n % 2 == 0) ? "even" : "odd";
  print(parity);
  let max3 = (n > 5) ? (n > 100 ? "big" : "mid") : "small";
  print(max3);


  let pick = (n > 5) ? P { x: 1, y: 2 } : P { x: 9, y: 8 };
  print(pick);


  let mut i: i64 = 0;
  let a = i++;
  print(a, i);
  let b = ++i;
  print(b, i);
  let c = i--; print(c, i);
  let d = --i; print(d, i);


  let mut ch: char = 'A';
  ch++; print(ch);
  ch--; ch--; print(ch);


  let mut z: usize = 5;
  z++; z++;
  print(z);


  let mut arr: [i64; 4] = [10, 20, 30, 40];
  let mut p: *i64 = &arr[0];
  p++;
  print(mmio_load(p));


  let mut v: [i64; 3] = [1, 2, 3];
  v[1]++; v[1]++;
  print(v[1]);


  let mut pt: P = P { x: 5, y: 6 };
  pt.x++; pt.y--;
  print(pt);


  let mut q: *i64 = &arr[2];
  (*q) = (*q) + 100;
  (*q)++;
  print(arr[2]);

  return 0;
}
