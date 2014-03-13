// csamisc_contiguousswitch.t.hpp                                     -*-C++-*-
// -----------------------------------------------------------------------------
// Copyright 2012 Dietmar Kuehl http://www.dietmar-kuehl.de              
// Distributed under the Boost Software License, Version 1.0. (See file  
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt).     
// -----------------------------------------------------------------------------

#if !defined(INCLUDED_CSAMISC_CONTIGUOUSSWITCH)
#define INCLUDED_CSAMISC_CONTIGUOUSSWITCH 1
#if !defined(INCLUDED_BDES_IDENT)
#  include <bdes_ident.h>
#endif
#ifndef INCLUDED_CSASCM_VERSION
#  include <csascm_version.h>
#endif

// -----------------------------------------------------------------------------

namespace bde_verify
{
    namespace csamisc
    {
        struct contiguousswitch
        {
            static int f(int arg);
        };
    }
}

// -----------------------------------------------------------------------------

#endif
