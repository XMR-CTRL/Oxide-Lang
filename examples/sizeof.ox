


struct Point { x: i64; y: i64; }
struct Padded { a: i8; b: i64; }


const N = sizeof([u8; 9]);

fn main() -> i64 {

  print(sizeof(i64), sizeof(i32), sizeof(i8), sizeof(bool), sizeof(f64), sizeof(char));


  print(sizeof(&i64), sizeof(str));


  print(sizeof([i64; 4]));
  print(sizeof([u8; 9]));
  print(sizeof([[i32; 2]; 3]));


  print(sizeof(Point));
  print(sizeof(Padded));


  print(N);


  let npts = sizeof([Point; 3]) / sizeof(Point);
  print("points in array =", npts);


  if sizeof(i64) == 8 {
    print("64-bit pointers it is");
  }
  return 0;
}
