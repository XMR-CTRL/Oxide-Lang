


struct Point { x: i64, y: i64 }

fn main() -> i64 {
  let pts: vec[Point] = vec[Point];
  push(pts, Point { x: 1, y: 2 });
  push(pts, Point { x: 9, y: 0 });
  push(pts, Point { x: 4, y: 5 });
  print("pts =", pts);


  pts[1].x = 7;
  print("after mutate, pts[1] =", pts[1]);
  print("pts len =", len(pts));


  let p = pts[2];
  print("p.x + p.y =", p.x + p.y);


  let grid: vec[[i64; 3]] = vec[[i64; 3]];
  let row = [11, 22, 33];
  push(grid, row);
  let row2 = [44, 55, 66];
  push(grid, row2);
  print("grid =", grid);
  print("grid[0][1] =", grid[0][1]);
  grid[1][2] = 999;
  print("grid[1] =", grid[1]);


  let board: vec[vec[i64]] = vec[vec[i64]];
  let a: vec[i64] = vec[i64];
  push(a, 1); push(a, 2);
  push(board, a);
  let b: vec[i64] = vec[i64];
  push(b, 3); push(b, 4); push(b, 5);
  push(board, b);
  print("board =", board);
  print("board[1][2] =", board[1][2]);


  let mat: vec[i64] = board[1];
  push(mat, 6);
  print("board[1] length =", len(mat));


  let mut s: i64 = 0;
  for q in pts {
    s = s + q.x + q.y;
  }
  print("sum of all points =", s);


  pts[0].x += 5;
  pts[2].y -= 5;
  print("after += / -=, pts =", pts);

  return 0;
}
