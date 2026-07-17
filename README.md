# Oxide

Oxide is a small compiled programming language with C like curly brace syntax.
It is written in C++17 and targets LLVM. The compiler emits textual LLVM IR and
drives clang to assemble, optimize, and link it. Oxide links nothing against
LLVM. clang runs out of process on the generated IR.

Oxide has two faces. It is a general language for hosted programs, and it is a
freestanding toolchain for bare metal targets. The freestanding path is where
Oxide earns its keep. It ships inline assembly, volatile MMIO, and pointer
arithmetic so a kernel or a Type 1 hypervisor is expressible in the language
itself. A working multiboot2 hypervisor kernel lives in `hv/` and does `vmxon`,
`vmlaunch`, and a VM exit decode loop, all written in Oxide.

## Status

Experimental, single author, work in progress. The freestanding and hypervisor
path is the proven lane and the recommended place to start. Hosted linking
depends on a system C linker being present and is undertested on this machine.
Generics are not a shipped feature. Nothing in the hypervisor stack has been
booted on hardware. Machine code in `hv/` is verified against the Intel SDM by
disassembly, not by running it in a VM.

## Build

Build the compiler with any C++17 compiler.

```
clang++ -std=c++17 -O2 src/*.cpp -o oxide
```

On Windows, the LLVM install at `C:\Program Files\LLVM` ships a working
`clang++`. A `clang` or `clang-cl` or `gcc` or `cl` and the LLVM `lld` linker
must be reachable on PATH when you run `oxide build` or `oxide exe` or
`oxide run`.

A CMake project is included.

```
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

## Usage

```
oxide run   program.ox    compile and run via a temp executable
oxide emit  program.ox    print the generated LLVM IR to stdout
oxide build program.ox    emit an object file (.o)
oxide exe   program.ox    compile to a native executable
oxide rt    [-o path.c]   print or write the bundled C runtime for separate linking
```

Options, any of these may follow the command.

```
-o PATH          output path for build, exe, or rt
-O0              disable the default -O2 pass
--target TRIPLE  put a target triple into the IR, default is host and omitted
--freestanding   alias for --no-rt, skip the C runtime and the @main rename wrapper
--entry NAME     the program entry symbol for freestanding mode, default main
```

`--target` lets you cross emit, for example `--target aarch64-linux-gnu`. By
default no target triple line is written so the host clang applies its own and
no conflicting override warning appears. `--freestanding` produces a bare object
or exe that pulls in no `ox_*` runtime. It is intended for kernels and embedded
targets. The program's own functions, named by `--entry` and unmangled, become
the link symbols in freestanding mode. Runtime backed intrinsics such as `print`
are then a sema error unless you supply your own via `extern`.

## Language

Types are `i64`, `f64`, `bool`, `str`, and `void`. Fixed size arrays use
`[T; n]`. Structs and enums are user defined. Pointers and references use `&T`
with auto deref on `.field` and `[i]`. Dynamic arrays use `vec[T]`. Structural
aliases use `typedef`.

Binding. `let` is immutable. `mut` or `let mut` is mutable. The compiler rejects
reassignment of immutable variables.

```
fn fib(n: i64) -> i64 {
  if n < 2 { return n; }
  return fib(n - 1) + fib(n - 2);
}

fn main() -> i64 {
  let mut sum = 0;
  for let mut i = 0; i < 10; i = i + 1 {
    sum = sum + fib(i);
  }
  print("fib sum 0..9 =", sum);
  return 0;
}
```

Control flow. `if`, `else`, `else if`, `while`, C style `for`, `break`,
`continue`, `return`. Parentheses around `if`, `while`, and `for` headers are
optional.

Operators. Arithmetic, comparison, logical with short circuit `&&` `||` `!`,
bitwise `& | ^ ~ << >>`, and all the compound assignments. C and C++ pre and post
increment and decrement `++x` `--x` `x++` `x--` mutate any lvalue of int, char,
usize, bool, or pointer type. For a pointer the step is one pointee element, the
same as `p + 1`. The C and C++ ternary `cond ? then : else` yields the selected
arm. Arms may be any type unified like a value returning `if` and `else`, and it
nests right associative. Comparison `< <= > >=` also orders `str` pairs
lexicographically, and `sizeof(T)` yields a type's byte size at compile time.

```
let mut i: i64 = 0;
let a = i++;
let b = ++i;
let parity = (i % 2 == 0) ? "even" : "odd";
let pick = (n > 5) ? P{x:1,y:2} : P{x:9,y:8};
```

`print` takes any number of arguments of any supported type and prints them on
one line with a trailing newline. Arrays print as `[1, 2, 3]` and structs print
as `Name{field: v, ...}`, recursively.

Functions may be recursive and may be called before their definition. Forward
references resolve within a module. `oxide run` and `oxide exe` require a `main`.
`oxide build` and `oxide emit` do not, so a unit may be a library. In
freestanding mode `--entry` names the entry instead.

### Structs

A `struct` declares a record with named fields. Construct it with
`Name { field: value, ... }`, read a field with `p.x`, and assign a field with
`p.x = v`. Structs are value types, assignment copies, and they may contain other
structs.

```
struct Point { x: i64; y: i64; }
struct Rect { tl: Point; br: Point; }

fn area(r: Rect) -> i64 {
  return (r.br.x - r.tl.x) * (r.br.y - r.tl.y);
}

fn main() -> i64 {
  let mut p: Point = Point { x: 1, y: 2 };
  p.x = p.x + 5;
  print("p=", p);
  print("p.x=", p.x);

  let r = Rect {
    tl: Point { x: 0, y: 0 },
    br: Point { x: 4, y: 5 }
  };
  print("rect area=", area(r));
  return 0;
}
```

### Enums and match

An `enum` declares a set of named tags. A `match` dispatches on a tag. The
underscore `_` is a catch all arm.

```
enum Color { Red, Green, Blue }

fn name(c: Color) -> i64 {
  match c {
    Red   => { print("red");   return 1; }
    Green => { print("green"); return 2; }
    Blue  => { print("blue");  return 3; }
  }
  return 0;
}

fn main() -> i64 {
  let c: Color = Green;
  print(name(c));
  print(c == Green);
  print(c as i64);
  return 0;
}
```

### Vec

Dynamic arrays `vec[T]` grow at run time and bounds check on index. Vec works for
any fixed size element type. Scalars take a fast path. Structs, arrays, and
nested vecs go through a memcpy path keyed on the element byte width.

### Methods and operator overloading

`impl` blocks attach methods to a type. Receivers are `self` or `&self`.
Associated functions use `Type::name`. Fields may be `private`.

Operator overloading is by named methods. `__add __sub __mul __div __mod`,
`__eq` with `!=` defined as `!__eq`, `__lt __le __gt __ge`,
`__band __bor __bxor __shl __shr`, unary `__neg`, `__index`,
`__iadd` through `__imod`, and `__assign`. Load the `examples/operator_overload.ox`
and `examples/methods.ox` examples to see the full surface.

### Freestanding and the hypervisor

Freestanding mode drops the C runtime and the `@main` to `@oxide_main` wrapper.
It exposes four bare metal primitives that a Type 1 hypervisor needs.

1. Inline assembly `asm!(...)` lowers to an LLVM inline asm node.
2. Volatile MMIO `mmio_load` and `mmio_store` lower to `load volatile` and
   `store volatile`.
3. Block fill and copy `memset` and `memcpy` lower to the LLVM memset and memcpy
   intrinsics.
4. Pointer arithmetic `p + n` and `p - n` lowers to `getelementptr`. Element
   stride is the default, so `p + n` means `p + n * sizeof(T)`. Byte stride uses
   `p as *u8 + n`.

The `hv/` directory is a real multiboot2 Type 1 hypervisor written in Oxide.

```
hv/
  boot/
    stub.S        32 bit protected mode to long mode entry, GDT, TSS, page tables
    vmcs_syms.S   asm functions that resolve Oxide global symbol addresses
    linker.ld     multiboot2 load layout, entry _start, load at 1 MiB
  src/
    vmx.ox        every VMCS field encoding from SDM appendix B plus vmx_adjust_ctl
    vmxops.ox     vmclear, vmptrld, vmwrite, vmread, vmlaunch, vmresume, vmxoff, vmcall
    vmlaunch.ox   build a 64 bit guest VMCS, vmlaunch, and the VM exit handler
    kernel.ox     serial COM1, VT-x cpuid probe, CR4.VMXE, IA32_VMX_BASIC, vmxon
    freestanding_runtime.ox  bare metal ox_bounds_fail, the only ox_* the kernel needs
  build.sh        concat the sources in dep order, emit IR, clang c, ld.lld link
```

Build it.

```
bash hv/build.sh
```

This needs `clang`, `ld.lld`, and `python` on PATH. It produces
`hv/build_out/kernel.elf`. The build validates the multiboot2 header with
`hv/build/mb2check.py`. Booting the result needs `qemu-system-x86_64`, which is
not present in this environment, so the kernel is verified by disassembly against
the Intel SDM and not by running it.

## Examples

The `examples/` directory is the regression suite. A representative sample.

```
fib.ox             recursive function
structs.ox         value typed records
enums.ox           enum and match
methods.ox         impl blocks, self and &self
operator_overload.ox   the full operator set as named methods
strings.ox         first class str, concat, comparison
foreach.ox         for in iteration
vec_structs.ox     vec of struct elements
sizeof.ox          compile time sizeof(T)
inc_ternary.ox     ++ and -- and the ternary
hv_enablers.ox     the four freestanding hypervisor primitives in one file
extern/            separate compilation demo
freestanding/      runtime less object with unmangled symbols
```

## Repository layout

```
src/        the compiler, C++17
  main.cpp        CLI entry
  Lexer.*         tokenizer
  Parser.*        recursive descent parser
  Sema.*          type checking and mutability
  Types.cpp       the BType value type and type tables
  IRGen.*         textual LLVM IR generation
  Driver.*        pipeline, runtime, freestanding, clang invocation
  AST.h           the syntax tree
examples/   Oxide programs and the regression suite
hv/         the multiboot2 hypervisor kernel in Oxide and assembly
CMakeLists.txt   cmake build for the compiler
build.bat, build_debug.bat   Windows build helpers
run.bat, run_dbg.bat         Windows run helpers using the built oxide.exe
test.sh          run an example end to end
```

## License

This project is released as experimental. No license file is included by
default.
