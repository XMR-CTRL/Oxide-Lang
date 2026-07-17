#include "Driver.h"
#include <cstdio>
#include <fstream>
#include <string>
#include <vector>


std::string runtimeC();

static void usage(const char* argv0) {
  std::printf(
    "oxide compiler\n"
    "usage: %s <command> [options] <file.ox>\n"
    "commands:\n"
    "  run     jit compile and run the program\n"
    "  emit    print LLVM IR to stdout\n"
    "  build   emit an object file (.o)\n"
    "  exe     compile to a native executable\n"
    "  rt      print (or -o PATH write) the bundled C runtime for separate linking\n"
    "options:\n"
    "  -o PATH          output path (build/exe/rt)\n"
    "  -O0              disable optimization\n"
    "  --target TRIPLE  emit a target triple into the IR (default: host, omitted)\n"
    "  --freestanding   skip the C runtime + the @main->@oxide_main wrapper (--no-rt)\n"
    "  --entry NAME     the program entry symbol (freestanding), default `main`\n"
    "  --link NAME      link a native library (adds `-l NAME`); repeatable. e.g.\n"
    "                   --link user32 --link kernel32 to call Win32 from `extern fn`\n"
    "  -Wl,FLAG         pass a raw flag straight to the linker; repeatable\n",
    argv0);
}

int main(int argc, char** argv) {
  if (argc < 2) { usage(argv[0]); return 1; }

  Options opt;
  std::string cmd = argv[1];
  if (cmd == "run") opt.action = Action::run;
  else if (cmd == "emit") opt.action = Action::emit;
  else if (cmd == "build") opt.action = Action::build;
  else if (cmd == "exe") opt.action = Action::exe;


  else if (cmd == "rt") {
    std::string rt = runtimeC();
    bool writeFile = false;
    std::string outPath;
    for (int i = 2; i < argc; i++) { std::string a = argv[i]; if (a == "-o" && i + 1 < argc) { outPath = argv[++i]; writeFile = true; } }
    if (writeFile) {
      std::ofstream f(outPath, std::ios::binary);
      if (!f) { std::printf("error: cannot open '%s' for write\n", outPath.c_str()); return 1; }
      f << rt;
      return (bool)f ? 0 : 1;
    }
    std::fputs(rt.c_str(), stdout);
    return 0;
  }
  else if (cmd == "-h" || cmd == "--help" || cmd == "help") { usage(argv[0]); return 0; }
  else {
    std::printf("unknown command '%s'\n", cmd.c_str());
    usage(argv[0]);
    return 1;
  }

  for (int i = 2; i < argc; i++) {
    std::string a = argv[i];
    if (a == "-o") {
      if (i + 1 < argc) { opt.output = argv[++i]; }
      else { std::printf("error: -o requires a path\n"); return 1; }
    } else if (a == "-O0") {
      opt.optimize = false;
    } else if (a == "--target") {
      if (i + 1 < argc) { opt.targetTriple = argv[++i]; }
      else { std::printf("error: --target requires a triple\n"); return 1; }
    } else if (a == "--freestanding" || a == "--no-rt") {
      opt.freestanding = true;
    } else if (a == "--entry") {
      if (i + 1 < argc) { opt.entry = argv[++i]; }
      else { std::printf("error: --entry requires a name\n"); return 1; }
    } else if (a == "--link" || a == "-l") {
      if (i + 1 < argc) { opt.linkLibs.push_back(argv[++i]); }
      else { std::printf("error: --link requires a library name\n"); return 1; }
    } else if (a.rfind("-Wl,", 0) == 0) {


      opt.linkFlags.push_back(a);
    } else {
      opt.input = a;
    }
  }

  if (opt.input.empty()) {
    std::printf("error: no input file given\n");
    usage(argv[0]);
    return 1;
  }

  Driver driver;
  if (!driver.run(opt)) {
    if (driver.errs.empty()) {
      std::printf("error: compilation failed\n");
    } else {
      driver.printErrors(opt.input);
    }
    return 1;
  }
  return 0;
}
