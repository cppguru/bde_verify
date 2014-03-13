// csamisc_arrayinitialization.cpp                                    -*-C++-*-
// -----------------------------------------------------------------------------
// Copyright 2012 Dietmar Kuehl http://www.dietmar-kuehl.de              
// Distributed under the Boost Software License, Version 1.0. (See file  
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt).     
// -----------------------------------------------------------------------------

#include <csabase_analyser.h>
#include <csabase_registercheck.h>
#include <set>
#ident "$Id$"

// -----------------------------------------------------------------------------

static std::string const check_name("array-initialization");

// -----------------------------------------------------------------------------

static bool isDefaultConstructor(bde_verify::csabase::Analyser& analyser,
                                 clang::Expr const* init)
{
    clang::CXXConstructExpr const* ctor = llvm::dyn_cast<clang::CXXConstructExpr>(init);
    return ctor
        && (ctor->getNumArgs() == 0
            || (ctor->getNumArgs() == 1 && llvm::dyn_cast<clang::CXXDefaultArgExpr>(ctor->getArg(0))));
}

// -----------------------------------------------------------------------------

static bool isDefaultValue(bde_verify::csabase::Analyser& analyser,
                           clang::InitListExpr const* expr,
                           clang::Expr const* init)
{
    clang::Expr const* orig(init); 
    do
    {
        orig = init;
        init = const_cast<clang::Expr*>(init)->IgnoreImplicit();

        if (clang::CastExpr const* cast = llvm::dyn_cast<clang::CastExpr>(init))
        {
            init = cast->getSubExpr();
        }
        else if (clang::CXXConstructExpr const* ctor = llvm::dyn_cast<clang::CXXConstructExpr>(init))
        {
            if (ctor->getNumArgs() == 1
                && llvm::dyn_cast<clang::MaterializeTemporaryExpr>(ctor->getArg(0)))
            {
                init = llvm::dyn_cast<clang::MaterializeTemporaryExpr>(ctor->getArg(0))->GetTemporaryExpr();
            }
        }
    }
    while (orig != init);

    return  llvm::dyn_cast<clang::CXXScalarValueInitExpr>(init)
        || (llvm::dyn_cast<clang::CharacterLiteral>(init)
            && llvm::dyn_cast<clang::CharacterLiteral>(init)->getValue() == 0)
        || (llvm::dyn_cast<clang::IntegerLiteral>(init)
            && llvm::dyn_cast<clang::IntegerLiteral>(init)->getValue().getLimitedValue() == 0u)
        || isDefaultConstructor(analyser, init);
}

// -----------------------------------------------------------------------------

namespace
{
    struct reported
    {
        std::set<void const*> reported_;
    };
}

static void
check(bde_verify::csabase::Analyser& analyser, clang::InitListExpr const* expr)
{
    clang::Type const* type(expr->getType().getTypePtr());
    if (type->isConstantArrayType()
        && !expr->isStringLiteralInit()
        )
    {
        clang::ConstantArrayType const* array(analyser.context()->getAsConstantArrayType(expr->getType()));
        if (0u < expr->getNumInits()
            && expr->getNumInits() < array->getSize().getLimitedValue()
            && !isDefaultValue(analyser, expr, expr->getInit(expr->getNumInits() - 1u))
            && analyser.attachment<reported >().reported_.insert(expr).second)
        {
            analyser.report(expr, check_name, "II01",
                    "Incomplete initialization with non-defaulted last value")
                << expr->getInit(expr->getNumInits() - 1u)->getSourceRange();
        }
    }
}

// -----------------------------------------------------------------------------

static bde_verify::csabase::RegisterCheck register_check(check_name, &check);
