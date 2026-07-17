

fn main() -> i64 {

  let b: u8 = 255;
  let overflowed: u8 = b + 1;
  print("u8 255 + 1=", overflowed);

  let s: i16 = -4;
  let widened: i64 = s as i64;
  print("i16 -4 as i64=", widened);

  let big: i64 = 100000;
  let narrowed: u8 = big as u8;
  print("i64 100000 as u8=", narrowed);


  let u: u32 = 0;
  let gtu: bool = u > 1;
  print("u32 0 > 1 = (should be false)", gtu);


  let f: f64 = 3.7;
  let truncated: i64 = f as i64;
  print("3.7 as i64=", truncated);
  let back: f64 = (truncated + 1) as f64;
  print("as f64 again=", back);


  let bytes: usize = 16;
  let total: usize = bytes * 4;
  print("usize bytes*4=", total);


  let mix: i32 = 7;
  let toWide: i64 = mix as i64;
  let toNarrow: i8 = toWide as i8;
  print("i32 7 -> i64 -> i8=", toNarrow);
  return 0;
}
