


fn ox_com1_panic_put(c: i64) {
  asm!("outb %al, %dx", in("{dx}") 0x3F8, in("{al}") c);
}


fn ox_bounds_fail(idx: i64, bound: i64) {

  ox_com1_panic_put(0x4B);
  ox_com1_panic_put(0x70);
  ox_com1_panic_put(0x4C);
  ox_com1_panic_put(0x2E);
  ox_com1_panic_put(0x2E);


  ox_com1_panic_put((idx ^ bound) & 0xFF);

  let mut i: i64 = 0;
  while i < 1000 {
    asm!("cli", sideeffect=true);
    asm!("hlt", sideeffect=true);
    i = i + 1;
  }
  while true { asm!("hlt", sideeffect=true); }
}
