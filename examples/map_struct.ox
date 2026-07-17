struct Point { x: i64; y: i64 }
fn main() -> i64 {

  let pts: map[i64, Point] = map[i64, Point];
  map_set(pts, 2, Point { x: 1, y: 2 });
  map_set(pts, 1, Point { x: 9, y: 8 });
  map_set(pts, 5, Point { x: 4, y: 3 });
  print(pts);
  let p = map_get(pts, 2);
  print(p);
  print(map_get(pts, 1).x);
  print(len(pts));


  let grid: map[i64, map[i64, i64]] = map[i64, map[i64, i64]];
  let inner: map[i64, i64] = map[i64, i64];
  map_set(inner, 1, 10);
  map_set(inner, 2, 20);
  map_set(grid, 0, inner);
  print(grid);
  print(map_get(map_get(grid, 0), 2));
  return 0;
}
