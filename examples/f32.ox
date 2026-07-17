


struct Pt { x: f32; y: f32; }

fn half(x: f32) -> f32 { return x * 0.5; }

fn main() -> i64 {
  let a: f32 = 1.5;
  let b: f32 = half(a);
  print(a, b);

  let c: f32 = a + 2.0;
  print(c);


  let wide: f64 = b as f64;
  let narrow: f32 = wide as f32;
  print(wide, narrow);


  let n: i64 = wide as i64;
  let back: f32 = n as f32;
  print(n, back);


  print(3.0 > 2.0, a < c, a == 1.5);


  let p = Pt { x: 1.0, y: 2.0 };
  print(p.x, p.y);


  let arr: [f32; 3] = [0.5, 1.5, 2.5];
  print(arr[0], arr[1], arr[2]);
  print(arr);

  return 0;
}
