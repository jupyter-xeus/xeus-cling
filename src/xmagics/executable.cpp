/***********************************************************************************
* Copyright (c) 2020, Jonas Hahnfeld                                               *
* Copyright (c) 2020, Chair for Computer Science 12 (HPC), RWTH Aachen University  *
*                                                                                  *
* Distributed under the terms of the BSD 3-Clause License.                         *
*                                                                                  *
* The full license is in the file LICENSE, distributed with this software.         *
************************************************************************************/

#include <algorithm>
#include <iostream>
#include <iterator>
#include <fstream>
#include <memory>
#include <string>
#include <vector>

#include "llvm/ADT/SmallString.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/FileUtilities.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/Path.h"
#include "llvm/Support/Program.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/DeclGroup.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Basic/DebugInfoOptions.h"
#include "clang/Basic/Sanitizers.h"
#include "clang/Basic/TargetInfo.h"
#include "clang/CodeGen/BackendUtil.h"
#include "clang/CodeGen/ModuleBuilder.h"
#include "clang/Frontend/CompilerInstance.h"
#include "cling/Interpreter/Interpreter.h"
#include "cling/Interpreter/Transaction.h"

#include "xeus-cling/xoptions.hpp"

#include "../xparser.hpp"

#include "executable.hpp"

namespace xcpp
{
    xoptions executable::get_options()
    {
        xoptions options{"executable", "write executable"};
        options.add_options()
            ("f,filename", "filename",
             cxxopts::value<std::string>()->default_value(""))
            ("o,options", "options",
             cxxopts::value<std::vector<std::string>>()->default_value(""));
        options.parse_positional({"filename", "options"});
        return options;
    }

    std::string executable::generate_fns(const std::string& cell,
                                         std::string& main,
                                         std::string& unique_fn)
    {
        // See https://en.cppreference.com/w/cpp/language/main_function
        // TODO: Find out if argc and argv would make problems if declared as
        // arguments and in the body.

        // Generate a unique fn that is not unloaded after generating the
        // executable. This is necessary for templates like std::endl to
        // work correctly in subsequent cells.
        std::string fn_name = "__xeus_cling_main_wrapper_";
        fn_name += std::to_string(m_unique++);
        unique_fn = "int " + fn_name + "() {\n";
        unique_fn += cell + "\n";
        unique_fn += "return 0;\n";
        unique_fn += "}";

        main = "int main() {\n";
        main += "return " + fn_name + "();\n";
        main += "}";
        return main;
    }

    class FindTopLevelDecls
        : public clang::RecursiveASTVisitor<FindTopLevelDecls>
    {
    public:
        FindTopLevelDecls(clang::ASTConsumer* C) : m_consumer(C) {}

        bool shouldVisitTemplateInstantiations() { return true; }

        bool VisitFunctionDecl(clang::FunctionDecl* D)
        {
            if (!D->getName().startswith("__cling"))
            {
                m_consumer->HandleTopLevelDecl(clang::DeclGroupRef(D));
            }
            return true;
        }

        bool VisitVarDecl(clang::VarDecl* D)
        {
            if (D->isFileVarDecl())
            {
                m_consumer->HandleTopLevelDecl(clang::DeclGroupRef(D));
            }
            return true;
        }

    private:
        clang::ASTConsumer* m_consumer;
    };

    bool executable::generate_obj(std::string& ObjectFile, bool EnableDebugInfo)
    {
        // Generate LLVM IR for current AST.
        auto* CI = m_interpreter.getCI();
        auto* Context = m_interpreter.getLLVMContext();
        auto& AST = CI->getASTContext();
        auto& HeaderSearchOpts = CI->getHeaderSearchOpts();

        // Generate relocations suitable for dynamic linking.
        auto CodeGenOpts = CI->getCodeGenOpts();
        CodeGenOpts.RelocationModel = "pic";

        // Enable debug information if requested.
        if (EnableDebugInfo)
        {
            CodeGenOpts.setDebugInfo(
                clang::codegenoptions::DebugInfoKind::FullDebugInfo);
        }

        std::unique_ptr<clang::CodeGenerator> CG(clang::CreateLLVMCodeGen(
            CI->getDiagnostics(), "object", HeaderSearchOpts,
            CI->getPreprocessorOpts(), CodeGenOpts, *Context));
        CG->Initialize(AST);

        FindTopLevelDecls Visitor(CG.get());
        Visitor.TraverseDecl(AST.getTranslationUnitDecl());

        CG->HandleTranslationUnit(AST);

        // Generate (temporary) object code from LLVM IR.
        int ObjectFD;
        llvm::SmallString<64> ObjectFilePath;
        std::error_code EC = llvm::sys::fs::createTemporaryFile(
            "object", "o", ObjectFD, ObjectFilePath);
        if (EC)
        {
            std::cerr << "Could not create temporary object file:" << std::endl
                      << EC.message() << std::endl;
            return false;
        }
        ObjectFile = ObjectFilePath.str();

        std::unique_ptr<llvm::raw_pwrite_stream> OS(
            new llvm::raw_fd_ostream(ObjectFD, true));

        auto DataLayout = AST.getTargetInfo().getDataLayout();
        EmitBackendOutput(CI->getDiagnostics(), HeaderSearchOpts,
                          CodeGenOpts, CI->getTargetOpts(),
                          CI->getLangOpts(), DataLayout, CG->GetModule(),
                          clang::Backend_EmitObj, std::move(OS));
        return true;
    }

    bool executable::generate_exe(const std::string& ObjectFile,
                                  const std::string& ExeFile,
                                  const std::vector<std::string>& LinkerOptions)
    {
        auto& HeaderSearchOpts = m_interpreter.getCI()->getHeaderSearchOpts();
        // Generate executable by linking the created object code.
        llvm::StringRef InstallDir = llvm::sys::path::parent_path(
            llvm::sys::path::parent_path(
                llvm::sys::path::parent_path(HeaderSearchOpts.ResourceDir)));
        llvm::SmallString<256> Compiler(InstallDir);
        llvm::sys::path::append(Compiler, "bin", "clang++");

        // Construct arguments to linker command.
        llvm::SmallVector<const char*, 16> Args;
        Args.push_back(Compiler.c_str());
        Args.push_back(ObjectFile.c_str());
        for (auto& O : LinkerOptions)
        {
            Args.push_back(O.c_str());
        }
        Args.push_back("-o");
        Args.push_back(ExeFile.c_str());
        Args.push_back(NULL);

        // Redirect output and error streams from linker.
        llvm::SmallString<64> OutputFile, ErrorFile;
        llvm::sys::fs::createTemporaryFile("linker", "out", OutputFile);
        llvm::sys::fs::createTemporaryFile("linker", "err", ErrorFile);
        llvm::FileRemover OutputRemover(OutputFile.c_str());
        llvm::FileRemover ErrorRemover(ErrorFile.c_str());

        llvm::StringRef OutputFileStr(OutputFile);
        llvm::StringRef ErrorFileStr(ErrorFile);
        const llvm::StringRef* Redirects[] = {nullptr, &OutputFileStr,
                                              &ErrorFileStr};

        // Finally run the linker.
        int ret = llvm::sys::ExecuteAndWait(Compiler, Args.data(), nullptr,
                                            Redirects);

        // Read back output and error streams.
        llvm::StringRef OutputStr, ErrorStr;
        auto OutputBuf = llvm::MemoryBuffer::getFile(OutputFileStr);
        if (OutputBuf)
        {
            OutputStr = OutputBuf.get()->getBuffer();
        }
        auto ErrorBuf = llvm::MemoryBuffer::getFile(ErrorFileStr);
        if (ErrorBuf)
        {
            ErrorStr = ErrorBuf.get()->getBuffer();
        }

        // Forward to user.
        if (!OutputStr.empty())
        {
            std::cout << "---" << std::endl;
            std::cout << OutputStr.str();
        }
        if (!ErrorStr.empty())
        {
            std::cerr << ErrorStr.str();
            return false;
        }
        else if (ret != 0)
        {
            // At least let the user know that something went wrong.
            std::cerr << "Could not link executable" << std::endl;
            return false;
        }

        // Return success!
        return true;
    }

    void executable::operator()(const std::string& line, const std::string& cell)
    {
        auto options = get_options().parse(line);

        std::string ExeFile = options["filename"].as<std::string>();
        if (ExeFile.empty())
        {
            std::cerr << "UsageError: "
                      << "the following arguments are required: filename"
                      << std::endl;
            return;
        }
        std::vector<std::string> LinkerOptions =
            options["options"].as<std::vector<std::string>>();

        std::string main, unique_fn;
        generate_fns(cell, main, unique_fn);
        // First declare the unique_fn that is not unloaded.
        auto result = m_interpreter.declare(unique_fn);
        if (result != cling::Interpreter::kSuccess)
        {
            return;
        }

        // Now declare main() function.
        cling::Transaction* t = nullptr;
        result = m_interpreter.declare(main, &t);
        if (result != cling::Interpreter::kSuccess || t == nullptr)
        {
            return;
        }

        // Make sure to unload the transaction that added the main() function.
        // This enables repeated execution of a %%executable cell.
        struct Unloader
        {
            cling::Interpreter& m_interpreter;
            cling::Transaction& m_transaction;
            Unloader(cling::Interpreter& i, cling::Transaction& t)
                : m_interpreter(i), m_transaction(t) {}
            ~Unloader()
            {
                m_interpreter.unload(m_transaction);
            }
        }
        unloader(m_interpreter, *t);

        // Enable debug information if user requested -g in the linker options.
        bool EnableDebugInfo =
            (std::find(LinkerOptions.begin(), LinkerOptions.end(),
                       "-g") != LinkerOptions.end());
        if (EnableDebugInfo)
        {
            std::cout << "Enabling debug information" << std::endl;
        }

        // Enable TSan instrumentation if user requested -fsanitize=thread in
        // the linker options.
        bool SanitizeThread =
            (std::find(LinkerOptions.begin(), LinkerOptions.end(),
                       "-fsanitize=thread") != LinkerOptions.end());
        auto& SanitizeOpts = m_interpreter.getCI()->getLangOpts().Sanitize;
        if (SanitizeThread)
        {
            std::cout << "Enabling instrumentation for ThreadSanitizer"
                      << std::endl;
            SanitizeOpts.set(clang::SanitizerKind::Thread, true);

            // Imply debug information because it gives the user a clue which
            // line of the input caused the race.
            EnableDebugInfo = true;
        }

        std::cout << "Writing executable to " << ExeFile << std::endl;

        std::string ObjectFile;
        if (!generate_obj(ObjectFile, EnableDebugInfo))
        {
            return;
        }
        // Cleanup after we exit.
        llvm::FileRemover ObjectRemover(ObjectFile);

        generate_exe(ObjectFile, ExeFile, LinkerOptions);

        if (SanitizeThread)
        {
            SanitizeOpts.set(clang::SanitizerKind::Thread, false);
        }
    }
}
