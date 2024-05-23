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

    // TODO reorder all of this
    // it's in reverse compared to parser

    void genFile(FileNode *);

    llvm::Function *genFunction(FunctionNode *);
    llvm::Function *genPrototype(PrototypeNode *);

    void genCompoundStmnt(CompoundStmntNode *);
    void genStmnt(StmntNode *);
    llvm::Value *genDecl(DeclNode *);

    llvm::Value *genExpr(ExprNode *);
    llvm::Value *genCallExpr(CallExprNode *);
    llvm::Value *genBinExpr(BinExprNode *);
    llvm::Value *genUnaryExpr(UnaryExprNode *);
    llvm::Value *genAddressOf(ExprNode *);
    llvm::Value *genIdExpr(IdExprNode *);
    llvm::Value *genNumLitExpr(NumLitExprNode *);
};

} // namespace jcc
