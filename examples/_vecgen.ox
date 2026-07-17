enum Suit { Clubs, Diamonds, Hearts, Spades }

fn main() -> i64 {

  let chars: vec[char] = vec[char];
  for c in "hello" {
    push(chars, c);
  }
  print(chars);


  let hand: vec[Suit] = vec[Suit];
  push(hand, Hearts);
  push(hand, Spades);
  push(hand, Clubs);
  print(hand);


  let mut total = 0;
  for s in hand {
    total = total + (s as i64);
  }
  print("sum of suits:", total);


  let ns: vec[i64] = vec[i64];
  for let mut i = 0; i < 5; i = i + 1 {
    push(ns, i * i);
  }
  print(ns);
  return 0;
}
