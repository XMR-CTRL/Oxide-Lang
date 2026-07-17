

fn main() -> i64 {

  let h = file_open("oxide_io_demo.txt", "w");
  if h < 0 {
    print("could not open file for writing");
    return 1;
  }
  file_write(h, "hello from oxide\n");
  file_write(h, "second line\n");
  file_close(h);

  if file_exists("oxide_io_demo.txt") {
    print("file exists");
  }


  let contents: str = read_file("oxide_io_demo.txt");
  print("contents:");
  print(contents);


  let h2 = file_open("oxide_io_demo.txt", "r");
  if h2 < 0 {
    print("could not reopen");
    return 1;
  }
  let streamed: str = file_read(h2);
  print("streamed bytes:");
  print(streamed);
  file_close(h2);


  print("done");
  return 0;
}
