// hv/src/vmx.ox - VMX capability MSRs + VMCS field encodings (Intel SDM Vol 3C).
//
// Everything here is compile-time constants grouped per the SDM, plus the
// capability-adjust helper the SDM mandates for every control field. The
// constants are the exact 32-bit field encodings from Appendix B; a wrong
// encoding reads/writes the wrong VMCS field, so they are not abbreviated.
//
// This module is freestanding-clean: only the kernel's rdmsr/wrmsr helpers are
// called, plus mmio_store for VMCS writes. No ox_* runtime.

// ----------------------------------------------------------------------------
// VMX capability MSRs (SDM Vol 3A, Table 2-2; Vol 3C Appendix A)
// ----------------------------------------------------------------------------
const MSR_IA32_VMX_BASIC            = 0x480;
const MSR_IA32_VMX_PINBASED_CTLS    = 0x481;
const MSR_IA32_VMX_PROCBASED_CTLS   = 0x482;   // primary proc-based
const MSR_IA32_VMX_EXIT_CTLS        = 0x483;
const MSR_IA32_VMX_ENTRY_CTLS       = 0x484;
const MSR_IA32_VMX_TRUE_PINBASED    = 0x48D;
const MSR_IA32_VMX_TRUE_PROCBASED   = 0x48E;
const MSR_IA32_VMX_TRUE_EXIT        = 0x48F;
const MSR_IA32_VMX_TRUE_ENTRY       = 0x490;
const MSR_IA32_VMX_PROCBASED2_CTLS  = 0x491;   // secondary proc-based

// IA32_VMX_BASIC bits (read once; drives VMCS revision id + VMX width)
const VMX_BASIC_DUAL_MONITOR_BIT  = 1 << 49;   // bit 49: dual-monitor treatment
const VMX_BASIC_MEM_TYPE_WB       = 6;          // bits 53:50 = mem type (6 = WB)
const VMX_BASIC_INS_OUTS_BIT      = 1 << 54;   // bit 54: VM-exit on ins/outs
const VMX_BASIC_TRUE_CTLS_BIT     = 1 << 55;   // bit 55: TRUE_* _CTLS available

// ----------------------------------------------------------------------------
// VMCS field encodings (SDM Vol 3C, Appendix B). 32-bit encodings.
// ----------------------------------------------------------------------------
// --- 16-bit guest-state fields (high half = 0x0000) ---
const VMCS_GUEST_CS_SEL          = 0x0802;
const VMCS_GUEST_SS_SEL          = 0x0804;
const VMCS_GUEST_DS_SEL          = 0x0806;
const VMCS_GUEST_ES_SEL          = 0x0800;
const VMCS_GUEST_FS_SEL          = 0x0808;
const VMCS_GUEST_GS_SEL          = 0x080A;
const VMCS_GUEST_LDTR_SEL        = 0x080C;
const VMCS_GUEST_TR_SEL          = 0x080E;

// --- 16-bit control fields ---
const VMCS_VPID                  = 0x0000;       // virtual-processor id (18 bits)

// --- 16-bit host-state fields ---
const VMCS_HOST_CS_SEL           = 0x0C02;
const VMCS_HOST_SS_SEL           = 0x0C04;
const VMCS_HOST_DS_SEL           = 0x0C06;
const VMCS_HOST_ES_SEL           = 0x0C00;
const VMCS_HOST_FS_SEL           = 0x0C08;
const VMCS_HOST_GS_SEL           = 0x0C0A;
const VMCS_HOST_TR_SEL           = 0x0C0C;

// --- 64-bit control fields ---
const VMCS_ADDR_IO_BITMAP_A      = 0x2000;
const VMCS_ADDR_IO_BITMAP_B      = 0x2002;
const VMCS_ADDR_MSR_BITMAP       = 0x2004;
const VMCS_EXECUTIVE_VMCS_PTR    = 0x200C;

// --- 64-bit read-only data fields ---
const VMCS_GUEST_PHYSICAL_ADDR   = 0x2400;

// --- 64-bit guest-state fields ---
const VMCS_GUEST_VMCS_LINK_PTR   = 0x2800;       // must be ~0 for basic VMX
const VMCS_GUEST_IA32_EFER       = 0x2806;
const VMCS_GUEST_PDPTE_BASE0     = 0x280A;       // PDPTE0..3 (PAE paging guest)
const VMCS_GUEST_PDPTE_BASE1     = 0x280C;
const VMCS_GUEST_PDPTE_BASE2     = 0x280E;
const VMCS_GUEST_PDPTE_BASE3     = 0x2810;

// --- 32-bit control fields ---
const VMCS_PIN_BASED             = 0x4000;       // pin-based controls
const VMCS_PROC_BASED            = 0x4002;       // primary proc-based controls
const VMCS_EXC_BITMAP            = 0x4004;
const VMCS_PROC_BASED2           = 0x401E;       // secondary proc-based controls
const VMCS_EXIT_CTLS             = 0x400C;
const VMCS_EXIT_MSR_STORE_COUNT  = 0x400E;
const VMCS_EXIT_MSR_LOAD_COUNT   = 0x4010;
const VMCS_ENTRY_CTLS            = 0x4012;
const VMCS_ENTRY_MSR_LOAD_COUNT = 0x4014;
const VMCS_ENTRY_INTERRUPTION_INFO = 0x4016;
const VMCS_ENTRY_EXCEPTION_ERRORCODE = 0x4018;
const VMCS_ENTRY_INSTRUCTION_LENGTH = 0x401A;

// --- 32-bit read-only data fields ---
const VMCS_INSTRUCTION_ERROR     = 0x4400;
const VMCS_EXIT_REASON          = 0x4402;
const VMCS_VMEXIT_INSTRUCTION_LENGTH = 0x440C;
const VMCS_VMEXIT_INTERRUPTION_INFO   = 0x4404;

// --- 32-bit guest-state fields ---
const VMCS_GUEST_CS_LIMIT        = 0x4802;
const VMCS_GUEST_SS_LIMIT        = 0x4804;
const VMCS_GUEST_DS_LIMIT        = 0x4806;
const VMCS_GUEST_ES_LIMIT        = 0x4800;
const VMCS_GUEST_FS_LIMIT        = 0x4808;
const VMCS_GUEST_GS_LIMIT        = 0x480A;
const VMCS_GUEST_LDTR_LIMIT      = 0x480C;
const VMCS_GUEST_TR_LIMIT        = 0x480E;
const VMCS_GUEST_CS_ACCESS_RIGHTS  = 0x4816;
const VMCS_GUEST_SS_ACCESS_RIGHTS  = 0x4818;
const VMCS_GUEST_DS_ACCESS_RIGHTS  = 0x481A;
const VMCS_GUEST_ES_ACCESS_RIGHTS  = 0x4814;
const VMCS_GUEST_FS_ACCESS_RIGHTS  = 0x481C;
const VMCS_GUEST_GS_ACCESS_RIGHTS  = 0x481E;
const VMCS_GUEST_LDTR_ACCESS_RIGHTS = 0x4820;
const VMCS_GUEST_TR_ACCESS_RIGHTS  = 0x4822;
const VMCS_GUEST_INTERRUPT_STATUS = 0x4824;
const VMCS_GUEST_SYSENTER_CS      = 0x482A;

// --- 32-bit host-state fields ---
const VMCS_HOST_SYSENTER_CS       = 0x4C00;

// --- natural-width (64 on x86-64) control fields ---
const VMCS_CR0_GUEST_HOST_MASK    = 0x6000;
const VMCS_CR4_GUEST_HOST_MASK    = 0x6002;
const VMCS_CR0_READ_SHADOW        = 0x6004;
const VMCS_CR4_READ_SHADOW        = 0x6006;

// --- natural-width guest-state fields ---
const VMCS_GUEST_CR0              = 0x6800;
const VMCS_GUEST_CR3              = 0x6802;
const VMCS_GUEST_CR4              = 0x6804;
const VMCS_GUEST_ES_BASE          = 0x6806;
const VMCS_GUEST_CS_BASE          = 0x6808;
const VMCS_GUEST_SS_BASE          = 0x680A;
const VMCS_GUEST_DS_BASE          = 0x680C;
const VMCS_GUEST_FS_BASE          = 0x680E;
const VMCS_GUEST_GS_BASE          = 0x6810;
const VMCS_GUEST_LDTR_BASE        = 0x6812;
const VMCS_GUEST_TR_BASE          = 0x6814;
const VMCS_GUEST_GDTR_BASE        = 0x6816;
const VMCS_GUEST_IDTR_BASE        = 0x6818;
const VMCS_GUEST_DR7              = 0x681A;
const VMCS_GUEST_RSP              = 0x681C;
const VMCS_GUEST_RIP              = 0x681E;
const VMCS_GUEST_RFLAGS           = 0x6820;
const VMCS_GUEST_SYSENTER_ESP     = 0x6824;
const VMCS_GUEST_SYSENTER_EIP     = 0x6826;

// --- natural-width host-state fields ---
const VMCS_HOST_CR0               = 0x6C00;
const VMCS_HOST_CR3               = 0x6C02;
const VMCS_HOST_CR4               = 0x6C04;
const VMCS_HOST_FS_BASE           = 0x6C06;
const VMCS_HOST_GS_BASE           = 0x6C08;
const VMCS_HOST_TR_BASE           = 0x6C0A;
const VMCS_HOST_GDTR_BASE         = 0x6C0C;
const VMCS_HOST_IDTR_BASE         = 0x6C0E;
const VMCS_HOST_SYSENTER_ESP      = 0x6C10;
const VMCS_HOST_SYSENTER_EIP      = 0x6C12;
const VMCS_HOST_RSP               = 0x6C14;
const VMCS_HOST_RIP               = 0x6C16;   // host RIP: ia host code after vmexit

// ----------------------------------------------------------------------------
// Access-rights encoding helpers (SDM 22.2.3): a 4000h, c000h, 7000h system/gate,
// present, DPL 0, granularity, big, etc. We only need a few real shapes.
//   P=1 S=? DPL=? type=? L=? D=? G=?  — bitfield laid out SDM Vol 3 Figure 24-8
// ----------------------------------------------------------------------------
// system segment (TR): 0x8b = present, type=0xb (busy 64-bit TSS), S=0.
const AR_TR_BUSY_64       = 0x8B;
// code segment: present, S=1 (code/data), type exec/read, DPL0, 64-bit (L=1)
const AR_CODE64_EXEC_READ_DPL0 = 0xA09B;   // P=1, DPL=0, S=1, type=A (exec/read, accessed)
const AR_CODE64_RO_DPL0   = 0x809B;   // P=1, DPL=0, S=1, non-accessed; we set accessed
// data segment: present, writable, type=3 (read/write, accessed)
const AR_DATA_RW_DPL0     = 0xC093;
// null/usable=0 segment selector access rights (Unusable bit = bit 16)
const AR_UNUSABLE         = 0x10000;

// ----------------------------------------------------------------------------
// Pin-based controls (SDM A.3.1 / Vol 3C 24.6.1)
// ----------------------------------------------------------------------------
const PIN_EXTERNAL_INTERRUPT_EXITING = 1 << 0;
const PIN_NMI_EXITING                  = 1 << 3;
const PIN_VIRTUAL_NMI                 = 1 << 5;
const PIN_PREEMPT_TIMER               = 1 << 6;
const PIN_POSTED_INTERRUPT            = 1 << 7;

// primary proc-based controls (SDM Vol 3C 24.6.2)
const CPU_BASED_USE_MSR_BITMAPS        = 1 << 28;
const CPU_BASED_ACTIVATE_SECONDARY     = 1 << 31;
const CPU_BASED_HLT_EXITING            = 1 << 7;
const CPU_BASED_INVLPG_EXITING         = 1 << 9;
const CPU_BASED_MOV_DR_EXITING         = 1 << 23;
const CPU_BASED_UNCONDITIONAL_IO_EXITING = 1 << 24;
const CPU_BASED_USE_IO_BITMAPS         = 1 << 25;

// secondary proc-based controls (SDM Vol 3C 24.6.2)
const CPU_BASED2_ENABLE_EPT            = 1 << 1;
const CPU_BASED2_ENABLE_VPID           = 1 << 0;
const CPU_BASED2_UNRESTRICTED_GUEST     = 1 << 7;
const CPU_BASED2_ENABLE_RDTSCP          = 1 << 3;
const CPU_BASED2_ENABLE_VIRT_EXCEPTIONS = 1 << 4;
const CPU_BASED2_ENABLE_INVPCID         = 1 << 12;

// VM-exit controls (SDM Vol 3C 24.7.1)
const VM_EXIT_SAVE_DEBUG_CONTROLS      = 1 << 2;
const VM_EXIT_HOST_ADDR_SPACE_SIZE     = 1 << 9;
const VM_EXIT_ACK_INTR_ON_EXIT         = 1 << 15;
const VM_EXIT_SAVE_IA32_EFER           = 1 << 20;
const VM_EXIT_LOAD_IA32_EFER           = 1 << 21;

// VM-entry controls (SDM Vol 3C 24.8.1)
const VM_ENTRY_LOAD_DEBUG_CONTROLS     = 1 << 2;
const VM_ENTRY_IA32E_MODE              = 1 << 9;
const VM_ENTRY_LOAD_IA32_EFER          = 1 << 15;

// ----------------------------------------------------------------------------
// VM-exit reasons (SDM Vol 3C Appendix C — Table C-1). The handler switches on
// these. We name the established set; a reason we don't expect lands in the
// unknown/abort arm with the reason number printed for diagnostics.
// ----------------------------------------------------------------------------
const EXIT_REASON_EXCEPTION_NMI       = 0;
const EXIT_REASON_EXTERNAL_INTERRUPT  = 1;
const EXIT_REASON_TRIPLE_FAULT        = 2;
const EXIT_REASON_INIT_SIGNAL          = 3;
const EXIT_REASON_TASK_SWITCH            = 9;
const EXIT_REASON_CPUID                  = 10;
const EXIT_REASON_HLT                   = 12;   // the HLT exit reason (canonical)
const EXIT_REASON_INVD                   = 13;
const EXIT_REASON_INVLPG                 = 14;
const EXIT_REASON_RDPMC                  = 15;
const EXIT_REASON_RDTSC                  = 16;
const EXIT_REASON_VMCALL                 = 18;   // guest hypercall -> we service it
const EXIT_REASON_VMCLEAR                = 19;
const EXIT_REASON_VMLAUNCH               = 20;
const EXIT_REASON_VMPTRLD                = 21;
const EXIT_REASON_VMPTRST                = 22;
const EXIT_REASON_VMREAD                 = 23;
const EXIT_REASON_VMRESUME               = 24;
const EXIT_REASON_VMWRITE                = 25;
const EXIT_REASON_VMXOFF                 = 26;
const EXIT_REASON_VMXON                   = 27;
const EXIT_REASON_CR_ACCESS               = 28;
const EXIT_REASON_DR_ACCESS               = 29;
const EXIT_REASON_IO_INSTRUCTION         = 30;
const EXIT_REASON_RDMSR                  = 31;
const EXIT_REASON_WRMSR                  = 32;
const EXIT_REASON_INVALID_GUEST_STATE    = 33;
const EXIT_REASON_MSR_LOADING             = 34;
const EXIT_REASON_MWAIT                    = 36;
const EXIT_REASON_MONITOR                  = 39;
const EXIT_REASON_PAUSE                    = 40;
const EXIT_REASON_MACHINE_CHECK            = 41;
const EXIT_REASON_APIC_ACCESS              = 44;
const EXIT_REASON_GDTR_OR_IDTR_ACCESS      = 46;
const EXIT_REASON_LDTR_OR_TR_ACCESS        = 47;
const EXIT_REASON_EPT_VIOLATION            = 48;
const EXIT_REASON_EPT_MISCONFIG            = 49;
const EXIT_REASON_INVEPT                   = 50;
const EXIT_REASON_VMX_PREEMPT_TIMER_EXPIRED = 52;
const EXIT_REASON_INVVPID                  = 53;
const EXIT_REASON_WBINVD                  = 54;
const EXIT_REASON_XSETBV                  = 55;

// ----------------------------------------------------------------------------
// VMCS field width by encoding:        SDM Vol 3C, Appendix B, field encoding
//   high half (bits 13:10 of the 14-bit encoding) selects the field WIDTH:
//     0000 = 16-bit,  0001 = 64-bit,  0010 = 32-bit,  0011 = natural-width,
//     0100 = 128-bit
//   The field's absolute encoding is 14 bits; we use the top 3 nibbles to drive
//   vmwrite width so we never write a 64-bit field as 32-bit (zeroes high half).
// ----------------------------------------------------------------------------
fn vmcs_field_width(enc: i64) -> i64 {
  // bits 13:10 of the 14-bit encoding = (enc >> 10) & 0xF
  return (enc >> 10) & 0xF;
}
// is a field a 64-bit or natural-width (8-byte) field?  -> use write64 path
fn vmcs_field_is_wide(enc: i64) -> bool {
  let w = vmcs_field_width(enc);
  return w == 1 || w == 3;   // 64-bit or natural-width on x86-64
}

// ----------------------------------------------------------------------------
// Capability-adjust helper (SDM A.3.1). For each control MSR, bits 31:0 are the
// allowed-1 settings (bits that MAY be set) and bits 63:32 are the allowed-0
// settings (bits that MAY be clear). A legal control value `want` is:
//     adjusted = (msr_lo | want) & msr_hi           ... i.e. default-1 set,
//   default-0 cleared, and only bits the CPU actually allows touched.
//   This is what makes the VMCS controls portable across CPU models: we never
//   set a bit the MSR forbids, and we keep every default-1 the MSR requires.
// ----------------------------------------------------------------------------
fn vmx_adjust_ctl(msr: i64, want: i64) -> i64 {
  let v = rdmsr(msr);
  let allowed1 = v & 0xFFFFFFFF;        // low 32 = allowed-1
  let allowed0 = (v >> 32) & 0xFFFFFFFF; // high 32 = allowed-0
  // (want | allowed1) sets all required-default-1 bits; & allowed0 keeps only
  // bits permitted to be 1. (allowed0 is the set of bits that may be 1, since
  // a bit may be 0 only if its allowed-0 allows it... ) Careful: a value is
  // legal iff (value & ~allowed1) == 0  AND  (~value & ~allowed0) == 0.
  // The standard folded form is:  v = (allowed1 & want) | (allowed0 & allowed1)
  //   == allowed1 & (want | ~allowed0)? Just use the well-known identity:
  //     adjusted = want & allowed1 | (~want & allowed0 & allowed1)?
  // Simpler and exact:  bits that are 1 in `want` must be in allowed1;
  // bits that are 0 in `want` should be 1 iff required (in allowed1 \ allowed0).
  //   adjusted = (want & allowed1) | (allowed1 & ~allowed0)
  let required1 = allowed1 & (~allowed0 & 0xFFFFFFFF); // default-1 required bits
  return ((want & allowed1) | required1) & 0xFFFFFFFF;
}
// hv/src/vmxops.ox - VMX instruction wrappers (vmclear/vmptrld/vmwrite/vmread/
// vmlaunch/vmresume/vmxoff/vmcall). Each lowers to its exact x86 encoding.
//
// VMX instruction status (SDM 30.2): the vm-ptr instructions and vmwrite/vmread
// set CF (VMfailInvalid) or ZF (VMfailValid) on failure; vmlaunch/vmresume/vmxoff
// likewise. Phase 3 keeps these wrappers simple (the SDM encodings are the
// critical correctness point); phase 4 will capture the flag word to surface a
// precise failure reason at callsites.
//
// Encoding notes (AT&T order: src, dst):
//   vmptrld/vmclear/vmxon  m64  - physical addr of region in the r/m operand
//   vmwrite  src=r/m (value), dst=r64 (field)  -> AT&T "vmwrite %rax, %rdx"
//        writes the value in %rdx to the field whose encoding is in %rax
//   vmread   src=r/m (field), dst=r64 (value)   -> AT&T "vmread %rdx, %rax"
//        reads field %rdx into %rax

// read the VM instruction-error field (0x4400) after a failed vm-entry. SDM 30.4.
fn vmx_instruct_error() -> i64 {
  let mut err: i64 = 0;
  asm!("vmread %rdx, %rax", in("{rdx}") 0x4400, out("{rax}") err);
  return err;
}

// vmclear m64 : initialize a VMCS region and mark it "clear" (the launch state
// needed before vmptrld/vmlaunch). phys addr of the 4 KiB VMCS in the operand.
fn vmclear(region_phys: i64) -> i64 {
  asm!("vmclear (%rax)", in("{rax}") region_phys, sideeffect=true);
  return 0;
}

// vmptrld m64 : load the current-VMCS pointer (the VMCS subsequent vmwrite/
// vmlaunch act on).
fn vmptrld(region_phys: i64) -> i64 {
  asm!("vmptrld (%rax)", in("{rax}") region_phys, sideeffect=true);
  return 0;
}

// vmwrite field value : write `value` to the current VMCS field `field`.
// field encoding goes in %rax; value in %rdx.
fn vmwrite(field: i64, value: i64) {
  asm!("vmwrite %rax, %rdx", in("{rax}") field, in("{rdx}") value, sideeffect=true);
}

// vmwrite for control fields: mask to 32 bits (high bits written as zero, which
// is correct for every control field we set).
fn vmwrite32(field: i64, value: i64) {
  vmwrite(field, value & 0xFFFFFFFF);
}

// vmread field -> value : read the current VMCS field.
fn vmread(field: i64) -> i64 {
  let mut v: i64 = 0;
  asm!("vmread %rdx, %rax", in("{rdx}") field, out("{rax}") v);
  return v;
}

// vmlaunch : launch the guest in the current VMCS. On success control transfers
// to the guest and we do NOT return here; on failure we fall through (CF/ZF set
// + instruction-error in the VMCS). Returning 1 from here means vmlaunch FAILED.
fn vmlaunch() -> i64 {
  let mut rc: i64 = 0;
  asm!("vmlaunch", out("{rax}") rc);   // rax r/v on the fall-through path
  return 1;
}

// vmresume : resume the guest after a VM-exit (phase 4's exit loop). Returning 1
// means the resume failed.
fn vmresume() -> i64 {
  let mut rc: i64 = 0;
  asm!("vmresume", out("{rax}") rc);
  return 1;
}

// vmxoff : leave VMX root operation (clean teardown).
fn vmxoff() -> i64 {
  asm!("vmxoff", sideeffect=true);
  return 0;
}

// vmcall : the guest hypercall - issues a VM-exit (reason 18). Called only from
// inside the guest; including it here so the guest payload code can use it.
fn vmcall() {
  asm!("vmcall", sideeffect=true);
}
// hv/src/vmlaunch.ox - build a VMCS and launch a minimal 64-bit guest.
//
// The guest runs in 64-bit long mode (paging on, identity-mapped low 2 MiB --
// the same map stub.S set up for the host). Its payload issues `vmcall`
// (hypercall) then, should it ever be resumed, spins. vmcall forces a VM-exit
// that phase 4 will decode; for phase 3 the goal is a correctly-loaded VMCS
// whose vmlaunch passes the SDM 26.3.1 guest-state + 26.2 host-state checks and
// transfers control to the guest -- verified by correct machine code and
// SDM-accurate structures, NOT by booting (no QEMU here).
//
// Every VMCS field below is set per Intel SDM Vol 3C ch 23-26. Control fields
// pass through vmx_adjust_ctl so only CPU-allowed bits are touched.

// a VMCS region: 4 KiB; dword 0 = VMCS revision id (IA32_VMX_BASIC[30:0]),
// dword 1 = VMX-abort indicator (0). Zero-init then stamp the rev id.
let mut vmcs_region: [u8; 0x1000];

// host VM-exit stack (RSP loaded on every VM-exit by the VMCS host-RSP field).
let mut host_stack: [u8; 0x2000];

// ---- address helpers defined in stub.S -> vmcs_syms.S (Oxide can't take a
// named fn's address as a value; these C-callable asm fns return it instead). ----
extern fn ox_host_rip() -> i64;
extern fn ox_guest_rip() -> i64;
extern fn ox_host_stack_top_addr() -> i64;

// vmwrite a 16-bit field (selector fields). The field encoding is the same
// natural/16 width; vmwrite the zero-extended 16-bit value.
fn vmwrite16(field: i64, value: i64) {
  vmwrite(field, value & 0xFFFF);
}

// guest CR3 = the host's PML4 base (guest shares the identity map). Read CR3 and
// mask the low 12 PCID bits. (kernel.ox has write_cr3 but not a read.)
fn read_cr3_phys() -> i64 {
  let mut v: i64 = 0;
  asm!("mov %cr3, %rax", out("{rax}") v);
  return v & 0xFFFFFFFFFFFFF000;
}

// build + launch the VMCS. Returns 0 on success (we never get here -- control
// leaves to the guest), nonzero on a detectable failure (with instruction-error
// reason in the low bits).
fn launch_guest() -> i64 {
  // 1. VMCS revision id from IA32_VMX_BASIC[30:0]; stamp dword 0, zero dword 1.
  let basic = rdmsr(MSR_IA32_VMX_BASIC);
  let rev: u32 = (basic & 0x7FFFFFFF) as u32;
  let vmcs_ptr: &u8 = &vmcs_region[0];
  let rev_ptr: &u32 = vmcs_ptr as &u32;
  mmio_store(rev_ptr, rev);
  let abort_ptr: &u32 = (vmcs_ptr + 4) as &u32;
  mmio_store(abort_ptr, 0);

  // 2. VMCLEAR then VMPTRLD (physical addr == virt under the identity map).
  let vmcs_phys: i64 = vmcs_ptr as usize;
  vmclear(vmcs_phys);
  vmptrld(vmcs_phys);

  // 3. host-state area (SDM 24.5): loaded on every VM-exit. Host RIP/RSP land at
  // the real Oxide handler + the host exit stack top.
  vmwrite(VMCS_HOST_RIP, ox_host_rip());
  vmwrite(VMCS_HOST_RSP, ox_host_stack_top_addr());
  // host segment selectors: reuse the host GDT from stub.S (0x08 code, 0x10 data).
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
  vmwrite(VMCS_HOST_GS_BASE, rdmsr(0xC0000101));   // IA32_GS_BASE
  // GDTR/IDTR/TR base: phase 3 sets a minimal-but-present host. A full host loads
  // a real IDT on exit; phase 4 does that. For now 0 (no host interrupts taken
  // before phase 4 sets the IDT).
  vmwrite(VMCS_HOST_GDTR_BASE, 0);
  vmwrite(VMCS_HOST_IDTR_BASE, 0);
  vmwrite(VMCS_HOST_TR_BASE,  0);
  vmwrite(VMCS_HOST_SYSENTER_CS, 0);
  vmwrite(VMCS_HOST_SYSENTER_ESP, 0);
  vmwrite(VMCS_HOST_SYSENTER_EIP, 0);

  // 4. VM-exit controls (SDM 24.7), adjusted vs the cap MSR. Host-addr-space-size
  // keeps the host in 64-bit; save+load EFER so the host's EFER is restored.
  let exit_ctl = vmx_adjust_ctl(MSR_IA32_VMX_EXIT_CTLS,
        VM_EXIT_HOST_ADDR_SPACE_SIZE | VM_EXIT_SAVE_IA32_EFER
      | VM_EXIT_LOAD_IA32_EFER);
  vmwrite32(VMCS_EXIT_CTLS, exit_ctl);

  // 5. VM-entry controls (SDM 24.8): IA32e-mode guest (64-bit) + load EFER.
  let entry_ctl = vmx_adjust_ctl(MSR_IA32_VMX_ENTRY_CTLS,
        VM_ENTRY_IA32E_MODE | VM_ENTRY_LOAD_IA32_EFER);
  vmwrite32(VMCS_ENTRY_CTLS, entry_ctl);

  // 6. pin + proc controls (SDM 24.6). HLT exiting + activate secondary (for a
  // future EPT/VPID; phase 3 disables them in proc2).
  let pin = vmx_adjust_ctl(MSR_IA32_VMX_PINBASED_CTLS, 0);
  vmwrite32(VMCS_PIN_BASED, pin);
  let proc = vmx_adjust_ctl(MSR_IA32_VMX_PROCBASED_CTLS,
        CPU_BASED_HLT_EXITING | CPU_BASED_ACTIVATE_SECONDARY);
  vmwrite32(VMCS_PROC_BASED, proc);
  let proc2 = vmx_adjust_ctl(MSR_IA32_VMX_PROCBASED2_CTLS, 0);
  vmwrite32(VMCS_PROC_BASED2, proc2);
  vmwrite32(VMCS_EXC_BITMAP, 0);   // no exceptions vm-exit

  // 7. guest control registers + CR0/CR4 guest/host masks + read shadows. The
  // guest shares CR bits with the host; mask=0 + shadow=value means guest CR
  // reads return the stored value and writes are owned by the host.
  //   CR0: PE+PG+ET+NE set, CD+NW clear -> 0x80000033 (PG=31, ET=4, NE=5, PE=0).
  let gcr0 = 0x80000033;
  vmwrite(VMCS_GUEST_CR0, gcr0);
  vmwrite(VMCS_CR0_GUEST_HOST_MASK, 0);
  vmwrite(VMCS_CR0_READ_SHADOW, gcr0);
  vmwrite(VMCS_GUEST_CR3, read_cr3_phys());
  //   CR4: guest needs PAE (bit5) for long mode; VMXE (bit13) MUST be 0 in guest.
  let gcr4 = (1 as i64) << 5;
  vmwrite(VMCS_GUEST_CR4, gcr4);
  vmwrite(VMCS_CR4_GUEST_HOST_MASK, 0);
  vmwrite(VMCS_CR4_READ_SHADOW, gcr4);

  // 8. guest EFER: LME+LMA set for a 64-bit guest (bits 8+10 => 0x300).
  vmwrite(VMCS_GUEST_IA32_EFER, 0x300);

  // 9. guest segment state: CS/SS/DS/ES present (reuse host GDT selectors 0x08/
  // 0x10); FS/GS/LDTR unusable; TR a busy 64-bit TSS at selector 0x18.
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
  // limits: CS/SS/DS/ES 0xFFFFF (G=1 => 4 GiB); TR >= 0x67 (SDM 26.3.1.2) => 0x6F.
  vmwrite32(VMCS_GUEST_CS_LIMIT, 0xFFFFF);
  vmwrite32(VMCS_GUEST_SS_LIMIT, 0xFFFFF);
  vmwrite32(VMCS_GUEST_DS_LIMIT, 0xFFFFF);
  vmwrite32(VMCS_GUEST_ES_LIMIT, 0xFFFFF);
  vmwrite32(VMCS_GUEST_FS_LIMIT, 0xFFFFF);
  vmwrite32(VMCS_GUEST_GS_LIMIT, 0xFFFFF);
  vmwrite32(VMCS_GUEST_LDTR_LIMIT, 0xFFFFF);
  vmwrite32(VMCS_GUEST_TR_LIMIT, 0x6F);
  // bases: flat (0). long mode ignores CS/SS base; TR/GDTR/IDTR 0 (no TSS/GDT
  // used by this minimal guest).
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

  // 10. guest GP regs. RIP = the guest payload (vmcall; jmp self). RSP = the
  // host stack top (shared; the tiny guest never pushes). RFLAGS: bit 1 set
  // (reserved-1), IF clear (no guest interrupts yet).
  vmwrite(VMCS_GUEST_RIP, ox_guest_rip());
  vmwrite(VMCS_GUEST_RSP, ox_host_stack_top_addr());
  vmwrite(VMCS_GUEST_RFLAGS, 0x2);
  vmwrite(VMCS_GUEST_DR7, 0);
  vmwrite(VMCS_GUEST_SYSENTER_CS, 0);
  vmwrite(VMCS_GUEST_SYSENTER_ESP, 0);
  vmwrite(VMCS_GUEST_SYSENTER_EIP, 0);

  // 11. VMCS link ptr: ~0 (no nested VMX). SDM 24.4.2.
  vmwrite(VMCS_GUEST_VMCS_LINK_PTR, 0xFFFFFFFFFFFFFFFF);

  // 12. launch. On success control leaves to the guest; the fall-through means
  // vm-entry failed, with the instruction-error field set in the VMCS.
  let launched = vmlaunch();
  if launched != 0 {
    let err = vmx_instruct_error();
    serial_puts(str_ptr("[oxide-hv] vmlaunch FAILED\n"));
    return (err & 0xFF) | 0x100;   // nonzero, with the error reason
  }
  return 0;
}

// the guest payload: the guest sets a hypercall argument in %rax, issues
// `vmcall` (which VM-exits to the host with reason 18), is resumed, repeats with
// a different argument, then `hlt` (reason 12) for a clean shutdown. This is a
// full guest->host hypercall round trip: the guest runs in the VM, hypercalls
// out, the host services it, and resumes the guest. The arguments (0x41 'A',
// 0x42 'B', 0x43 'C') are what the host writes to COM1 as proof of execution.
//
// (`hlt` exits because we set CPU_BASED_HLT_EXITING; the byte 0xF4 in the guest.)
fn guest_payload() {
  asm!("mov $$0x41, %rax\nvmcall\nmov $$0x42, %rax\nvmcall\nmov $$0x43, %rax\nvmcall\nhlt",
       sideeffect=true);
}

// advance the guest past the instruction that caused the exit, by adding the
// VM-exit instruction-length to guest RIP. Required for vmcall/cpuid-style exits
// so the guest doesn't re-execute the exiting instruction forever.
fn advance_guest_rip() {
  let len = vmread(VMCS_VMEXIT_INSTRUCTION_LENGTH);
  let rip = vmread(VMCS_GUEST_RIP);
  vmwrite(VMCS_GUEST_RIP, rip + len);
}

// service a VMCALL hypercall: the guest's argument is in %rax, the first value
// the vmexit_entry stub pushed into the saved-gpr block `g`. Read it, write its
// low byte to COM1 as visible proof the guest ran inside the VM, then advance
// guest RIP past the vmcall.
fn service_vmcall(g: &i64) {
  let arg: &i64 = g + 0;   // saved rax is block[0]
  let val: i64 = mmio_load(arg);
  if val != 0 {
    serial_putc((val & 0xFF) as u8);
  }
  advance_guest_rip();
}

// vmexit_c_handler - the Oxide side of the VM-exit path. Called by vmexit_entry
// (vmcs_syms.S) with %rdi = pointer to the saved-gpr block (rax=block[0]...).
// Returns 0 = "resume the guest", nonzero = "halt the host". The stub restores
// GPRs and vmresumes only when we return 0; a terminal exit (HLT/triple/unknown)
// returns nonzero so the stub halts instead of resuming into the same exiting
// instruction forever. (The stub owns the GPR save/restore + the vmresume because
// Oxide fns have prologues that clobber registers and can't be the naked host_rip.)
fn vmexit_c_handler(g: &i64) -> i64 {
  let reason = vmread(VMCS_EXIT_REASON) & 0xFFFF;   // low 16 = reason; bit 31 = intr-pending, masked
  if reason == EXIT_REASON_VMCALL {
    service_vmcall(g);          // write guest's arg to COM1, advance rip
    return 0;                   // resume the guest
  } else if reason == EXIT_REASON_HLT {
    serial_puts(str_ptr("\n[oxide-hv] guest HLT -> clean shutdown\n"));
    return 1;                   // terminal: stop, don't resume
  } else if reason == EXIT_REASON_TRIPLE_FAULT {
    serial_puts(str_ptr("[oxide-hv] TRIPLE FAULT\n"));
    return 1;
  } else {
    serial_puts(str_ptr("[oxide-hv] unhandled exit reason\n"));
    return 1;
  }
  return 1;
}

// vmexit_done - called by vmexit_entry's halt path after a terminal exit. Just
// announces the host is shutting down; the stub then halts the CPU.
fn vmexit_done() {
  serial_puts(str_ptr("[oxide-hv] host shutting down after guest exit\n"));
}




// hv/src/freestanding_runtime.ox
//
// Bare-metal implementations of the few Oxide runtime helpers that freestanding
// code still references. A hosted build links these against the C runtime in
// liboxide; a freestanding kernel must provide them itself (they cannot come
// from libc). Today only `ox_bounds_fail` is referenced (array indexing that
// lowers to a bounds check). We write a panic marker to COM1 and halt the CPU
// in raw asm -- keeping this self-contained so module order / forward
// declarations never matter.

// raw COM1 byte out: set up dx=0x3F8, al=byte, `out`.
fn ox_com1_panic_put(c: i64) {
  asm!("outb %al, %dx", in("{dx}") 0x3F8, in("{al}") c);
}

// the bounds-violation trap from generated array checks. idx/bound are the
// real values. Print a fixed marker, fold bound into it so the args are used
// (and a panic with bound==0 is distinguishable from one with bound!=0), hang.
fn ox_bounds_fail(idx: i64, bound: i64) {
  // KpL.. — "kernel panic locator", a fixed 5-byte sentinel.
  ox_com1_panic_put(0x4B);
  ox_com1_panic_put(0x70);
  ox_com1_panic_put(0x4C);
  ox_com1_panic_put(0x2E);
  ox_com1_panic_put(0x2E);
  // fold the args in so they're not dead: emit the low byte of (idx ^ bound)
  // as an extra locator nibble (cheap, and uses both params).
  ox_com1_panic_put((idx ^ bound) & 0xFF);
  // cli; hlt; ... forever.
  let mut i: i64 = 0;
  while i < 1000 {
    asm!("cli", sideeffect=true);
    asm!("hlt", sideeffect=true);
    i = i + 1;
  }
  while true { asm!("hlt", sideeffect=true); }
}
// hv/src/kernel.ox - Oxide long-mode hypervisor entry (phase 2).
//
// Reached after hv/boot/stub.S far-jumps here in 64-bit long mode. From this
// point everything is compiled Oxide (64-bit LLVM IR). This phase:
//   - set up a stack and a real GDT/IDT
//   - probe VT-x via cpuid
//   - set CR4.VMXE, read IA32_VMX_BASIC, build a VMXON region, vmxon
//   - print "VMX root entered\n" to the COM1 serial port as proof of execution
//
// "Works" for this phase = compiles to correct 64-bit x86 + the structures match
// the Intel SDM; booting under QEMU is blocked on you installing qemu-system-x86_64.

const COM1 = 0x3F8;   // legacy COM1 I/O port base

// ---- low-level privileged helpers (native CPU mode cr/cr4/msr) ----
// Oxide has no implicit numeric narrowing at call sites, so these take i64 and
// widen/narrow inside the asm via typed constraints. This keeps call sites clean.

// read a Model-Specific Register (ecx = msr) -> edx:eax packed into i64.
fn rdmsr(msr: i64) -> i64 {
  let mut lo: i64 = 0;
  let mut hi: i64 = 0;
  let ecx: i64 = msr;
  asm!("rdmsr", in("{ecx}") ecx, out("{eax}") lo, out("{edx}") hi);
  return (hi << 32) | lo;
}
// write edx:eax to an MSR. (no output; side-effect only.)
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
// write %rax -> %cr3 (page-table base); used later for EPT pointer reloads.
fn write_cr3(v: i64) { asm!("mov %rax, %cr3", in("{rax}") v); }

// outb(out dx, al) / inb(in al, dx). port goes in dx (16-bit), the byte in al.
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

// hforever-halt (should never be reached; the loop is intentional).
fn halt_forever() {
  // a real host would never return here; on a hang, halt + cli loop.
  asm!("cli", sideeffect=true);
  asm!("hlt", sideeffect=true);
  halt_forever();
}

// ---- COM1 serial console (proof of execution, no hosted runtime) ----

// COM1 line-control config: 38400 baud, 8N1. Writes the config registers via
// OUTB to the COM1 port offsets, exactly like a kernel's uart init.
fn serial_init() {
  outb(COM1 + 1, 0);    // disable interrupts
  outb(COM1 + 3, 0x80); // enable DLAB (set baud rate divisor)
  outb(COM1 + 0, 3);    // divisor low  (38400 baud = 3)
  outb(COM1 + 1, 0);    // divisor high
  outb(COM1 + 3, 3);    // 8 bits, no parity, one stop (clears DLAB)
  outb(COM1 + 2, 0xC7); // enable FIFO, clear them, 14-byte threshold
  outb(COM1 + 4, 0x0B); // IRQs enabled, RTS/DSR set
}
// poll the Line Status Register (offset 5) bit 5 = transmit-holding-empty.
fn serial_can_send() -> bool {
  return (inb(COM1 + 5) & 0x20) != 0;
}
fn serial_putc(c: u8) {
  // spin until the UART can take a byte, then write it.
  let mut guard = 0;
  while !serial_can_send() {
    guard = guard + 1;
    if guard > 100000 { return; }   // avoid an infinite spin in a dead UART
  }
  outb(COM1, c as i64);
}
fn serial_puts(s: &u8) {
  // walk a NUL-terminated byte string at s, printing each byte. Volatile loads
  // (mmio_load) keep the scan side-effect-clean and coalesce-proof.
  let mut p: &u8 = s;
  while true {
    let c: u8 = mmio_load(p);
    if c == 0 { return; }
    serial_putc(c);
    p = p + 1;
  }
}

// ---- VT-x detection ----

// cpuid.01H:ECX[5] == 1 means VMX is supported.
fn has_vmx() -> bool {
  let mut eax: i64 = 1;   // cpuid leaf (feature info)
  let mut ecx: i64 = 0;
  let mut ebx: i64 = 0;
  let mut edx: i64 = 0;
  // EAX is read (leaf) and written (max leaf) by cpuid -> use `inout` so it is
  // both seeded before and captured after. ECX bit 5 is the VMX feature bit.
  asm!("cpuid", inout("{eax}") eax, out("{ebx}") ebx,
                 out("{ecx}") ecx, out("{edx}") edx);
  return (ecx & (1 << 5)) != 0;
}

// ---- VMX root operation ----

// a VMXON region: 4 KiB; the first u32 must hold the VMCS revision id (bits 30:0
// of IA32_VMX_BASIC). Pin the region to a fixed object we zero + stamp.
let mut vmxon_region: [u8; 0x1000];

fn vmx_on() -> i64 {
  // CR4.VMXE (bit 13) must be set before vmxon.
  let cr4v = read_cr4();
  write_cr4(cr4v | (1 << 13));
  // revision id = IA32_VMX_BASIC[30:0].
  let basic = rdmsr(MSR_IA32_VMX_BASIC);
  let rev: u32 = (basic & 0x7FFFFFFF) as u32;
  // stamp the revision id into the first 4 bytes of the VMXON region.
  // (u32 store at a u8 pointer with a cast through a *u32.)
  let region_ptr: &u8 = &vmxon_region[0];
  let rev_ptr: &u32 = region_ptr as &u32;
  mmio_store(rev_ptr, rev);
  // vmxon m64: opcode 0F C7 /6 r/m64. %rax holds the physical address of the
  // VMXON region. We identity-map the low 2 MiB in stub.S, so the virtual
  // address == physical address. vmxon sets ZF: 0 on success, 1 on fail (we'd
  // branch on it in a fuller host; for phase 2 we proceed and inspect later).
  let region_phys: i64 = region_ptr as usize;
  asm!("vmxon (%rax)", in("{rax}") region_phys, sideeffect=true);
  return 0;
}

// a small static string the kernel prints as proof.
const MSG_BOOT: str = "[oxide-hv] long-mode entry reached\n";

// ---- the long-mode entry symbol (reached from stub.S's far jump) ----
// This is the FIRST Oxide function to run. stub.S far-jumps here once long mode
// is active. We mark it extern-free: it is just a normal Oxide fn whose name the
// linker resolves (entry symbol `--entry oxide_long_mode_entry`).
fn oxide_long_mode_entry() -> i64 {
  // set up the host stack: stub.S reserved 16 KiB at `stack_top` (symbol in
  // .bss). ox_entry_stack_top_addr (an asm helper, see vmcs_syms.S) returns its
  // address; load it into rsp. (Oxide cannot take a bare symbol's address as a
  // value, so the boot + VMCS-launch helpers live in the .S file.)
  let sp = ox_entry_stack_top_addr();
  asm!("mov %rax, %rsp", in("{rax}") sp);

  // load the task register (TR) with the 0x18 TSS descriptor stub.S added to the
  // GDT. A valid host TR is required by VMX: on every VM-exit the CPU loads
  // host TR from the VMCS, and the entry TR must already be consistent. ltr
  // also marks the descriptor "busy".
  asm!("ltr %ax", in("{ax}") 0x18);

  // initialize the serial console and announce long-mode entry.
  serial_init();
  serial_puts(str_ptr(MSG_BOOT));

  // detect VT-x; if absent, print a marker and halt (no guest possible).
  if !has_vmx() {
    serial_puts(str_ptr("[oxide-hv] VT-x NOT available\n"));
    halt_forever();
  }

  // enter VMX root operation.
  let status = vmx_on();
  serial_puts(str_ptr("[oxide-hv] VMX root entered\n"));

  // phase 3: build the VMCS and vmlaunch a tiny guest. On success control
  // transfers to the guest (vmcall -> VM-exit -> host_vmexit_handler). If
  // launch_guest returns, vm-entry failed; report and halt.
  let launch_rc = launch_guest();
  serial_puts(str_ptr("[oxide-hv] vm-entry failed\n"));
  halt_forever();
  return 0;
}

// the asm-symbol address helper used above (defined in vmcs_syms.S).
extern fn ox_entry_stack_top_addr() -> i64;
