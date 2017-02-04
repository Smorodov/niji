/*//////////////////////////////////////////////////////////////////////////////
    Copyright (c) 2017 Jamboree

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//////////////////////////////////////////////////////////////////////////////*/
#ifndef NIJI_MAKE_GENERATOR_HPP_INCLUDED
#define NIJI_MAKE_GENERATOR_HPP_INCLUDED

#include <type_traits>

namespace niji
{
    template<class Point, class F>
    struct generator_path
    {
        using point_type = Point;
        
        F f;

        // Either one of the 2 overloads should be viable.
        template<class Sink>
        auto render(Sink& sink) const -> decltype(f(sink, std::false_type{}))
        {
            f(sink, std::false_type{});
        }

        template<class Sink>
        auto render(Sink& sink) const -> decltype(f(sink))
        {
            f(sink);
        }

        // May be disabled.
        template<class Sink>
        auto inverse_render(Sink& sink) const -> decltype(f(sink, std::true_type{}))
        {
            f(sink, std::true_type{});
        }
    };
    
    template<class Point, class F>
    inline generator_path<Point, F> make_generator(F&& f)
    {
        return {std::forward<F>(f)};
    }
}

#endif