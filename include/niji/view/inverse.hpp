/*//////////////////////////////////////////////////////////////////////////////
    Copyright (c) 2015 Jamboree

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//////////////////////////////////////////////////////////////////////////////*/
#ifndef NIJI_VIEW_INVERSE_HPP_INCLUDED
#define NIJI_VIEW_INVERSE_HPP_INCLUDED

#include <niji/support/view.hpp>
#include <niji/support/identifier.hpp>

namespace niji
{
    struct inverse_view : view<inverse_view>
    {
        template<class Path, class Sink>
        static void render(Path const& path, Sink& sink)
        {
             niji::inverse_render(path, sink);
        }
                
        template<class Path, class Sink>
        static void inverse_render(Path const& path, Sink& sink)
        {
             niji::render(path, sink);
        }
    };
}

namespace niji { namespace views
{
    NIJI_IDENTIFIER(inverse_view, inverse)
}}

#endif
