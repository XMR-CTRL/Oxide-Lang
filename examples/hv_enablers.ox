


const APIC_BASE  = 0xFEE00000;
const PAGE_SHIFT = 12;
const PAGE_SIZE  = 1 << PAGE_SHIFT;
const PTE_PRESENT = 0b1;


fn apic_read(reg: &u32) -> u32 {
  return mmio_load(reg);
}
fn apic_write(reg: &u32, val: u32) {
  mmio_store(reg, val);
}


fn zero_page(dst: &u8) {
  memset(dst, 0, PAGE_SIZE);
}
fn copy_page(dst: &u8, src: &u8) {
  memcpy(dst, src, PAGE_SIZE);
}


fn pte_at(table: &u8, idx: i64) -> &u8 {
  return table + idx * PAGE_SIZE;
}


fn cpu_halt() {
  asm!("hlt", sideeffect=true);
}
fn cpu_vmcall() {
  asm!("vmcall", sideeffect=true, memory=true);
}

fn rdtsc_low() -> u32 {
  let mut lo: u32 = 0;
  asm!("rdtsc", out("{eax}") lo, clobbers="edx");
  return lo;
}


fn kmain(a: i64, b: i64) -> i64 {
  cpu_halt();
  cpu_vmcall();
  let tsc = rdtsc_low();
  let table: &u8 = null as &u8;
  zero_page(table);
  let entry = pte_at(table, 3);
  copy_page(entry, table);
  return (tsc as i64) + a + b;
}
