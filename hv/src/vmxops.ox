


fn vmx_instruct_error() -> i64 {
  let mut err: i64 = 0;
  asm!("vmread %rdx, %rax", in("{rdx}") 0x4400, out("{rax}") err);
  return err;
}


fn vmclear(region_phys: i64) -> i64 {
  asm!("vmclear (%rax)", in("{rax}") region_phys, sideeffect=true);
  return 0;
}


fn vmptrld(region_phys: i64) -> i64 {
  asm!("vmptrld (%rax)", in("{rax}") region_phys, sideeffect=true);
  return 0;
}


fn vmwrite(field: i64, value: i64) {
  asm!("vmwrite %rax, %rdx", in("{rax}") field, in("{rdx}") value, sideeffect=true);
}


fn vmwrite32(field: i64, value: i64) {
  vmwrite(field, value & 0xFFFFFFFF);
}


fn vmread(field: i64) -> i64 {
  let mut v: i64 = 0;
  asm!("vmread %rdx, %rax", in("{rdx}") field, out("{rax}") v);
  return v;
}


fn vmlaunch() -> i64 {
  let mut rc: i64 = 0;
  asm!("vmlaunch", out("{rax}") rc);
  return 1;
}


fn vmresume() -> i64 {
  let mut rc: i64 = 0;
  asm!("vmresume", out("{rax}") rc);
  return 1;
}


fn vmxoff() -> i64 {
  asm!("vmxoff", sideeffect=true);
  return 0;
}


fn vmcall() {
  asm!("vmcall", sideeffect=true);
}
