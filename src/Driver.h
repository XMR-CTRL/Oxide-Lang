#pragma once

#include <string>
#include <vector>
#include <memory>
#include <set>

#include "Lexer.h"

struct CompileError {
  std::string stage;
  std::string msg;
  int line;
  int col = 0;
  std::string hint;
};

enum class Action { run, emit, build, exe };

struct Options {
  Action action = Action::run;
  std::string input;
  std::string output;
  bool optimize = true;


  std::string targetTriple;
  bool freestanding = false;
  std::string entry = "main";


  std::vector<std::string> linkLibs;
  std::vector<std::string> linkFlags;
};

class Driver {
public:
  bool run(const Options& opt);

  void printErrors(const std::string& file) const;

  std::vector<CompileError> errs;

private:
  std::string ir_;
  std::vector<std::string> srcLines_;
  std::set<std::string> importVisited_;

  bool doRun(const Options& opt);
  bool doEmit();
  bool doBuild(const Options& opt, const std::string& outPath);
  bool doExe(const Options& opt, const std::string& outPath);

  std::string findTool(const std::vector<const char*>& names);
  int runCmd(const std::string& cmd);
  static std::string optFlag(bool optimize) { return optimize ? "-O2" : "-O0"; }


  static std::string renameMain(const std::string& ir);


  std::vector<Token> resolveImports(std::vector<Token>& toks, const std::string& dir,
                                    std::vector<CompileError>& lexErrs);
};
