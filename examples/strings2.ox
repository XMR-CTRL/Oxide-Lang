


fn main() -> i64 {
  let s = "hello, oxide";


  print(substr("hello", 1, 3));
  print(substr(s, 7, 5));
  print(substr("short", 2, 100));
  print(substr("abc", 100, 3));


  print(index_of(s, ','));
  print(index_of(s, 'o'));
  print(index_of("rust", 'z'));


  print(ftos(3.14));
  print("pi ~ " + ftos(3.14159));


  let bang = char_to_str('!');
  print(bang);
  print("done" + '!');
  print('!' + "done");


  let greeting = "hi" + ' ' + "oxide" + '\n';
  print(greeting);


  let word = "rust";
  print(word[0], word[1], word[2], word[3]);


  return 0;
}
