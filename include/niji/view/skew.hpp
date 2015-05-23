/*//////////////////////////////////////////////////////////////////////////////
    Copyright (c) 2015 Jamboree

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//////////////////////////////////////////////////////////////////////////////*/
#ifndef NIJI_VIEW_SKEW_HPP_INCLUDED
#define NIJI_VIEW_SKEW_HPP_INCLUDED

#include <niji/view/transform.hpp>
#include <niji/support/transform/skew.hpp>

namespace niji { namespace views
{
    using transforms::skew;
}}

namespace niji { namespace transforms
{
    template<class Path, class T>
    inline auto operator|(Path&& path, skew<T>&& d)
    {
        return std::forward<Path>(path) | views::transform(std::move(d));
    }
    
    template<class Path, class T>
    inline auto operator|(Path&& path, skew<T> const& d)
    {
        return std::forward<Path>(path) | views::transform(d);
    }
}}

#endif