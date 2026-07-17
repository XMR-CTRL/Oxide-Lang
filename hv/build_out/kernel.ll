; oxide generated ir
target triple = "x86_64-elf"
declare i32 @ox_puts(i8*)
declare i32 @ox_puti(i64)
declare i32 @ox_putf(double)
declare i32 @ox_newline()
declare i32 @ox_putc(i64)
declare i64 @ox_abs_i64(i64)
declare double @llvm.fabs.f64(double)
declare double @ox_sqrt(double)
declare i64 @ox_imin(i64, i64)
declare i64 @ox_imax(i64, i64)
declare double @ox_fmin2(double, double)
declare double @ox_fmax2(double, double)
declare i8* @ox_itos(i64)
declare i64 @ox_stoi(i8*)
declare double @ox_stod(i8*)
declare i64 @ox_strlen(i8*)
declare i64 @ox_strcmp(i8*, i8*)
declare i8* @ox_substr(i8*, i64, i64)
declare i64 @ox_strchr(i8*, i64)
declare i8* @ox_char_str(i64)
declare i8* @ox_ftos(double)
declare i8* @ox_sb_new()
declare void @ox_sb_puts(i8*, i8*)
declare i8* @ox_sb_finish(i8*)
declare i8* @ox_read_line()
declare i8* @ox_read_file(i8*)
declare i64 @ox_file_open(i8*, i8*)
declare i64 @ox_file_close(i64)
declare i8* @ox_file_read(i64)
declare i64 @ox_file_write(i64, i8*)
declare i1 @ox_file_exists(i8*)

; extern functions
declare i64 @ox_entry_stack_top_addr()
declare i64 @ox_guest_rip()
declare i64 @ox_host_rip()
declare i64 @ox_host_stack_top_addr()

; top-level globals
@AR_CODE64_EXEC_READ_DPL0 = private constant i64 41115
@AR_CODE64_RO_DPL0 = private constant i64 32923
@AR_DATA_RW_DPL0 = private constant i64 49299
@AR_TR_BUSY_64 = private constant i64 139
@AR_UNUSABLE = private constant i64 65536
@COM1 = private constant i64 1016
@CPU_BASED2_ENABLE_EPT = private constant i64 2
@CPU_BASED2_ENABLE_INVPCID = private constant i64 4096
@CPU_BASED2_ENABLE_RDTSCP = private constant i64 8
@CPU_BASED2_ENABLE_VIRT_EXCEPTIONS = private constant i64 16
@CPU_BASED2_ENABLE_VPID = private constant i64 1
@CPU_BASED2_UNRESTRICTED_GUEST = private constant i64 128
@CPU_BASED_ACTIVATE_SECONDARY = private constant i64 2147483648
@CPU_BASED_HLT_EXITING = private constant i64 128
@CPU_BASED_INVLPG_EXITING = private constant i64 512
@CPU_BASED_MOV_DR_EXITING = private constant i64 8388608
@CPU_BASED_UNCONDITIONAL_IO_EXITING = private constant i64 16777216
@CPU_BASED_USE_IO_BITMAPS = private constant i64 33554432
@CPU_BASED_USE_MSR_BITMAPS = private constant i64 268435456
@EXIT_REASON_APIC_ACCESS = private constant i64 44
@EXIT_REASON_CPUID = private constant i64 10
@EXIT_REASON_CR_ACCESS = private constant i64 28
@EXIT_REASON_DR_ACCESS = private constant i64 29
@EXIT_REASON_EPT_MISCONFIG = private constant i64 49
@EXIT_REASON_EPT_VIOLATION = private constant i64 48
@EXIT_REASON_EXCEPTION_NMI = private constant i64 0
@EXIT_REASON_EXTERNAL_INTERRUPT = private constant i64 1
@EXIT_REASON_GDTR_OR_IDTR_ACCESS = private constant i64 46
@EXIT_REASON_HLT = private constant i64 12
@EXIT_REASON_INIT_SIGNAL = private constant i64 3
@EXIT_REASON_INVALID_GUEST_STATE = private constant i64 33
@EXIT_REASON_INVD = private constant i64 13
@EXIT_REASON_INVEPT = private constant i64 50
@EXIT_REASON_INVLPG = private constant i64 14
@EXIT_REASON_INVVPID = private constant i64 53
@EXIT_REASON_IO_INSTRUCTION = private constant i64 30
@EXIT_REASON_LDTR_OR_TR_ACCESS = private constant i64 47
@EXIT_REASON_MACHINE_CHECK = private constant i64 41
@EXIT_REASON_MONITOR = private constant i64 39
@EXIT_REASON_MSR_LOADING = private constant i64 34
@EXIT_REASON_MWAIT = private constant i64 36
@EXIT_REASON_PAUSE = private constant i64 40
@EXIT_REASON_RDMSR = private constant i64 31
@EXIT_REASON_RDPMC = private constant i64 15
@EXIT_REASON_RDTSC = private constant i64 16
@EXIT_REASON_TASK_SWITCH = private constant i64 9
@EXIT_REASON_TRIPLE_FAULT = private constant i64 2
@EXIT_REASON_VMCALL = private constant i64 18
@EXIT_REASON_VMCLEAR = private constant i64 19
@EXIT_REASON_VMLAUNCH = private constant i64 20
@EXIT_REASON_VMPTRLD = private constant i64 21
@EXIT_REASON_VMPTRST = private constant i64 22
@EXIT_REASON_VMREAD = private constant i64 23
@EXIT_REASON_VMRESUME = private constant i64 24
@EXIT_REASON_VMWRITE = private constant i64 25
@EXIT_REASON_VMXOFF = private constant i64 26
@EXIT_REASON_VMXON = private constant i64 27
@EXIT_REASON_VMX_PREEMPT_TIMER_EXPIRED = private constant i64 52
@EXIT_REASON_WBINVD = private constant i64 54
@EXIT_REASON_WRMSR = private constant i64 32
@EXIT_REASON_XSETBV = private constant i64 55
@MSG_BOOT = private constant i8* getelementptr ([36 x i8], [36 x i8]* @str8, i32 0, i32 0)
@MSR_IA32_VMX_BASIC = private constant i64 1152
@MSR_IA32_VMX_ENTRY_CTLS = private constant i64 1156
@MSR_IA32_VMX_EXIT_CTLS = private constant i64 1155
@MSR_IA32_VMX_PINBASED_CTLS = private constant i64 1153
@MSR_IA32_VMX_PROCBASED2_CTLS = private constant i64 1169
@MSR_IA32_VMX_PROCBASED_CTLS = private constant i64 1154
@MSR_IA32_VMX_TRUE_ENTRY = private constant i64 1168
@MSR_IA32_VMX_TRUE_EXIT = private constant i64 1167
@MSR_IA32_VMX_TRUE_PINBASED = private constant i64 1165
@MSR_IA32_VMX_TRUE_PROCBASED = private constant i64 1166
@PIN_EXTERNAL_INTERRUPT_EXITING = private constant i64 1
@PIN_NMI_EXITING = private constant i64 8
@PIN_POSTED_INTERRUPT = private constant i64 128
@PIN_PREEMPT_TIMER = private constant i64 64
@PIN_VIRTUAL_NMI = private constant i64 32
@VMCS_ADDR_IO_BITMAP_A = private constant i64 8192
@VMCS_ADDR_IO_BITMAP_B = private constant i64 8194
@VMCS_ADDR_MSR_BITMAP = private constant i64 8196
@VMCS_CR0_GUEST_HOST_MASK = private constant i64 24576
@VMCS_CR0_READ_SHADOW = private constant i64 24580
@VMCS_CR4_GUEST_HOST_MASK = private constant i64 24578
@VMCS_CR4_READ_SHADOW = private constant i64 24582
@VMCS_ENTRY_CTLS = private constant i64 16402
@VMCS_ENTRY_EXCEPTION_ERRORCODE = private constant i64 16408
@VMCS_ENTRY_INSTRUCTION_LENGTH = private constant i64 16410
@VMCS_ENTRY_INTERRUPTION_INFO = private constant i64 16406
@VMCS_ENTRY_MSR_LOAD_COUNT = private constant i64 16404
@VMCS_EXC_BITMAP = private constant i64 16388
@VMCS_EXECUTIVE_VMCS_PTR = private constant i64 8204
@VMCS_EXIT_CTLS = private constant i64 16396
@VMCS_EXIT_MSR_LOAD_COUNT = private constant i64 16400
@VMCS_EXIT_MSR_STORE_COUNT = private constant i64 16398
@VMCS_EXIT_REASON = private constant i64 17410
@VMCS_GUEST_CR0 = private constant i64 26624
@VMCS_GUEST_CR3 = private constant i64 26626
@VMCS_GUEST_CR4 = private constant i64 26628
@VMCS_GUEST_CS_ACCESS_RIGHTS = private constant i64 18454
@VMCS_GUEST_CS_BASE = private constant i64 26632
@VMCS_GUEST_CS_LIMIT = private constant i64 18434
@VMCS_GUEST_CS_SEL = private constant i64 2050
@VMCS_GUEST_DR7 = private constant i64 26650
@VMCS_GUEST_DS_ACCESS_RIGHTS = private constant i64 18458
@VMCS_GUEST_DS_BASE = private constant i64 26636
@VMCS_GUEST_DS_LIMIT = private constant i64 18438
@VMCS_GUEST_DS_SEL = private constant i64 2054
@VMCS_GUEST_ES_ACCESS_RIGHTS = private constant i64 18452
@VMCS_GUEST_ES_BASE = private constant i64 26630
@VMCS_GUEST_ES_LIMIT = private constant i64 18432
@VMCS_GUEST_ES_SEL = private constant i64 2048
@VMCS_GUEST_FS_ACCESS_RIGHTS = private constant i64 18460
@VMCS_GUEST_FS_BASE = private constant i64 26638
@VMCS_GUEST_FS_LIMIT = private constant i64 18440
@VMCS_GUEST_FS_SEL = private constant i64 2056
@VMCS_GUEST_GDTR_BASE = private constant i64 26646
@VMCS_GUEST_GS_ACCESS_RIGHTS = private constant i64 18462
@VMCS_GUEST_GS_BASE = private constant i64 26640
@VMCS_GUEST_GS_LIMIT = private constant i64 18442
@VMCS_GUEST_GS_SEL = private constant i64 2058
@VMCS_GUEST_IA32_EFER = private constant i64 10246
@VMCS_GUEST_IDTR_BASE = private constant i64 26648
@VMCS_GUEST_INTERRUPT_STATUS = private constant i64 18468
@VMCS_GUEST_LDTR_ACCESS_RIGHTS = private constant i64 18464
@VMCS_GUEST_LDTR_BASE = private constant i64 26642
@VMCS_GUEST_LDTR_LIMIT = private constant i64 18444
@VMCS_GUEST_LDTR_SEL = private constant i64 2060
@VMCS_GUEST_PDPTE_BASE0 = private constant i64 10250
@VMCS_GUEST_PDPTE_BASE1 = private constant i64 10252
@VMCS_GUEST_PDPTE_BASE2 = private constant i64 10254
@VMCS_GUEST_PDPTE_BASE3 = private constant i64 10256
@VMCS_GUEST_PHYSICAL_ADDR = private constant i64 9216
@VMCS_GUEST_RFLAGS = private constant i64 26656
@VMCS_GUEST_RIP = private constant i64 26654
@VMCS_GUEST_RSP = private constant i64 26652
@VMCS_GUEST_SS_ACCESS_RIGHTS = private constant i64 18456
@VMCS_GUEST_SS_BASE = private constant i64 26634
@VMCS_GUEST_SS_LIMIT = private constant i64 18436
@VMCS_GUEST_SS_SEL = private constant i64 2052
@VMCS_GUEST_SYSENTER_CS = private constant i64 18474
@VMCS_GUEST_SYSENTER_EIP = private constant i64 26662
@VMCS_GUEST_SYSENTER_ESP = private constant i64 26660
@VMCS_GUEST_TR_ACCESS_RIGHTS = private constant i64 18466
@VMCS_GUEST_TR_BASE = private constant i64 26644
@VMCS_GUEST_TR_LIMIT = private constant i64 18446
@VMCS_GUEST_TR_SEL = private constant i64 2062
@VMCS_GUEST_VMCS_LINK_PTR = private constant i64 10240
@VMCS_HOST_CR0 = private constant i64 27648
@VMCS_HOST_CR3 = private constant i64 27650
@VMCS_HOST_CR4 = private constant i64 27652
@VMCS_HOST_CS_SEL = private constant i64 3074
@VMCS_HOST_DS_SEL = private constant i64 3078
@VMCS_HOST_ES_SEL = private constant i64 3072
@VMCS_HOST_FS_BASE = private constant i64 27654
@VMCS_HOST_FS_SEL = private constant i64 3080
@VMCS_HOST_GDTR_BASE = private constant i64 27660
@VMCS_HOST_GS_BASE = private constant i64 27656
@VMCS_HOST_GS_SEL = private constant i64 3082
@VMCS_HOST_IDTR_BASE = private constant i64 27662
@VMCS_HOST_RIP = private constant i64 27670
@VMCS_HOST_RSP = private constant i64 27668
@VMCS_HOST_SS_SEL = private constant i64 3076
@VMCS_HOST_SYSENTER_CS = private constant i64 19456
@VMCS_HOST_SYSENTER_EIP = private constant i64 27666
@VMCS_HOST_SYSENTER_ESP = private constant i64 27664
@VMCS_HOST_TR_BASE = private constant i64 27658
@VMCS_HOST_TR_SEL = private constant i64 3084
@VMCS_INSTRUCTION_ERROR = private constant i64 17408
@VMCS_PIN_BASED = private constant i64 16384
@VMCS_PROC_BASED = private constant i64 16386
@VMCS_PROC_BASED2 = private constant i64 16414
@VMCS_VMEXIT_INSTRUCTION_LENGTH = private constant i64 17420
@VMCS_VMEXIT_INTERRUPTION_INFO = private constant i64 17412
@VMCS_VPID = private constant i64 0
@VMX_BASIC_DUAL_MONITOR_BIT = private constant i64 562949953421312
@VMX_BASIC_INS_OUTS_BIT = private constant i64 18014398509481984
@VMX_BASIC_MEM_TYPE_WB = private constant i64 6
@VMX_BASIC_TRUE_CTLS_BIT = private constant i64 36028797018963968
@VM_ENTRY_IA32E_MODE = private constant i64 512
@VM_ENTRY_LOAD_DEBUG_CONTROLS = private constant i64 4
@VM_ENTRY_LOAD_IA32_EFER = private constant i64 32768
@VM_EXIT_ACK_INTR_ON_EXIT = private constant i64 32768
@VM_EXIT_HOST_ADDR_SPACE_SIZE = private constant i64 512
@VM_EXIT_LOAD_IA32_EFER = private constant i64 2097152
@VM_EXIT_SAVE_DEBUG_CONTROLS = private constant i64 4
@VM_EXIT_SAVE_IA32_EFER = private constant i64 1048576
@host_stack = global [8192 x i8] zeroinitializer
@vmcs_region = global [4096 x i8] zeroinitializer
@vmxon_region = global [4096 x i8] zeroinitializer

@str0 = private constant [28 x i8] c"[oxide-hv] vmlaunch FAILED\0A\00"
@str1 = private constant [41 x i8] c"\0A[oxide-hv] guest HLT -> clean shutdown\0A\00"
@str2 = private constant [25 x i8] c"[oxide-hv] TRIPLE FAULT\0A\00"
@str3 = private constant [34 x i8] c"[oxide-hv] unhandled exit reason\0A\00"
@str4 = private constant [48 x i8] c"[oxide-hv] host shutting down after guest exit\0A\00"
@str5 = private constant [31 x i8] c"[oxide-hv] VT-x NOT available\0A\00"
@str6 = private constant [29 x i8] c"[oxide-hv] VMX root entered\0A\00"
@str7 = private constant [28 x i8] c"[oxide-hv] vm-entry failed\0A\00"
@str8 = private constant [36 x i8] c"[oxide-hv] long-mode entry reached\0A\00"

define i64 @vmcs_field_width(i64 %arg_enc) {
entry0:
  %var_enc_1 = alloca i64
  store i64 %arg_enc, i64* %var_enc_1
  %t0 = load i64, i64* %var_enc_1
  %t1 = ashr i64 %t0, 10
  %t2 = and i64 %t1, 15
  ret i64 %t2
}

define i1 @vmcs_field_is_wide(i64 %arg_enc) {
entry0:
  %var_enc_1 = alloca i64
  store i64 %arg_enc, i64* %var_enc_1
  %var_w_2 = alloca i64
  %t3 = load i64, i64* %var_enc_1
  %t4 = call i64 @vmcs_field_width(i64 %t3)
  store i64 %t4, i64* %var_w_2
  %t6 = load i64, i64* %var_w_2
  %t7 = icmp eq i64 %t6, 1
  br i1 %t7, label %sc_true5, label %sc_rhs3
sc_rhs3:
  %t8 = load i64, i64* %var_w_2
  %t9 = icmp eq i64 %t8, 3
  br label %sc_done4
sc_true5:
  br label %sc_done4
sc_done4:
  %t5 = phi i1 [ %t9, %sc_rhs3 ], [ true, %sc_true5 ]
  ret i1 %t5
}

define i64 @vmx_adjust_ctl(i64 %arg_msr, i64 %arg_want) {
entry0:
  %var_msr_1 = alloca i64
  store i64 %arg_msr, i64* %var_msr_1
  %var_want_2 = alloca i64
  store i64 %arg_want, i64* %var_want_2
  %var_v_3 = alloca i64
  %t10 = load i64, i64* %var_msr_1
  %t11 = call i64 @rdmsr(i64 %t10)
  store i64 %t11, i64* %var_v_3
  %var_allowed1_4 = alloca i64
  %t12 = load i64, i64* %var_v_3
  %t13 = and i64 %t12, 4294967295
  store i64 %t13, i64* %var_allowed1_4
  %var_allowed0_5 = alloca i64
  %t14 = load i64, i64* %var_v_3
  %t15 = ashr i64 %t14, 32
  %t16 = and i64 %t15, 4294967295
  store i64 %t16, i64* %var_allowed0_5
  %var_required1_6 = alloca i64
  %t17 = load i64, i64* %var_allowed1_4
  %t18 = load i64, i64* %var_allowed0_5
  %t19 = xor i64 %t18, -1
  %t20 = and i64 %t19, 4294967295
  %t21 = and i64 %t17, %t20
  store i64 %t21, i64* %var_required1_6
  %t22 = load i64, i64* %var_want_2
  %t23 = load i64, i64* %var_allowed1_4
  %t24 = and i64 %t22, %t23
  %t25 = load i64, i64* %var_required1_6
  %t26 = or i64 %t24, %t25
  %t27 = and i64 %t26, 4294967295
  ret i64 %t27
}

define i64 @vmx_instruct_error() {
entry0:
  %var_err_1 = alloca i64
  store i64 0, i64* %var_err_1
  %t28 = load i64, i64* %var_err_1
  %asm2 = call i64 asm sideeffect "vmread %rdx, %rax", "={rax},{rdx}"(i64 17408)
  store i64 %asm2, i64* %var_err_1
  %t29 = load i64, i64* %var_err_1
  ret i64 %t29
}

define i64 @vmclear(i64 %arg_region_phys) {
entry0:
  %var_region_phys_1 = alloca i64
  store i64 %arg_region_phys, i64* %var_region_phys_1
  %t30 = load i64, i64* %var_region_phys_1
  call void asm sideeffect "vmclear (%rax)", "{rax}"(i64 %t30)
  ret i64 0
}

define i64 @vmptrld(i64 %arg_region_phys) {
entry0:
  %var_region_phys_1 = alloca i64
  store i64 %arg_region_phys, i64* %var_region_phys_1
  %t31 = load i64, i64* %var_region_phys_1
  call void asm sideeffect "vmptrld (%rax)", "{rax}"(i64 %t31)
  ret i64 0
}

define void @vmwrite(i64 %arg_field, i64 %arg_value) {
entry0:
  %var_field_1 = alloca i64
  store i64 %arg_field, i64* %var_field_1
  %var_value_2 = alloca i64
  store i64 %arg_value, i64* %var_value_2
  %t32 = load i64, i64* %var_field_1
  %t33 = load i64, i64* %var_value_2
  call void asm sideeffect "vmwrite %rax, %rdx", "{rax},{rdx}"(i64 %t32, i64 %t33)
  ret void
}

define void @vmwrite32(i64 %arg_field, i64 %arg_value) {
entry0:
  %var_field_1 = alloca i64
  store i64 %arg_field, i64* %var_field_1
  %var_value_2 = alloca i64
  store i64 %arg_value, i64* %var_value_2
  %t34 = load i64, i64* %var_field_1
  %t35 = load i64, i64* %var_value_2
  %t36 = and i64 %t35, 4294967295
  call void @vmwrite(i64 %t34, i64 %t36)
  ret void
}

define i64 @vmread(i64 %arg_field) {
entry0:
  %var_field_1 = alloca i64
  store i64 %arg_field, i64* %var_field_1
  %var_v_2 = alloca i64
  store i64 0, i64* %var_v_2
  %t37 = load i64, i64* %var_v_2
  %t38 = load i64, i64* %var_field_1
  %asm3 = call i64 asm sideeffect "vmread %rdx, %rax", "={rax},{rdx}"(i64 %t38)
  store i64 %asm3, i64* %var_v_2
  %t39 = load i64, i64* %var_v_2
  ret i64 %t39
}

define i64 @vmlaunch() {
entry0:
  %var_rc_1 = alloca i64
  store i64 0, i64* %var_rc_1
  %t40 = load i64, i64* %var_rc_1
  %asm2 = call i64 asm sideeffect "vmlaunch", "={rax}"()
  store i64 %asm2, i64* %var_rc_1
  ret i64 1
}

define i64 @vmresume() {
entry0:
  %var_rc_1 = alloca i64
  store i64 0, i64* %var_rc_1
  %t41 = load i64, i64* %var_rc_1
  %asm2 = call i64 asm sideeffect "vmresume", "={rax}"()
  store i64 %asm2, i64* %var_rc_1
  ret i64 1
}

define i64 @vmxoff() {
entry0:
  call void asm sideeffect "vmxoff", ""()
  ret i64 0
}

define void @vmcall() {
entry0:
  call void asm sideeffect "vmcall", ""()
  ret void
}

define void @vmwrite16(i64 %arg_field, i64 %arg_value) {
entry0:
  %var_field_1 = alloca i64
  store i64 %arg_field, i64* %var_field_1
  %var_value_2 = alloca i64
  store i64 %arg_value, i64* %var_value_2
  %t42 = load i64, i64* %var_field_1
  %t43 = load i64, i64* %var_value_2
  %t44 = and i64 %t43, 65535
  call void @vmwrite(i64 %t42, i64 %t44)
  ret void
}

define i64 @read_cr3_phys() {
entry0:
  %var_v_1 = alloca i64
  store i64 0, i64* %var_v_1
  %t45 = load i64, i64* %var_v_1
  %asm2 = call i64 asm sideeffect "mov %cr3, %rax", "={rax}"()
  store i64 %asm2, i64* %var_v_1
  %t46 = load i64, i64* %var_v_1
  %t47 = and i64 %t46, 18446744073709547520
  ret i64 %t47
}

define i64 @launch_guest() {
entry0:
  %var_basic_1 = alloca i64
  %t48 = load i64, i64* @MSR_IA32_VMX_BASIC
  %t49 = call i64 @rdmsr(i64 %t48)
  store i64 %t49, i64* %var_basic_1
  %var_rev_2 = alloca i32
  %t50 = load i64, i64* %var_basic_1
  %t51 = and i64 %t50, 2147483647
  %cast3 = trunc i64 %t51 to i32
  store i32 %cast3, i32* %var_rev_2
  %var_vmcs_ptr_4 = alloca i8*
  %t52 = load [4096 x i8], [4096 x i8]* @vmcs_region
  %ic5 = icmp slt i64 0, 0
  %ic6 = icmp sge i64 0, 4096
  %bad7 = or i1 %ic5, %ic6
  br i1 %bad7, label %idx_fail9, label %idx_ok8
idx_fail9:
  call void @ox_bounds_fail(i64 0, i64 4096)
  unreachable
idx_ok8:
  %ep10 = getelementptr inbounds [4096 x i8], [4096 x i8]* @vmcs_region, i64 0, i64 0
  %idx11 = load i8, i8* %ep10
  %ic12 = icmp slt i64 0, 0
  %ic13 = icmp sge i64 0, 4096
  %bad14 = or i1 %ic12, %ic13
  br i1 %bad14, label %idx_fail16, label %idx_ok15
idx_fail16:
  call void @ox_bounds_fail(i64 0, i64 4096)
  unreachable
idx_ok15:
  %ep17 = getelementptr inbounds [4096 x i8], [4096 x i8]* @vmcs_region, i64 0, i64 0
  store i8* %ep17, i8** %var_vmcs_ptr_4
  %var_rev_ptr_18 = alloca i32*
  %t53 = load i8*, i8** %var_vmcs_ptr_4
  %cast19 = bitcast i8* %t53 to i32*
  store i32* %cast19, i32** %var_rev_ptr_18
  %t54 = load i32*, i32** %var_rev_ptr_18
  %t55 = load i32, i32* %var_rev_2
  store volatile i32 %t55, i32* %t54
  %var_abort_ptr_20 = alloca i32*
  %t56 = load i8*, i8** %var_vmcs_ptr_4
  %gep21 = getelementptr inbounds i8, i8* %t56, i64 4
  %cast22 = bitcast i8* %gep21 to i32*
  store i32* %cast22, i32** %var_abort_ptr_20
  %t57 = load i32*, i32** %var_abort_ptr_20
  %cast23 = trunc i64 0 to i32
  store volatile i32 %cast23, i32* %t57
  %var_vmcs_phys_24 = alloca i64
  %t58 = load i8*, i8** %var_vmcs_ptr_4
  %cast25 = ptrtoint i8* %t58 to i64
  store i64 %cast25, i64* %var_vmcs_phys_24
  %t59 = load i64, i64* %var_vmcs_phys_24
  %t60 = call i64 @vmclear(i64 %t59)
  %t61 = load i64, i64* %var_vmcs_phys_24
  %t62 = call i64 @vmptrld(i64 %t61)
  %t63 = load i64, i64* @VMCS_HOST_RIP
  %t64 = call i64 @ox_host_rip()
  call void @vmwrite(i64 %t63, i64 %t64)
  %t65 = load i64, i64* @VMCS_HOST_RSP
  %t66 = call i64 @ox_host_stack_top_addr()
  call void @vmwrite(i64 %t65, i64 %t66)
  %t67 = load i64, i64* @VMCS_HOST_CS_SEL
  call void @vmwrite16(i64 %t67, i64 8)
  %t68 = load i64, i64* @VMCS_HOST_SS_SEL
  call void @vmwrite16(i64 %t68, i64 16)
  %t69 = load i64, i64* @VMCS_HOST_DS_SEL
  call void @vmwrite16(i64 %t69, i64 16)
  %t70 = load i64, i64* @VMCS_HOST_ES_SEL
  call void @vmwrite16(i64 %t70, i64 16)
  %t71 = load i64, i64* @VMCS_HOST_FS_SEL
  call void @vmwrite16(i64 %t71, i64 0)
  %t72 = load i64, i64* @VMCS_HOST_GS_SEL
  call void @vmwrite16(i64 %t72, i64 0)
  %t73 = load i64, i64* @VMCS_HOST_TR_SEL
  call void @vmwrite16(i64 %t73, i64 24)
  %t74 = load i64, i64* @VMCS_HOST_CR0
  %t75 = call i64 @read_cr0()
  call void @vmwrite(i64 %t74, i64 %t75)
  %t76 = load i64, i64* @VMCS_HOST_CR3
  %t77 = call i64 @read_cr3_phys()
  call void @vmwrite(i64 %t76, i64 %t77)
  %t78 = load i64, i64* @VMCS_HOST_CR4
  %t79 = call i64 @read_cr4()
  call void @vmwrite(i64 %t78, i64 %t79)
  %t80 = load i64, i64* @VMCS_HOST_FS_BASE
  call void @vmwrite(i64 %t80, i64 0)
  %t81 = load i64, i64* @VMCS_HOST_GS_BASE
  %t82 = call i64 @rdmsr(i64 3221225729)
  call void @vmwrite(i64 %t81, i64 %t82)
  %t83 = load i64, i64* @VMCS_HOST_GDTR_BASE
  call void @vmwrite(i64 %t83, i64 0)
  %t84 = load i64, i64* @VMCS_HOST_IDTR_BASE
  call void @vmwrite(i64 %t84, i64 0)
  %t85 = load i64, i64* @VMCS_HOST_TR_BASE
  call void @vmwrite(i64 %t85, i64 0)
  %t86 = load i64, i64* @VMCS_HOST_SYSENTER_CS
  call void @vmwrite(i64 %t86, i64 0)
  %t87 = load i64, i64* @VMCS_HOST_SYSENTER_ESP
  call void @vmwrite(i64 %t87, i64 0)
  %t88 = load i64, i64* @VMCS_HOST_SYSENTER_EIP
  call void @vmwrite(i64 %t88, i64 0)
  %var_exit_ctl_26 = alloca i64
  %t89 = load i64, i64* @MSR_IA32_VMX_EXIT_CTLS
  %t90 = load i64, i64* @VM_EXIT_HOST_ADDR_SPACE_SIZE
  %t91 = load i64, i64* @VM_EXIT_SAVE_IA32_EFER
  %t92 = or i64 %t90, %t91
  %t93 = load i64, i64* @VM_EXIT_LOAD_IA32_EFER
  %t94 = or i64 %t92, %t93
  %t95 = call i64 @vmx_adjust_ctl(i64 %t89, i64 %t94)
  store i64 %t95, i64* %var_exit_ctl_26
  %t96 = load i64, i64* @VMCS_EXIT_CTLS
  %t97 = load i64, i64* %var_exit_ctl_26
  call void @vmwrite32(i64 %t96, i64 %t97)
  %var_entry_ctl_27 = alloca i64
  %t98 = load i64, i64* @MSR_IA32_VMX_ENTRY_CTLS
  %t99 = load i64, i64* @VM_ENTRY_IA32E_MODE
  %t100 = load i64, i64* @VM_ENTRY_LOAD_IA32_EFER
  %t101 = or i64 %t99, %t100
  %t102 = call i64 @vmx_adjust_ctl(i64 %t98, i64 %t101)
  store i64 %t102, i64* %var_entry_ctl_27
  %t103 = load i64, i64* @VMCS_ENTRY_CTLS
  %t104 = load i64, i64* %var_entry_ctl_27
  call void @vmwrite32(i64 %t103, i64 %t104)
  %var_pin_28 = alloca i64
  %t105 = load i64, i64* @MSR_IA32_VMX_PINBASED_CTLS
  %t106 = call i64 @vmx_adjust_ctl(i64 %t105, i64 0)
  store i64 %t106, i64* %var_pin_28
  %t107 = load i64, i64* @VMCS_PIN_BASED
  %t108 = load i64, i64* %var_pin_28
  call void @vmwrite32(i64 %t107, i64 %t108)
  %var_proc_29 = alloca i64
  %t109 = load i64, i64* @MSR_IA32_VMX_PROCBASED_CTLS
  %t110 = load i64, i64* @CPU_BASED_HLT_EXITING
  %t111 = load i64, i64* @CPU_BASED_ACTIVATE_SECONDARY
  %t112 = or i64 %t110, %t111
  %t113 = call i64 @vmx_adjust_ctl(i64 %t109, i64 %t112)
  store i64 %t113, i64* %var_proc_29
  %t114 = load i64, i64* @VMCS_PROC_BASED
  %t115 = load i64, i64* %var_proc_29
  call void @vmwrite32(i64 %t114, i64 %t115)
  %var_proc2_30 = alloca i64
  %t116 = load i64, i64* @MSR_IA32_VMX_PROCBASED2_CTLS
  %t117 = call i64 @vmx_adjust_ctl(i64 %t116, i64 0)
  store i64 %t117, i64* %var_proc2_30
  %t118 = load i64, i64* @VMCS_PROC_BASED2
  %t119 = load i64, i64* %var_proc2_30
  call void @vmwrite32(i64 %t118, i64 %t119)
  %t120 = load i64, i64* @VMCS_EXC_BITMAP
  call void @vmwrite32(i64 %t120, i64 0)
  %var_gcr0_31 = alloca i64
  store i64 2147483699, i64* %var_gcr0_31
  %t121 = load i64, i64* @VMCS_GUEST_CR0
  %t122 = load i64, i64* %var_gcr0_31
  call void @vmwrite(i64 %t121, i64 %t122)
  %t123 = load i64, i64* @VMCS_CR0_GUEST_HOST_MASK
  call void @vmwrite(i64 %t123, i64 0)
  %t124 = load i64, i64* @VMCS_CR0_READ_SHADOW
  %t125 = load i64, i64* %var_gcr0_31
  call void @vmwrite(i64 %t124, i64 %t125)
  %t126 = load i64, i64* @VMCS_GUEST_CR3
  %t127 = call i64 @read_cr3_phys()
  call void @vmwrite(i64 %t126, i64 %t127)
  %var_gcr4_32 = alloca i64
  %t128 = shl i64 1, 5
  store i64 %t128, i64* %var_gcr4_32
  %t129 = load i64, i64* @VMCS_GUEST_CR4
  %t130 = load i64, i64* %var_gcr4_32
  call void @vmwrite(i64 %t129, i64 %t130)
  %t131 = load i64, i64* @VMCS_CR4_GUEST_HOST_MASK
  call void @vmwrite(i64 %t131, i64 0)
  %t132 = load i64, i64* @VMCS_CR4_READ_SHADOW
  %t133 = load i64, i64* %var_gcr4_32
  call void @vmwrite(i64 %t132, i64 %t133)
  %t134 = load i64, i64* @VMCS_GUEST_IA32_EFER
  call void @vmwrite(i64 %t134, i64 768)
  %t135 = load i64, i64* @VMCS_GUEST_CS_SEL
  call void @vmwrite16(i64 %t135, i64 8)
  %t136 = load i64, i64* @VMCS_GUEST_SS_SEL
  call void @vmwrite16(i64 %t136, i64 16)
  %t137 = load i64, i64* @VMCS_GUEST_DS_SEL
  call void @vmwrite16(i64 %t137, i64 16)
  %t138 = load i64, i64* @VMCS_GUEST_ES_SEL
  call void @vmwrite16(i64 %t138, i64 16)
  %t139 = load i64, i64* @VMCS_GUEST_FS_SEL
  call void @vmwrite16(i64 %t139, i64 0)
  %t140 = load i64, i64* @VMCS_GUEST_GS_SEL
  call void @vmwrite16(i64 %t140, i64 0)
  %t141 = load i64, i64* @VMCS_GUEST_LDTR_SEL
  call void @vmwrite16(i64 %t141, i64 0)
  %t142 = load i64, i64* @VMCS_GUEST_TR_SEL
  call void @vmwrite16(i64 %t142, i64 24)
  %t143 = load i64, i64* @VMCS_GUEST_CS_ACCESS_RIGHTS
  %t144 = load i64, i64* @AR_CODE64_EXEC_READ_DPL0
  call void @vmwrite32(i64 %t143, i64 %t144)
  %t145 = load i64, i64* @VMCS_GUEST_SS_ACCESS_RIGHTS
  %t146 = load i64, i64* @AR_DATA_RW_DPL0
  call void @vmwrite32(i64 %t145, i64 %t146)
  %t147 = load i64, i64* @VMCS_GUEST_DS_ACCESS_RIGHTS
  %t148 = load i64, i64* @AR_DATA_RW_DPL0
  call void @vmwrite32(i64 %t147, i64 %t148)
  %t149 = load i64, i64* @VMCS_GUEST_ES_ACCESS_RIGHTS
  %t150 = load i64, i64* @AR_DATA_RW_DPL0
  call void @vmwrite32(i64 %t149, i64 %t150)
  %t151 = load i64, i64* @VMCS_GUEST_FS_ACCESS_RIGHTS
  %t152 = load i64, i64* @AR_UNUSABLE
  call void @vmwrite32(i64 %t151, i64 %t152)
  %t153 = load i64, i64* @VMCS_GUEST_GS_ACCESS_RIGHTS
  %t154 = load i64, i64* @AR_UNUSABLE
  call void @vmwrite32(i64 %t153, i64 %t154)
  %t155 = load i64, i64* @VMCS_GUEST_LDTR_ACCESS_RIGHTS
  %t156 = load i64, i64* @AR_UNUSABLE
  call void @vmwrite32(i64 %t155, i64 %t156)
  %t157 = load i64, i64* @VMCS_GUEST_TR_ACCESS_RIGHTS
  %t158 = load i64, i64* @AR_TR_BUSY_64
  call void @vmwrite32(i64 %t157, i64 %t158)
  %t159 = load i64, i64* @VMCS_GUEST_CS_LIMIT
  call void @vmwrite32(i64 %t159, i64 1048575)
  %t160 = load i64, i64* @VMCS_GUEST_SS_LIMIT
  call void @vmwrite32(i64 %t160, i64 1048575)
  %t161 = load i64, i64* @VMCS_GUEST_DS_LIMIT
  call void @vmwrite32(i64 %t161, i64 1048575)
  %t162 = load i64, i64* @VMCS_GUEST_ES_LIMIT
  call void @vmwrite32(i64 %t162, i64 1048575)
  %t163 = load i64, i64* @VMCS_GUEST_FS_LIMIT
  call void @vmwrite32(i64 %t163, i64 1048575)
  %t164 = load i64, i64* @VMCS_GUEST_GS_LIMIT
  call void @vmwrite32(i64 %t164, i64 1048575)
  %t165 = load i64, i64* @VMCS_GUEST_LDTR_LIMIT
  call void @vmwrite32(i64 %t165, i64 1048575)
  %t166 = load i64, i64* @VMCS_GUEST_TR_LIMIT
  call void @vmwrite32(i64 %t166, i64 111)
  %t167 = load i64, i64* @VMCS_GUEST_CS_BASE
  call void @vmwrite(i64 %t167, i64 0)
  %t168 = load i64, i64* @VMCS_GUEST_SS_BASE
  call void @vmwrite(i64 %t168, i64 0)
  %t169 = load i64, i64* @VMCS_GUEST_DS_BASE
  call void @vmwrite(i64 %t169, i64 0)
  %t170 = load i64, i64* @VMCS_GUEST_ES_BASE
  call void @vmwrite(i64 %t170, i64 0)
  %t171 = load i64, i64* @VMCS_GUEST_FS_BASE
  call void @vmwrite(i64 %t171, i64 0)
  %t172 = load i64, i64* @VMCS_GUEST_GS_BASE
  call void @vmwrite(i64 %t172, i64 0)
  %t173 = load i64, i64* @VMCS_GUEST_LDTR_BASE
  call void @vmwrite(i64 %t173, i64 0)
  %t174 = load i64, i64* @VMCS_GUEST_TR_BASE
  call void @vmwrite(i64 %t174, i64 0)
  %t175 = load i64, i64* @VMCS_GUEST_GDTR_BASE
  call void @vmwrite(i64 %t175, i64 0)
  %t176 = load i64, i64* @VMCS_GUEST_IDTR_BASE
  call void @vmwrite(i64 %t176, i64 0)
  %t177 = load i64, i64* @VMCS_GUEST_RIP
  %t178 = call i64 @ox_guest_rip()
  call void @vmwrite(i64 %t177, i64 %t178)
  %t179 = load i64, i64* @VMCS_GUEST_RSP
  %t180 = call i64 @ox_host_stack_top_addr()
  call void @vmwrite(i64 %t179, i64 %t180)
  %t181 = load i64, i64* @VMCS_GUEST_RFLAGS
  call void @vmwrite(i64 %t181, i64 2)
  %t182 = load i64, i64* @VMCS_GUEST_DR7
  call void @vmwrite(i64 %t182, i64 0)
  %t183 = load i64, i64* @VMCS_GUEST_SYSENTER_CS
  call void @vmwrite(i64 %t183, i64 0)
  %t184 = load i64, i64* @VMCS_GUEST_SYSENTER_ESP
  call void @vmwrite(i64 %t184, i64 0)
  %t185 = load i64, i64* @VMCS_GUEST_SYSENTER_EIP
  call void @vmwrite(i64 %t185, i64 0)
  %t186 = load i64, i64* @VMCS_GUEST_VMCS_LINK_PTR
  call void @vmwrite(i64 %t186, i64 18446744073709551615)
  %var_launched_33 = alloca i64
  %t187 = call i64 @vmlaunch()
  store i64 %t187, i64* %var_launched_33
  %t188 = load i64, i64* %var_launched_33
  %t189 = icmp ne i64 %t188, 0
  br i1 %t189, label %then34, label %else35
then34:
  %var_err_37 = alloca i64
  %t190 = call i64 @vmx_instruct_error()
  store i64 %t190, i64* %var_err_37
  %t191 = getelementptr inbounds [28 x i8], [28 x i8]* @str0, i64 0, i64 0
  call void @serial_puts(i8* %t191)
  %t192 = load i64, i64* %var_err_37
  %t193 = and i64 %t192, 255
  %t194 = or i64 %t193, 256
  ret i64 %t194
else35:
  br label %merge36
merge36:
  ret i64 0
}

define void @guest_payload() {
entry0:
  call void asm sideeffect "mov $$0x41, %rax\0Avmcall\0Amov $$0x42, %rax\0Avmcall\0Amov $$0x43, %rax\0Avmcall\0Ahlt", ""()
  ret void
}

define void @advance_guest_rip() {
entry0:
  %var_len_1 = alloca i64
  %t195 = load i64, i64* @VMCS_VMEXIT_INSTRUCTION_LENGTH
  %t196 = call i64 @vmread(i64 %t195)
  store i64 %t196, i64* %var_len_1
  %var_rip_2 = alloca i64
  %t197 = load i64, i64* @VMCS_GUEST_RIP
  %t198 = call i64 @vmread(i64 %t197)
  store i64 %t198, i64* %var_rip_2
  %t199 = load i64, i64* @VMCS_GUEST_RIP
  %t200 = load i64, i64* %var_rip_2
  %t201 = load i64, i64* %var_len_1
  %t202 = add i64 %t200, %t201
  call void @vmwrite(i64 %t199, i64 %t202)
  ret void
}

define void @service_vmcall(i64* %arg_g) {
entry0:
  %var_g_1 = alloca i64*
  store i64* %arg_g, i64** %var_g_1
  %var_arg_2 = alloca i64*
  %t203 = load i64*, i64** %var_g_1
  %gep3 = getelementptr inbounds i64, i64* %t203, i64 0
  store i64* %gep3, i64** %var_arg_2
  %var_val_4 = alloca i64
  %t204 = load i64*, i64** %var_arg_2
  %mmio5 = load volatile i64, i64* %t204
  store i64 %mmio5, i64* %var_val_4
  %t205 = load i64, i64* %var_val_4
  %t206 = icmp ne i64 %t205, 0
  br i1 %t206, label %then6, label %else7
then6:
  %t207 = load i64, i64* %var_val_4
  %t208 = and i64 %t207, 255
  %cast9 = trunc i64 %t208 to i8
  call void @serial_putc(i8 %cast9)
  br label %merge8
else7:
  br label %merge8
merge8:
  call void @advance_guest_rip()
  ret void
}

define i64 @vmexit_c_handler(i64* %arg_g) {
entry0:
  %var_g_1 = alloca i64*
  store i64* %arg_g, i64** %var_g_1
  %var_reason_2 = alloca i64
  %t209 = load i64, i64* @VMCS_EXIT_REASON
  %t210 = call i64 @vmread(i64 %t209)
  %t211 = and i64 %t210, 65535
  store i64 %t211, i64* %var_reason_2
  %t212 = load i64, i64* %var_reason_2
  %t213 = load i64, i64* @EXIT_REASON_VMCALL
  %t214 = icmp eq i64 %t212, %t213
  br i1 %t214, label %then3, label %else4
then3:
  %t215 = load i64*, i64** %var_g_1
  call void @service_vmcall(i64* %t215)
  ret i64 0
else4:
  %t216 = load i64, i64* %var_reason_2
  %t217 = load i64, i64* @EXIT_REASON_HLT
  %t218 = icmp eq i64 %t216, %t217
  br i1 %t218, label %then6, label %else7
then6:
  %t219 = getelementptr inbounds [41 x i8], [41 x i8]* @str1, i64 0, i64 0
  call void @serial_puts(i8* %t219)
  ret i64 1
else7:
  %t220 = load i64, i64* %var_reason_2
  %t221 = load i64, i64* @EXIT_REASON_TRIPLE_FAULT
  %t222 = icmp eq i64 %t220, %t221
  br i1 %t222, label %then9, label %else10
then9:
  %t223 = getelementptr inbounds [25 x i8], [25 x i8]* @str2, i64 0, i64 0
  call void @serial_puts(i8* %t223)
  ret i64 1
else10:
  %t224 = getelementptr inbounds [34 x i8], [34 x i8]* @str3, i64 0, i64 0
  call void @serial_puts(i8* %t224)
  ret i64 1
merge11:
  br label %merge8
merge8:
  br label %merge5
merge5:
  ret i64 1
}

define void @vmexit_done() {
entry0:
  %t225 = getelementptr inbounds [48 x i8], [48 x i8]* @str4, i64 0, i64 0
  call void @serial_puts(i8* %t225)
  ret void
}

define void @ox_com1_panic_put(i64 %arg_c) {
entry0:
  %var_c_1 = alloca i64
  store i64 %arg_c, i64* %var_c_1
  %t226 = load i64, i64* %var_c_1
  call void asm sideeffect "outb %al, %dx", "{dx},{al}"(i64 1016, i64 %t226)
  ret void
}

define void @ox_bounds_fail(i64 %arg_idx, i64 %arg_bound) {
entry0:
  %var_idx_1 = alloca i64
  store i64 %arg_idx, i64* %var_idx_1
  %var_bound_2 = alloca i64
  store i64 %arg_bound, i64* %var_bound_2
  call void @ox_com1_panic_put(i64 75)
  call void @ox_com1_panic_put(i64 112)
  call void @ox_com1_panic_put(i64 76)
  call void @ox_com1_panic_put(i64 46)
  call void @ox_com1_panic_put(i64 46)
  %t227 = load i64, i64* %var_idx_1
  %t228 = load i64, i64* %var_bound_2
  %t229 = xor i64 %t227, %t228
  %t230 = and i64 %t229, 255
  call void @ox_com1_panic_put(i64 %t230)
  %var_i_3 = alloca i64
  store i64 0, i64* %var_i_3
  br label %while_cond4
while_cond4:
  %t231 = load i64, i64* %var_i_3
  %t232 = icmp slt i64 %t231, 1000
  br i1 %t232, label %while_body5, label %while_end6
while_body5:
  call void asm sideeffect "cli", ""()
  call void asm sideeffect "hlt", ""()
  %t233 = load i64, i64* %var_i_3
  %t234 = add i64 %t233, 1
  store i64 %t234, i64* %var_i_3
  br label %while_cond4
while_end6:
  br label %while_cond7
while_cond7:
  br i1 1, label %while_body8, label %while_end9
while_body8:
  call void asm sideeffect "hlt", ""()
  br label %while_cond7
while_end9:
  ret void
}

define i64 @rdmsr(i64 %arg_msr) {
entry0:
  %var_msr_1 = alloca i64
  store i64 %arg_msr, i64* %var_msr_1
  %var_lo_2 = alloca i64
  store i64 0, i64* %var_lo_2
  %var_hi_3 = alloca i64
  store i64 0, i64* %var_hi_3
  %var_ecx_4 = alloca i64
  %t235 = load i64, i64* %var_msr_1
  store i64 %t235, i64* %var_ecx_4
  %t236 = load i64, i64* %var_lo_2
  %t237 = load i64, i64* %var_hi_3
  %t238 = load i64, i64* %var_ecx_4
  %asm5 = call {i64, i64} asm sideeffect "rdmsr", "={eax},={edx},{ecx}"(i64 %t238)
  %asmout6 = extractvalue {i64, i64} %asm5, 0
  store i64 %asmout6, i64* %var_lo_2
  %asmout7 = extractvalue {i64, i64} %asm5, 1
  store i64 %asmout7, i64* %var_hi_3
  %t239 = load i64, i64* %var_hi_3
  %t240 = shl i64 %t239, 32
  %t241 = load i64, i64* %var_lo_2
  %t242 = or i64 %t240, %t241
  ret i64 %t242
}

define void @wrmsr(i64 %arg_msr, i64 %arg_val) {
entry0:
  %var_msr_1 = alloca i64
  store i64 %arg_msr, i64* %var_msr_1
  %var_val_2 = alloca i64
  store i64 %arg_val, i64* %var_val_2
  %var_ecx_3 = alloca i64
  %t243 = load i64, i64* %var_msr_1
  store i64 %t243, i64* %var_ecx_3
  %var_lo_4 = alloca i64
  %t244 = load i64, i64* %var_val_2
  %t245 = and i64 %t244, 4294967295
  store i64 %t245, i64* %var_lo_4
  %var_hi_5 = alloca i64
  %t246 = load i64, i64* %var_val_2
  %t247 = ashr i64 %t246, 32
  %t248 = and i64 %t247, 4294967295
  store i64 %t248, i64* %var_hi_5
  %t249 = load i64, i64* %var_ecx_3
  %t250 = load i64, i64* %var_lo_4
  %t251 = load i64, i64* %var_hi_5
  call void asm sideeffect "wrmsr", "{ecx},{eax},{edx}"(i64 %t249, i64 %t250, i64 %t251)
  ret void
}

define i64 @read_cr0() {
entry0:
  %var_v_1 = alloca i64
  store i64 0, i64* %var_v_1
  %t252 = load i64, i64* %var_v_1
  %asm2 = call i64 asm sideeffect "mov %cr0, %rax", "={rax}"()
  store i64 %asm2, i64* %var_v_1
  %t253 = load i64, i64* %var_v_1
  ret i64 %t253
}

define void @write_cr0(i64 %arg_v) {
entry0:
  %var_v_1 = alloca i64
  store i64 %arg_v, i64* %var_v_1
  %t254 = load i64, i64* %var_v_1
  call void asm sideeffect "mov %rax, %cr0", "{rax}"(i64 %t254)
  ret void
}

define i64 @read_cr4() {
entry0:
  %var_v_1 = alloca i64
  store i64 0, i64* %var_v_1
  %t255 = load i64, i64* %var_v_1
  %asm2 = call i64 asm sideeffect "mov %cr4, %rax", "={rax}"()
  store i64 %asm2, i64* %var_v_1
  %t256 = load i64, i64* %var_v_1
  ret i64 %t256
}

define void @write_cr4(i64 %arg_v) {
entry0:
  %var_v_1 = alloca i64
  store i64 %arg_v, i64* %var_v_1
  %t257 = load i64, i64* %var_v_1
  call void asm sideeffect "mov %rax, %cr4", "{rax}"(i64 %t257)
  ret void
}

define void @write_cr3(i64 %arg_v) {
entry0:
  %var_v_1 = alloca i64
  store i64 %arg_v, i64* %var_v_1
  %t258 = load i64, i64* %var_v_1
  call void asm sideeffect "mov %rax, %cr3", "{rax}"(i64 %t258)
  ret void
}

define void @outb(i64 %arg_port, i64 %arg_val) {
entry0:
  %var_port_1 = alloca i64
  store i64 %arg_port, i64* %var_port_1
  %var_val_2 = alloca i64
  store i64 %arg_val, i64* %var_val_2
  %var_dx_3 = alloca i16
  %t259 = load i64, i64* %var_port_1
  %cast4 = trunc i64 %t259 to i16
  store i16 %cast4, i16* %var_dx_3
  %var_al_5 = alloca i8
  %t260 = load i64, i64* %var_val_2
  %cast6 = trunc i64 %t260 to i8
  store i8 %cast6, i8* %var_al_5
  %t261 = load i16, i16* %var_dx_3
  %t262 = load i8, i8* %var_al_5
  call void asm sideeffect "outb %al, %dx", "{dx},{al}"(i16 %t261, i8 %t262)
  ret void
}

define i64 @inb(i64 %arg_port) {
entry0:
  %var_port_1 = alloca i64
  store i64 %arg_port, i64* %var_port_1
  %var_dx_2 = alloca i16
  %t263 = load i64, i64* %var_port_1
  %cast3 = trunc i64 %t263 to i16
  store i16 %cast3, i16* %var_dx_2
  %var_al_4 = alloca i8
  %cast5 = trunc i64 0 to i8
  store i8 %cast5, i8* %var_al_4
  %t264 = load i8, i8* %var_al_4
  %t265 = load i16, i16* %var_dx_2
  %asm6 = call i8 asm sideeffect "inb %dx, %al", "={al},{dx}"(i16 %t265)
  store i8 %asm6, i8* %var_al_4
  %t266 = load i8, i8* %var_al_4
  %cast7 = sext i8 %t266 to i64
  ret i64 %cast7
}

define void @halt_forever() {
entry0:
  call void asm sideeffect "cli", ""()
  call void asm sideeffect "hlt", ""()
  call void @halt_forever()
  ret void
}

define void @serial_init() {
entry0:
  %t267 = load i64, i64* @COM1
  %t268 = add i64 %t267, 1
  call void @outb(i64 %t268, i64 0)
  %t269 = load i64, i64* @COM1
  %t270 = add i64 %t269, 3
  call void @outb(i64 %t270, i64 128)
  %t271 = load i64, i64* @COM1
  %t272 = add i64 %t271, 0
  call void @outb(i64 %t272, i64 3)
  %t273 = load i64, i64* @COM1
  %t274 = add i64 %t273, 1
  call void @outb(i64 %t274, i64 0)
  %t275 = load i64, i64* @COM1
  %t276 = add i64 %t275, 3
  call void @outb(i64 %t276, i64 3)
  %t277 = load i64, i64* @COM1
  %t278 = add i64 %t277, 2
  call void @outb(i64 %t278, i64 199)
  %t279 = load i64, i64* @COM1
  %t280 = add i64 %t279, 4
  call void @outb(i64 %t280, i64 11)
  ret void
}

define i1 @serial_can_send() {
entry0:
  %t281 = load i64, i64* @COM1
  %t282 = add i64 %t281, 5
  %t283 = call i64 @inb(i64 %t282)
  %t284 = and i64 %t283, 32
  %t285 = icmp ne i64 %t284, 0
  ret i1 %t285
}

define void @serial_putc(i8 %arg_c) {
entry0:
  %var_c_1 = alloca i8
  store i8 %arg_c, i8* %var_c_1
  %var_guard_2 = alloca i64
  store i64 0, i64* %var_guard_2
  br label %while_cond3
while_cond3:
  %t286 = call i1 @serial_can_send()
  %t287 = xor i1 %t286, true
  br i1 %t287, label %while_body4, label %while_end5
while_body4:
  %t288 = load i64, i64* %var_guard_2
  %t289 = add i64 %t288, 1
  store i64 %t289, i64* %var_guard_2
  %t290 = load i64, i64* %var_guard_2
  %t291 = icmp sgt i64 %t290, 100000
  br i1 %t291, label %then6, label %else7
then6:
  ret void
else7:
  br label %merge8
merge8:
  br label %while_cond3
while_end5:
  %t292 = load i64, i64* @COM1
  %t293 = load i8, i8* %var_c_1
  %cast9 = zext i8 %t293 to i64
  call void @outb(i64 %t292, i64 %cast9)
  ret void
}

define void @serial_puts(i8* %arg_s) {
entry0:
  %var_s_1 = alloca i8*
  store i8* %arg_s, i8** %var_s_1
  %var_p_2 = alloca i8*
  %t294 = load i8*, i8** %var_s_1
  store i8* %t294, i8** %var_p_2
  br label %while_cond3
while_cond3:
  br i1 1, label %while_body4, label %while_end5
while_body4:
  %var_c_6 = alloca i8
  %t295 = load i8*, i8** %var_p_2
  %mmio7 = load volatile i8, i8* %t295
  store i8 %mmio7, i8* %var_c_6
  %t296 = load i8, i8* %var_c_6
  %cast8 = trunc i64 0 to i8
  %t297 = icmp eq i8 %t296, %cast8
  br i1 %t297, label %then9, label %else10
then9:
  ret void
else10:
  br label %merge11
merge11:
  %t298 = load i8, i8* %var_c_6
  call void @serial_putc(i8 %t298)
  %t299 = load i8*, i8** %var_p_2
  %gep12 = getelementptr inbounds i8, i8* %t299, i64 1
  store i8* %gep12, i8** %var_p_2
  br label %while_cond3
while_end5:
  ret void
}

define i1 @has_vmx() {
entry0:
  %var_eax_1 = alloca i64
  store i64 1, i64* %var_eax_1
  %var_ecx_2 = alloca i64
  store i64 0, i64* %var_ecx_2
  %var_ebx_3 = alloca i64
  store i64 0, i64* %var_ebx_3
  %var_edx_4 = alloca i64
  store i64 0, i64* %var_edx_4
  %t300 = load i64, i64* %var_eax_1
  %t301 = load i64, i64* %var_ebx_3
  %t302 = load i64, i64* %var_ecx_2
  %t303 = load i64, i64* %var_edx_4
  %t304 = load i64, i64* %var_eax_1
  %asm5 = call {i64, i64, i64, i64} asm sideeffect "cpuid", "={eax},={ebx},={ecx},={edx},0"(i64 %t304)
  %asmout6 = extractvalue {i64, i64, i64, i64} %asm5, 0
  store i64 %asmout6, i64* %var_eax_1
  %asmout7 = extractvalue {i64, i64, i64, i64} %asm5, 1
  store i64 %asmout7, i64* %var_ebx_3
  %asmout8 = extractvalue {i64, i64, i64, i64} %asm5, 2
  store i64 %asmout8, i64* %var_ecx_2
  %asmout9 = extractvalue {i64, i64, i64, i64} %asm5, 3
  store i64 %asmout9, i64* %var_edx_4
  %t305 = load i64, i64* %var_ecx_2
  %t306 = shl i64 1, 5
  %t307 = and i64 %t305, %t306
  %t308 = icmp ne i64 %t307, 0
  ret i1 %t308
}

define i64 @vmx_on() {
entry0:
  %var_cr4v_1 = alloca i64
  %t309 = call i64 @read_cr4()
  store i64 %t309, i64* %var_cr4v_1
  %t310 = load i64, i64* %var_cr4v_1
  %t311 = shl i64 1, 13
  %t312 = or i64 %t310, %t311
  call void @write_cr4(i64 %t312)
  %var_basic_2 = alloca i64
  %t313 = load i64, i64* @MSR_IA32_VMX_BASIC
  %t314 = call i64 @rdmsr(i64 %t313)
  store i64 %t314, i64* %var_basic_2
  %var_rev_3 = alloca i32
  %t315 = load i64, i64* %var_basic_2
  %t316 = and i64 %t315, 2147483647
  %cast4 = trunc i64 %t316 to i32
  store i32 %cast4, i32* %var_rev_3
  %var_region_ptr_5 = alloca i8*
  %t317 = load [4096 x i8], [4096 x i8]* @vmxon_region
  %ic6 = icmp slt i64 0, 0
  %ic7 = icmp sge i64 0, 4096
  %bad8 = or i1 %ic6, %ic7
  br i1 %bad8, label %idx_fail10, label %idx_ok9
idx_fail10:
  call void @ox_bounds_fail(i64 0, i64 4096)
  unreachable
idx_ok9:
  %ep11 = getelementptr inbounds [4096 x i8], [4096 x i8]* @vmxon_region, i64 0, i64 0
  %idx12 = load i8, i8* %ep11
  %ic13 = icmp slt i64 0, 0
  %ic14 = icmp sge i64 0, 4096
  %bad15 = or i1 %ic13, %ic14
  br i1 %bad15, label %idx_fail17, label %idx_ok16
idx_fail17:
  call void @ox_bounds_fail(i64 0, i64 4096)
  unreachable
idx_ok16:
  %ep18 = getelementptr inbounds [4096 x i8], [4096 x i8]* @vmxon_region, i64 0, i64 0
  store i8* %ep18, i8** %var_region_ptr_5
  %var_rev_ptr_19 = alloca i32*
  %t318 = load i8*, i8** %var_region_ptr_5
  %cast20 = bitcast i8* %t318 to i32*
  store i32* %cast20, i32** %var_rev_ptr_19
  %t319 = load i32*, i32** %var_rev_ptr_19
  %t320 = load i32, i32* %var_rev_3
  store volatile i32 %t320, i32* %t319
  %var_region_phys_21 = alloca i64
  %t321 = load i8*, i8** %var_region_ptr_5
  %cast22 = ptrtoint i8* %t321 to i64
  store i64 %cast22, i64* %var_region_phys_21
  %t322 = load i64, i64* %var_region_phys_21
  call void asm sideeffect "vmxon (%rax)", "{rax}"(i64 %t322)
  ret i64 0
}

define i64 @oxide_long_mode_entry() {
entry0:
  %var_sp_1 = alloca i64
  %t323 = call i64 @ox_entry_stack_top_addr()
  store i64 %t323, i64* %var_sp_1
  %t324 = load i64, i64* %var_sp_1
  call void asm sideeffect "mov %rax, %rsp", "{rax}"(i64 %t324)
  call void asm sideeffect "ltr %ax", "{ax}"(i64 24)
  call void @serial_init()
  %t325 = load i8*, i8** @MSG_BOOT
  call void @serial_puts(i8* %t325)
  %t326 = call i1 @has_vmx()
  %t327 = xor i1 %t326, true
  br i1 %t327, label %then2, label %else3
then2:
  %t328 = getelementptr inbounds [31 x i8], [31 x i8]* @str5, i64 0, i64 0
  call void @serial_puts(i8* %t328)
  call void @halt_forever()
  br label %merge4
else3:
  br label %merge4
merge4:
  %var_status_5 = alloca i64
  %t329 = call i64 @vmx_on()
  store i64 %t329, i64* %var_status_5
  %t330 = getelementptr inbounds [29 x i8], [29 x i8]* @str6, i64 0, i64 0
  call void @serial_puts(i8* %t330)
  %var_launch_rc_6 = alloca i64
  %t331 = call i64 @launch_guest()
  store i64 %t331, i64* %var_launch_rc_6
  %t332 = getelementptr inbounds [28 x i8], [28 x i8]* @str7, i64 0, i64 0
  call void @serial_puts(i8* %t332)
  call void @halt_forever()
  ret i64 0
}

