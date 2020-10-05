/*//////////////////////////////////////////////////////////////////////////////
    Copyright (c) 2015-2020 Jamboree

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//////////////////////////////////////////////////////////////////////////////*/
#ifndef NIJI_VIEW_IDENTITY_HPP_INCLUDED
#define NIJI_VIEW_IDENTITY_HPP_INCLUDED

#include <niji/support/view.hpp>

namespace niji
{
    struct identity_view : view<identity_view>
    {
        template<class Path, class Sink>
        static void iterate(Path const& path, Sink& sink)
        {
             niji::iterate(path, sink);
        }
                
        template<class Path, class Sink>
        static void reverse_iterate(Path const& path, Sink& sink)
        {
             niji::reverse_iterate(path, sink);
        }
    };
}

namespace niji::views
{
    constexpr identity_view identity{};
}

#endif