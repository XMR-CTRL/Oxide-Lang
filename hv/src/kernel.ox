


const COM1 = 0x3F8;


fn rdmsr(msr: i64) -> i64 {
  let mut lo: i64 = 0;
  let mut hi: i64 = 0;
  let ecx: i64 = msr;
  asm!("rdmsr", in("{ecx}") ecx, out("{eax}") lo, out("{edx}") hi);
  return (hi << 32) | lo;
}

fn wrmsr(msr: i64, val: i64) {
  let ecx: i64 = msr;
  let lo: i64 = val & 0xFFFFFFFF;
  let hi: i64 = (val >> 32) & 0xFFFFFFFF;
  asm!("wrmsr", in("{ecx}") ecx, in("{eax}") lo, in("{edx}") hi);
}
fn read_cr0() -> i64 {
  let mut v: i64 = 0;
  asm!("mov %cr0, %rax", out("{rax}") v);
  return v;
}
fn write_cr0(v: i64) {
  asm!("mov %rax, %cr0", in("{rax}") v);
}
fn read_cr4() -> i64 {
  let mut v: i64 = 0;
  asm!("mov %cr4, %rax", out("{rax}") v);
  return v;
}
fn write_cr4(v: i64) {
  asm!("mov %rax, %cr4", in("{rax}") v);
}

fn write_cr3(v: i64) { asm!("mov %rax, %cr3", in("{rax}") v); }


fn outb(port: i64, val: i64) {
  let dx: i16 = port as i16;
  let al: i8 = val as i8;
  asm!("outb %al, %dx", in("{dx}") dx, in("{al}") al);
}
fn inb(port: i64) -> i64 {
  let dx: i16 = port as i16;
  let mut al: i8 = 0;
  asm!("inb %dx, %al", in("{dx}") dx, out("{al}") al);
  return al as i64;
}


fn halt_forever() {

  asm!("cli", sideeffect=true);
  asm!("hlt", sideeffect=true);
  halt_forever();
}


fn serial_init() {
  outb(COM1 + 1, 0);
  outb(COM1 + 3, 0x80);
  outb(COM1 + 0, 3);
  outb(COM1 + 1, 0);
  outb(COM1 + 3, 3);
  outb(COM1 + 2, 0xC7);
  outb(COM1 + 4, 0x0B);
}

fn serial_can_send() -> bool {
  return (inb(COM1 + 5) & 0x20) != 0;
}
fn serial_putc(c: u8) {

  let mut guard = 0;
  while !serial_can_send() {
    guard = guard + 1;
    if guard > 100000 { return; }
  }
  outb(COM1, c as i64);
}
fn serial_puts(s: &u8) {


  let mut p: &u8 = s;
  while true {
    let c: u8 = mmio_load(p);
    if c == 0 { return; }
    serial_putc(c);
    p = p + 1;
  }
}


fn has_vmx() -> bool {
  let mut eax: i64 = 1;
  let mut ecx: i64 = 0;
  let mut ebx: i64 = 0;
  let mut edx: i64 = 0;


  asm!("cpuid", inout("{eax}") eax, out("{ebx}") ebx,
                 out("{ecx}") ecx, out("{edx}") edx);
  return (ecx & (1 << 5)) != 0;
}


let mut vmxon_region: [u8; 0x1000];

fn vmx_on() -> i64 {

  let cr4v = read_cr4();
  write_cr4(cr4v | (1 << 13));

  let basic = rdmsr(MSR_IA32_VMX_BASIC);
  let rev: u32 = (basic & 0x7FFFFFFF) as u32;


  let region_ptr: &u8 = &vmxon_region[0];
  let rev_ptr: &u32 = region_ptr as &u32;
  mmio_store(rev_ptr, rev);


  let region_phys: i64 = region_ptr as usize;
  asm!("vmxon (%rax)", in("{rax}") region_phys, sideeffect=true);
  return 0;
}


const MSG_BOOT: str = "[oxide-hv] long-mode entry reached\n";


fn oxide_long_mode_entry() -> i64 {


  let sp = ox_entry_stack_top_addr();
  asm!("mov %rax, %rsp", in("{rax}") sp);


  asm!("ltr %ax", in("{ax}") 0x18);


  serial_init();
  serial_puts(str_ptr(MSG_BOOT));


  if !has_vmx() {
    serial_puts(str_ptr("[oxide-hv] VT-x NOT available\n"));
    halt_forever();
  }


  let status = vmx_on();
  serial_puts(str_ptr("[oxide-hv] VMX root entered\n"));


  let launch_rc = launch_guest();
  serial_puts(str_ptr("[oxide-hv] vm-entry failed\n"));
  halt_forever();
  return 0;
}


extern fn ox_entry_stack_top_addr() -> i64;
