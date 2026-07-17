


let mut vmcs_region: [u8; 0x1000];


let mut host_stack: [u8; 0x2000];


extern fn ox_host_rip() -> i64;
extern fn ox_guest_rip() -> i64;
extern fn ox_host_stack_top_addr() -> i64;


fn vmwrite16(field: i64, value: i64) {
  vmwrite(field, value & 0xFFFF);
}


fn read_cr3_phys() -> i64 {
  let mut v: i64 = 0;
  asm!("mov %cr3, %rax", out("{rax}") v);
  return v & 0xFFFFFFFFFFFFF000;
}


fn launch_guest() -> i64 {

  let basic = rdmsr(MSR_IA32_VMX_BASIC);
  let rev: u32 = (basic & 0x7FFFFFFF) as u32;
  let vmcs_ptr: &u8 = &vmcs_region[0];
  let rev_ptr: &u32 = vmcs_ptr as &u32;
  mmio_store(rev_ptr, rev);
  let abort_ptr: &u32 = (vmcs_ptr + 4) as &u32;
  mmio_store(abort_ptr, 0);


  let vmcs_phys: i64 = vmcs_ptr as usize;
  vmclear(vmcs_phys);
  vmptrld(vmcs_phys);


  vmwrite(VMCS_HOST_RIP, ox_host_rip());
  vmwrite(VMCS_HOST_RSP, ox_host_stack_top_addr());

  vmwrite16(VMCS_HOST_CS_SEL, 0x08);
  vmwrite16(VMCS_HOST_SS_SEL, 0x10);
  vmwrite16(VMCS_HOST_DS_SEL, 0x10);
  vmwrite16(VMCS_HOST_ES_SEL, 0x10);
  vmwrite16(VMCS_HOST_FS_SEL, 0x00);
  vmwrite16(VMCS_HOST_GS_SEL, 0x00);
  vmwrite16(VMCS_HOST_TR_SEL, 0x18);
  vmwrite(VMCS_HOST_CR0, read_cr0());
  vmwrite(VMCS_HOST_CR3, read_cr3_phys());
  vmwrite(VMCS_HOST_CR4, read_cr4());
  vmwrite(VMCS_HOST_FS_BASE, 0);
  vmwrite(VMCS_HOST_GS_BASE, rdmsr(0xC0000101));


  vmwrite(VMCS_HOST_GDTR_BASE, 0);
  vmwrite(VMCS_HOST_IDTR_BASE, 0);
  vmwrite(VMCS_HOST_TR_BASE,  0);
  vmwrite(VMCS_HOST_SYSENTER_CS, 0);
  vmwrite(VMCS_HOST_SYSENTER_ESP, 0);
  vmwrite(VMCS_HOST_SYSENTER_EIP, 0);


  let exit_ctl = vmx_adjust_ctl(MSR_IA32_VMX_EXIT_CTLS,
        VM_EXIT_HOST_ADDR_SPACE_SIZE | VM_EXIT_SAVE_IA32_EFER
      | VM_EXIT_LOAD_IA32_EFER);
  vmwrite32(VMCS_EXIT_CTLS, exit_ctl);


  let entry_ctl = vmx_adjust_ctl(MSR_IA32_VMX_ENTRY_CTLS,
        VM_ENTRY_IA32E_MODE | VM_ENTRY_LOAD_IA32_EFER);
  vmwrite32(VMCS_ENTRY_CTLS, entry_ctl);


  let pin = vmx_adjust_ctl(MSR_IA32_VMX_PINBASED_CTLS, 0);
  vmwrite32(VMCS_PIN_BASED, pin);
  let proc = vmx_adjust_ctl(MSR_IA32_VMX_PROCBASED_CTLS,
        CPU_BASED_HLT_EXITING | CPU_BASED_ACTIVATE_SECONDARY);
  vmwrite32(VMCS_PROC_BASED, proc);
  let proc2 = vmx_adjust_ctl(MSR_IA32_VMX_PROCBASED2_CTLS, 0);
  vmwrite32(VMCS_PROC_BASED2, proc2);
  vmwrite32(VMCS_EXC_BITMAP, 0);


  let gcr0 = 0x80000033;
  vmwrite(VMCS_GUEST_CR0, gcr0);
  vmwrite(VMCS_CR0_GUEST_HOST_MASK, 0);
  vmwrite(VMCS_CR0_READ_SHADOW, gcr0);
  vmwrite(VMCS_GUEST_CR3, read_cr3_phys());

  let gcr4 = (1 as i64) << 5;
  vmwrite(VMCS_GUEST_CR4, gcr4);
  vmwrite(VMCS_CR4_GUEST_HOST_MASK, 0);
  vmwrite(VMCS_CR4_READ_SHADOW, gcr4);


  vmwrite(VMCS_GUEST_IA32_EFER, 0x300);


  vmwrite16(VMCS_GUEST_CS_SEL, 0x08);
  vmwrite16(VMCS_GUEST_SS_SEL, 0x10);
  vmwrite16(VMCS_GUEST_DS_SEL, 0x10);
  vmwrite16(VMCS_GUEST_ES_SEL, 0x10);
  vmwrite16(VMCS_GUEST_FS_SEL, 0x00);
  vmwrite16(VMCS_GUEST_GS_SEL, 0x00);
  vmwrite16(VMCS_GUEST_LDTR_SEL, 0x00);
  vmwrite16(VMCS_GUEST_TR_SEL, 0x18);
  vmwrite32(VMCS_GUEST_CS_ACCESS_RIGHTS, AR_CODE64_EXEC_READ_DPL0);
  vmwrite32(VMCS_GUEST_SS_ACCESS_RIGHTS, AR_DATA_RW_DPL0);
  vmwrite32(VMCS_GUEST_DS_ACCESS_RIGHTS, AR_DATA_RW_DPL0);
  vmwrite32(VMCS_GUEST_ES_ACCESS_RIGHTS, AR_DATA_RW_DPL0);
  vmwrite32(VMCS_GUEST_FS_ACCESS_RIGHTS, AR_UNUSABLE);
  vmwrite32(VMCS_GUEST_GS_ACCESS_RIGHTS, AR_UNUSABLE);
  vmwrite32(VMCS_GUEST_LDTR_ACCESS_RIGHTS, AR_UNUSABLE);
  vmwrite32(VMCS_GUEST_TR_ACCESS_RIGHTS, AR_TR_BUSY_64);

  vmwrite32(VMCS_GUEST_CS_LIMIT, 0xFFFFF);
  vmwrite32(VMCS_GUEST_SS_LIMIT, 0xFFFFF);
  vmwrite32(VMCS_GUEST_DS_LIMIT, 0xFFFFF);
  vmwrite32(VMCS_GUEST_ES_LIMIT, 0xFFFFF);
  vmwrite32(VMCS_GUEST_FS_LIMIT, 0xFFFFF);
  vmwrite32(VMCS_GUEST_GS_LIMIT, 0xFFFFF);
  vmwrite32(VMCS_GUEST_LDTR_LIMIT, 0xFFFFF);
  vmwrite32(VMCS_GUEST_TR_LIMIT, 0x6F);


  vmwrite(VMCS_GUEST_CS_BASE, 0);
  vmwrite(VMCS_GUEST_SS_BASE, 0);
  vmwrite(VMCS_GUEST_DS_BASE, 0);
  vmwrite(VMCS_GUEST_ES_BASE, 0);
  vmwrite(VMCS_GUEST_FS_BASE, 0);
  vmwrite(VMCS_GUEST_GS_BASE, 0);
  vmwrite(VMCS_GUEST_LDTR_BASE, 0);
  vmwrite(VMCS_GUEST_TR_BASE, 0);
  vmwrite(VMCS_GUEST_GDTR_BASE, 0);
  vmwrite(VMCS_GUEST_IDTR_BASE, 0);


  vmwrite(VMCS_GUEST_RIP, ox_guest_rip());
  vmwrite(VMCS_GUEST_RSP, ox_host_stack_top_addr());
  vmwrite(VMCS_GUEST_RFLAGS, 0x2);
  vmwrite(VMCS_GUEST_DR7, 0);
  vmwrite(VMCS_GUEST_SYSENTER_CS, 0);
  vmwrite(VMCS_GUEST_SYSENTER_ESP, 0);
  vmwrite(VMCS_GUEST_SYSENTER_EIP, 0);


  vmwrite(VMCS_GUEST_VMCS_LINK_PTR, 0xFFFFFFFFFFFFFFFF);


  let launched = vmlaunch();
  if launched != 0 {
    let err = vmx_instruct_error();
    serial_puts(str_ptr("[oxide-hv] vmlaunch FAILED\n"));
    return (err & 0xFF) | 0x100;
  }
  return 0;
}


fn guest_payload() {
  asm!("mov $$0x41, %rax\nvmcall\nmov $$0x42, %rax\nvmcall\nmov $$0x43, %rax\nvmcall\nhlt",
       sideeffect=true);
}


fn advance_guest_rip() {
  let len = vmread(VMCS_VMEXIT_INSTRUCTION_LENGTH);
  let rip = vmread(VMCS_GUEST_RIP);
  vmwrite(VMCS_GUEST_RIP, rip + len);
}


fn service_vmcall(g: &i64) {
  let arg: &i64 = g + 0;
  let val: i64 = mmio_load(arg);
  if val != 0 {
    serial_putc((val & 0xFF) as u8);
  }
  advance_guest_rip();
}


fn vmexit_c_handler(g: &i64) -> i64 {
  let reason = vmread(VMCS_EXIT_REASON) & 0xFFFF;
  if reason == EXIT_REASON_VMCALL {
    service_vmcall(g);
    return 0;
  } else if reason == EXIT_REASON_HLT {
    serial_puts(str_ptr("\n[oxide-hv] guest HLT -> clean shutdown\n"));
    return 1;
  } else if reason == EXIT_REASON_TRIPLE_FAULT {
    serial_puts(str_ptr("[oxide-hv] TRIPLE FAULT\n"));
    return 1;
  } else {
    serial_puts(str_ptr("[oxide-hv] unhandled exit reason\n"));
    return 1;
  }
  return 1;
}


fn vmexit_done() {
  serial_puts(str_ptr("[oxide-hv] host shutting down after guest exit\n"));
}


