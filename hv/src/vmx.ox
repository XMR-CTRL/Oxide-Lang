


const MSR_IA32_VMX_BASIC            = 0x480;
const MSR_IA32_VMX_PINBASED_CTLS    = 0x481;
const MSR_IA32_VMX_PROCBASED_CTLS   = 0x482;
const MSR_IA32_VMX_EXIT_CTLS        = 0x483;
const MSR_IA32_VMX_ENTRY_CTLS       = 0x484;
const MSR_IA32_VMX_TRUE_PINBASED    = 0x48D;
const MSR_IA32_VMX_TRUE_PROCBASED   = 0x48E;
const MSR_IA32_VMX_TRUE_EXIT        = 0x48F;
const MSR_IA32_VMX_TRUE_ENTRY       = 0x490;
const MSR_IA32_VMX_PROCBASED2_CTLS  = 0x491;


const VMX_BASIC_DUAL_MONITOR_BIT  = 1 << 49;
const VMX_BASIC_MEM_TYPE_WB       = 6;
const VMX_BASIC_INS_OUTS_BIT      = 1 << 54;
const VMX_BASIC_TRUE_CTLS_BIT     = 1 << 55;


const VMCS_GUEST_CS_SEL          = 0x0802;
const VMCS_GUEST_SS_SEL          = 0x0804;
const VMCS_GUEST_DS_SEL          = 0x0806;
const VMCS_GUEST_ES_SEL          = 0x0800;
const VMCS_GUEST_FS_SEL          = 0x0808;
const VMCS_GUEST_GS_SEL          = 0x080A;
const VMCS_GUEST_LDTR_SEL        = 0x080C;
const VMCS_GUEST_TR_SEL          = 0x080E;


const VMCS_VPID                  = 0x0000;


const VMCS_HOST_CS_SEL           = 0x0C02;
const VMCS_HOST_SS_SEL           = 0x0C04;
const VMCS_HOST_DS_SEL           = 0x0C06;
const VMCS_HOST_ES_SEL           = 0x0C00;
const VMCS_HOST_FS_SEL           = 0x0C08;
const VMCS_HOST_GS_SEL           = 0x0C0A;
const VMCS_HOST_TR_SEL           = 0x0C0C;


const VMCS_ADDR_IO_BITMAP_A      = 0x2000;
const VMCS_ADDR_IO_BITMAP_B      = 0x2002;
const VMCS_ADDR_MSR_BITMAP       = 0x2004;
const VMCS_EXECUTIVE_VMCS_PTR    = 0x200C;


const VMCS_GUEST_PHYSICAL_ADDR   = 0x2400;


const VMCS_GUEST_VMCS_LINK_PTR   = 0x2800;
const VMCS_GUEST_IA32_EFER       = 0x2806;
const VMCS_GUEST_PDPTE_BASE0     = 0x280A;
const VMCS_GUEST_PDPTE_BASE1     = 0x280C;
const VMCS_GUEST_PDPTE_BASE2     = 0x280E;
const VMCS_GUEST_PDPTE_BASE3     = 0x2810;


const VMCS_PIN_BASED             = 0x4000;
const VMCS_PROC_BASED            = 0x4002;
const VMCS_EXC_BITMAP            = 0x4004;
const VMCS_PROC_BASED2           = 0x401E;
const VMCS_EXIT_CTLS             = 0x400C;
const VMCS_EXIT_MSR_STORE_COUNT  = 0x400E;
const VMCS_EXIT_MSR_LOAD_COUNT   = 0x4010;
const VMCS_ENTRY_CTLS            = 0x4012;
const VMCS_ENTRY_MSR_LOAD_COUNT = 0x4014;
const VMCS_ENTRY_INTERRUPTION_INFO = 0x4016;
const VMCS_ENTRY_EXCEPTION_ERRORCODE = 0x4018;
const VMCS_ENTRY_INSTRUCTION_LENGTH = 0x401A;


const VMCS_INSTRUCTION_ERROR     = 0x4400;
const VMCS_EXIT_REASON          = 0x4402;
const VMCS_VMEXIT_INSTRUCTION_LENGTH = 0x440C;
const VMCS_VMEXIT_INTERRUPTION_INFO   = 0x4404;


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


const VMCS_HOST_SYSENTER_CS       = 0x4C00;


const VMCS_CR0_GUEST_HOST_MASK    = 0x6000;
const VMCS_CR4_GUEST_HOST_MASK    = 0x6002;
const VMCS_CR0_READ_SHADOW        = 0x6004;
const VMCS_CR4_READ_SHADOW        = 0x6006;


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
const VMCS_HOST_RIP               = 0x6C16;


const AR_TR_BUSY_64       = 0x8B;

const AR_CODE64_EXEC_READ_DPL0 = 0xA09B;
const AR_CODE64_RO_DPL0   = 0x809B;

const AR_DATA_RW_DPL0     = 0xC093;

const AR_UNUSABLE         = 0x10000;


const PIN_EXTERNAL_INTERRUPT_EXITING = 1 << 0;
const PIN_NMI_EXITING                  = 1 << 3;
const PIN_VIRTUAL_NMI                 = 1 << 5;
const PIN_PREEMPT_TIMER               = 1 << 6;
const PIN_POSTED_INTERRUPT            = 1 << 7;


const CPU_BASED_USE_MSR_BITMAPS        = 1 << 28;
const CPU_BASED_ACTIVATE_SECONDARY     = 1 << 31;
const CPU_BASED_HLT_EXITING            = 1 << 7;
const CPU_BASED_INVLPG_EXITING         = 1 << 9;
const CPU_BASED_MOV_DR_EXITING         = 1 << 23;
const CPU_BASED_UNCONDITIONAL_IO_EXITING = 1 << 24;
const CPU_BASED_USE_IO_BITMAPS         = 1 << 25;


const CPU_BASED2_ENABLE_EPT            = 1 << 1;
const CPU_BASED2_ENABLE_VPID           = 1 << 0;
const CPU_BASED2_UNRESTRICTED_GUEST     = 1 << 7;
const CPU_BASED2_ENABLE_RDTSCP          = 1 << 3;
const CPU_BASED2_ENABLE_VIRT_EXCEPTIONS = 1 << 4;
const CPU_BASED2_ENABLE_INVPCID         = 1 << 12;


const VM_EXIT_SAVE_DEBUG_CONTROLS      = 1 << 2;
const VM_EXIT_HOST_ADDR_SPACE_SIZE     = 1 << 9;
const VM_EXIT_ACK_INTR_ON_EXIT         = 1 << 15;
const VM_EXIT_SAVE_IA32_EFER           = 1 << 20;
const VM_EXIT_LOAD_IA32_EFER           = 1 << 21;


const VM_ENTRY_LOAD_DEBUG_CONTROLS     = 1 << 2;
const VM_ENTRY_IA32E_MODE              = 1 << 9;
const VM_ENTRY_LOAD_IA32_EFER          = 1 << 15;


const EXIT_REASON_EXCEPTION_NMI       = 0;
const EXIT_REASON_EXTERNAL_INTERRUPT  = 1;
const EXIT_REASON_TRIPLE_FAULT        = 2;
const EXIT_REASON_INIT_SIGNAL          = 3;
const EXIT_REASON_TASK_SWITCH            = 9;
const EXIT_REASON_CPUID                  = 10;
const EXIT_REASON_HLT                   = 12;
const EXIT_REASON_INVD                   = 13;
const EXIT_REASON_INVLPG                 = 14;
const EXIT_REASON_RDPMC                  = 15;
const EXIT_REASON_RDTSC                  = 16;
const EXIT_REASON_VMCALL                 = 18;
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


fn vmcs_field_width(enc: i64) -> i64 {

  return (enc >> 10) & 0xF;
}

fn vmcs_field_is_wide(enc: i64) -> bool {
  let w = vmcs_field_width(enc);
  return w == 1 || w == 3;
}


fn vmx_adjust_ctl(msr: i64, want: i64) -> i64 {
  let v = rdmsr(msr);
  let allowed1 = v & 0xFFFFFFFF;
  let allowed0 = (v >> 32) & 0xFFFFFFFF;


  let required1 = allowed1 & (~allowed0 & 0xFFFFFFFF);
  return ((want & allowed1) | required1) & 0xFFFFFFFF;
}
