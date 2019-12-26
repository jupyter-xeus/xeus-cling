/***********************************************************************************
* Copyright (c) 2016, Johan Mabille, Loic Gouarin, Sylvain Corlay, Wolf Vollprecht *
* Copyright (c) 2016, QuantStack                                                   *
*                                                                                  *
* Distributed under the terms of the BSD 3-Clause License.                         *
*                                                                                  *
* The full license is in the file LICENSE, distributed with this software.         *
************************************************************************************/

#ifndef XCPP_MIME_INTERNAL_HPP
#define XCPP_MIME_INTERNAL_HPP

#include <cstddef>
#include <locale>
#include <string>

#include "nlohmann/json.hpp"

#include "cling/Interpreter/Exception.h"
#include "cling/Interpreter/CValuePrinter.h"
#include "cling/Interpreter/Interpreter.h"
#include "cling/Interpreter/InterpreterCallbacks.h"
#include "cling/Interpreter/LookupHelper.h"
#include "cling/Interpreter/Transaction.h"
#include "cling/Interpreter/Value.h"
#include "cling/Utils/AST.h"
#include "cling/Utils/Output.h"
#include "cling/Utils/Validation.h"

#include "clang/AST/ASTContext.h"
#include "clang/AST/Decl.h"
#include "clang/AST/DeclCXX.h"
#include "clang/AST/Expr.h"
#include "clang/AST/Type.h"
#include "clang/Frontend/CompilerInstance.h"

#include "llvm/ExecutionEngine/GenericValue.h"
#include "llvm/Support/Format.h"
#include "llvm/Support/raw_ostream.h"

namespace nl = nlohmann;

namespace xcpp
{

    /**************************************************************************
    * The content of the cling_detail namespace is derived from cling which   *
    * licensed under the UI/NCSAOSL license.                                  *
    ***************************************************************************/

    namespace cling_detail
    {
        struct LockCompilationDuringUserCodeExecutionRAII
        {
            /// Callbacks used to un/lock.
            cling::InterpreterCallbacks* fCallbacks;

            /// Info provided to UnlockCompilationDuringUserCodeExecution().
            void* fStateInfo = nullptr;

            LockCompilationDuringUserCodeExecutionRAII(cling::InterpreterCallbacks* callbacks)
                : fCallbacks(callbacks)
            {
                if (fCallbacks)
                {
                    fStateInfo = fCallbacks->LockCompilationDuringUserCodeExecution();
                }
            }

            LockCompilationDuringUserCodeExecutionRAII(cling::Interpreter& interp)
                : LockCompilationDuringUserCodeExecutionRAII(interp.getCallbacks())
            {
            }

            ~LockCompilationDuringUserCodeExecutionRAII()
            {
                if (fCallbacks)
                {
                    fCallbacks->UnlockCompilationDuringUserCodeExecution(fStateInfo);
                }
            }
        };

        struct AccessCtrlRAII_t
        {
            bool savedAccessControl;
            clang::LangOptions& LangOpts;

            AccessCtrlRAII_t(cling::Interpreter &Interp)
              : LangOpts(const_cast<clang::LangOptions &>(Interp.getCI()->getLangOpts()))
            {
                savedAccessControl = LangOpts.AccessControl;
                LangOpts.AccessControl = false;
            }

            ~AccessCtrlRAII_t()
            {
                LangOpts.AccessControl = savedAccessControl;
            }
        };

        static std::string enclose(std::string Mid, const char* Begin, const char* End, std::size_t Hint = 0)
        {
            Mid.reserve(Mid.size() + Hint ? Hint : (::strlen(Begin) + ::strlen(End)));
            Mid.insert(0, Begin);
            Mid.append(End);
            return Mid;
        }

        static std::string enclose(const clang::QualType &Ty, clang::ASTContext &C,
                                   const char* Begin = "(", const char* End = "*)",
                                   std::size_t Hint = 3)
        {
            return enclose(cling::utils::TypeName::GetFullyQualifiedName(Ty, C), Begin, End, Hint);
        }

        static clang::QualType getElementTypeAndExtent(const clang::ConstantArrayType* CArrTy, std::string& extent)
        {
            clang::QualType ElementTy = CArrTy->getElementType();
            const llvm::APInt &APSize = CArrTy->getSize();
            extent += '[' + std::to_string(APSize.getZExtValue()) + ']';
            if (auto CArrElTy = llvm::dyn_cast<clang::ConstantArrayType>(ElementTy.getTypePtr()))
            {
                return getElementTypeAndExtent(CArrElTy, extent);
            }
            return ElementTy;
        }

        static std::string getTypeString(const cling::Value& V)
        {
            clang::ASTContext& C = V.getASTContext();
            clang::QualType Ty = V.getType().getDesugaredType(C).getNonReferenceType();

            if (llvm::dyn_cast<clang::BuiltinType>(Ty.getCanonicalType()))
            {
                return enclose(Ty, C);
            }

            if (Ty->isPointerType())
            {
                // Print char pointers as strings.
                if (Ty->getPointeeType()->isCharType())
                {
                    return enclose(Ty, C);
                }

                // Fallback to void pointer for other pointers and print the address.
                return "(const void**)";
             }

            if (Ty->isArrayType())
            {
                if (Ty->isConstantArrayType())
                {
                    std::string extent("(*)");
                    clang::QualType InnermostElTy = getElementTypeAndExtent(C.getAsConstantArrayType(Ty), extent);
                    return enclose(InnermostElTy, C, "(", (extent + ")*(void**)").c_str());
                }
                return "(void**)";
            }
            if (Ty->isObjCObjectPointerType())
            {
                return "(const void**)";
            }

            // In other cases, dereference the address of the object.
            // If no overload or specific template matches,
            // the general template will be used which only prints the address.
            return enclose(Ty, C, "*(", "**)", 5);
        }
    }

    inline nl::json mime_repr(const cling::Value& V)
    {
        // Return a JSON mime bundle representing the specified value.

        cling::Interpreter *interpreter = V.getInterpreter();
        const void* value = V.getPtr();

        // Include "xmime.hpp" only on the first time a variable is displayed.
        static bool xmime_included = false;

        if (!xmime_included)
        {
            cling_detail::LockCompilationDuringUserCodeExecutionRAII LCDUCER(*interpreter);
            interpreter->declare("#include \"xcpp/xmime.hpp\"");
            xmime_included = true;
        }

        cling::Value mimeReprV;
        {
            // Use an llvm::raw_ostream to prepend '0x' in front of the pointer value.
            cling::ostrstream code;
            code << "using xcpp::mime_bundle_repr;";
            code << "mime_bundle_repr(";
            code << "*(" << xcpp::cling_detail::getTypeString(V);
            code << &value;
            code << "));";

            cling_detail::AccessCtrlRAII_t AccessCtrlRAII(*interpreter);
            cling_detail::LockCompilationDuringUserCodeExecutionRAII LCDUCER(*interpreter);
            interpreter->process(code.str(), &mimeReprV);
        }

        if (mimeReprV.isValid() && mimeReprV.getPtr())
        {
            return *(nl::json*)mimeReprV.getPtr();
        }
        else
        {
            return nl::json::object();
        }
    }
}

#endif
