// csamisc_componentheaderinclude.cpp                                 -*-C++-*-
// ----------------------------------------------------------------------------
// Copyright 2012 Dietmar Kuehl http://www.dietmar-kuehl.de              
// Distributed under the Boost Software License, Version 1.0. (See file  
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt).     
// ----------------------------------------------------------------------------

#include <csabase_analyser.h>
#include <csabase_ppobserver.h>
#include <csabase_registercheck.h>
#include <csabase_location.h>
#include <csabase_debug.h>
#include <utils/array.hpp>
#include <llvm/Support/raw_ostream.h>
#include <limits>
#ident "$Id$"

// ----------------------------------------------------------------------------

static std::string const check_name("component-header");

// ----------------------------------------------------------------------------

namespace
{
    struct status
    {
        status():
            check_(true),
            header_seen_(false),
            line_(std::numeric_limits<size_t>::max())
        {
        }
        bool   check_;
        bool   header_seen_;
        size_t line_;
    };
}

// ----------------------------------------------------------------------------

static std::string const builtin("<built-in>");
static std::string const command_line("<command line>");
static std::string const id_names[] = { "RCSId" };

// ----------------------------------------------------------------------------

static void 
close_file(bde_verify::csabase::Analyser& analyser,
           clang::SourceLocation    where,
           std::string const&       ,
           std::string const&       name)
{
    if (analyser.is_component_header(name))
    {
        analyser.attachment<status>().line_
            = analyser.get_location(where).line();
    }
}

// ----------------------------------------------------------------------------

static void 
include_file(bde_verify::csabase::Analyser& analyser,
             clang::SourceLocation    where,
             bool                     ,
             std::string const&       name)
{
    ::status& status(analyser.attachment< ::status>());
    if (status.check_)
    {
        if (analyser.is_component_header(name) ||
            analyser.is_component_header(analyser.toplevel()))
        {
            status.header_seen_ = true;
        }
        else if (!status.header_seen_
                 && analyser.toplevel() != name
                 && builtin != name
                 && command_line != name
                 && "bdes_ident.h" != name
                 && !analyser.is_main())
        {
            analyser.report(where, check_name, "TR09",
                            "Include files precede component header",
                            true);
            status.check_ = false;
        }
    }
}

// ----------------------------------------------------------------------------

static void
declaration(bde_verify::csabase::Analyser& analyser, clang::Decl const* decl)
{
    status& status(analyser.attachment< ::status>());
    if (status.check_)
    {
        bde_verify::csabase::Location loc(analyser.get_location(decl));
        if ((analyser.toplevel() != loc.file() && status.header_seen_)
            || (analyser.toplevel() == loc.file()
                && status.line_ < loc.line()))
        {
            status.check_ = false;
        }
        else if (((analyser.toplevel() != loc.file() && !status.header_seen_)
                  || loc.line() < status.line_)
                 && builtin != loc.file() && command_line != loc.file()
                 && (llvm::dyn_cast<clang::NamedDecl>(decl) == 0
                     || utils::end(id_names)
                          == std::find(utils::begin(id_names),
                                       utils::end(id_names),
                                       llvm::dyn_cast<clang::NamedDecl>(decl)
                                                          ->getNameAsString()))
                 && !analyser.is_main())
        {
            analyser.report(decl, check_name, "TR09",
                            "Declarations precede component header",
                            true);
            status.check_ = false;
        }
    }
}

// ----------------------------------------------------------------------------

namespace
{
    template <typename T>
    struct analyser_binder
    {
        analyser_binder(void (*function)(bde_verify::csabase::Analyser&,
                                         clang::SourceLocation,
                                         T,
                                         std::string const&),
                        bde_verify::csabase::Analyser& analyser):
            function_(function),
            analyser_(analyser)
        {
        }
        void operator()(clang::SourceLocation where,
                        T                     arg,
                        std::string const&    name) const
        {
            function_(analyser_, where, arg, name);
        }
        void          (*function_)(bde_verify::csabase::Analyser&,
                                   clang::SourceLocation,
                                   T,
                                   std::string const&);
        bde_verify::csabase::Analyser& analyser_;
    };
}

// -----------------------------------------------------------------------------

static void
subscribe(bde_verify::csabase::Analyser&   analyser,
          bde_verify::csabase::Visitor&    ,
          bde_verify::csabase::PPObserver& observer)
{
    observer.onInclude   += analyser_binder<bool>(include_file, analyser);
    observer.onCloseFile += analyser_binder<std::string const&>(close_file,
                                                                  analyser);
}

// -----------------------------------------------------------------------------

static bde_verify::csabase::RegisterCheck register_observer(check_name,
                                                      &subscribe);
static bde_verify::csabase::RegisterCheck register_check(check_name, &declaration);
