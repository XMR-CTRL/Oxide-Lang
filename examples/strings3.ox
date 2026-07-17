


fn main() -> i64 {

  print("apple" < "banana");
  print("banana" < "apple");
  print("apple" < "apple");
  print("apple" <= "apple");
  print("apple" >= "apple");
  print("apple" > "apple");


  print("app" < "apple");
  print("apple" > "app");


  print("Zebra" < "apple");
  print("Zebra" < "zebra");


  let word = "oxide";
  if word >= "m" && word < "z" {
    print("in range");
  }


  let names: [str; 4] = ["carol", "alice", "dave", "bob"];

  let mut arr = names;
  let mut i = 0;
  while i < 4 {
    let mut j = 0;
    while j < 3 - i {
      if arr[j + 1] < arr[j] {
        let tmp = arr[j];
        arr[j] = arr[j + 1];
        arr[j + 1] = tmp;
      }
      j = j + 1;
    }
    i = i + 1;
  }
  print(arr);
  return 0;
}
