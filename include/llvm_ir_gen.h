#pragma once

#include "ast.h"

#include "llvm/ADT/APFloat.h"
#include "llvm/ADT/APInt.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Verifier.h"

namespace jcc {

class LLVMIRGen {
public:
    std::unique_ptr<llvm::LLVMContext> m_context;
    std::unique_ptr<llvm::Module> m_module;
    std::unique_ptr<llvm::IRBuilder<>> m_builder;
    std::unordered_map<std::string, std::pair<DeclNode *, llvm::Value *>>
        m_named_values;

    LLVMIRGen();

    llvm::Type *to_llvm_type(CType *);

    llvm::Value *gen_str_lit_expr(StrLitExprNode *);
    llvm::Value *gen_num_lit_expr(NumLitExprNode *);
    llvm::Value *gen_id_expr(IdExprNode *);
    llvm::Value *gen_address_of(ExprNode *);
    llvm::Value *gen_unary_expr(UnaryExprNode *);
    llvm::Value *gen_assign(BinExprNode *);
    llvm::Value *gen_bin_expr(BinExprNode *);
    llvm::Value *gen_call_expr(CallExprNode *);
    llvm::Value *gen_expr(ExprNode *);

    llvm::Value *gen_decl(DeclNode *);

    void gen_if_stmnt(IfStmntNode *);
    void gen_compound_stmnt(CompoundStmntNode *);
    void gen_stmnt(StmntNode *);

    llvm::Function *gen_prototype(PrototypeNode *);
    llvm::Function *gen_function(FunctionNode *);

    void gen_file(FileNode *);
};

} // namespace jcc
