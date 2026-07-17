

fn sum_array(a: [i64; 5]) -> i64 {
  let mut total = 0;
  for let mut i = 0; i < 5; i = i + 1 {
    total += a[i];
  }
  return total;
}

fn main() -> i64 {
  let mut nums: [i64; 5] = [10, 20, 30, 40, 50];
  print("nums=", nums);
  print("nums[2]=", nums[2]);

  nums[2] = 99;
  print("after write nums[2]=", nums[2]);
  print("sum=", sum_array(nums));


  let mut rev: [i64; 5] = [0, 0, 0, 0, 0];
  for let mut i = 0; i < 5; i = i + 1 {
    rev[i] = nums[4 - i];
  }
  print("reversed=", rev);


  let pts: [f64; 3] = [1.5, 2.5, 3.5];
  print("pts=", pts);


  let mut grid: [[i64; 2]; 3] = [[1, 2], [3, 4], [5, 6]];
  print("grid[1][1]=", grid[1][1]);
  grid[1][1] = 40;
  print("grid[1][1] now=", grid[1][1]);
  print("grid=", grid);
  return 0;
}
