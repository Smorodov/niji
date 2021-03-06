/*//////////////////////////////////////////////////////////////////////////////
    Copyright (c) 2015-2019 Jamboree

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//////////////////////////////////////////////////////////////////////////////*/
#ifndef NIJI_SUPPORT_POINT_HPP_INCLUDED
#define NIJI_SUPPORT_POINT_HPP_INCLUDED

#include <type_traits>
#include <boost/geometry/core/access.hpp>
#include <boost/geometry/core/cs.hpp>
#include <boost/geometry/core/coordinate_dimension.hpp>
#include <boost/geometry/core/coordinate_type.hpp>
#include <boost/geometry/core/tags.hpp>
#include <niji/support/traits.hpp>
#include <niji/support/is_narrowing.hpp>
#include <niji/support/convert_geometry.hpp>

namespace niji
{
    template<class T>
    struct point
    {
        using coordinate_type = T;

        T x, y;
        
        point() : x(), y() {}

        point(T x, T y) : x(x), y(y) {}
        
        template<class U, std::enable_if_t<std::is_convertible_v<U, T> && !is_narrowing_v<U, T>, bool> = true>
        point(point<U> const& other) : x(other.x), y(other.y) {}

        template<class U, std::enable_if_t<is_narrowing_v<U, T>, bool> = true>
        explicit point(point<U> const& other) : x(static_cast<T>(other.x)), y(static_cast<T>(other.y)) {}
        
#if 0
        template<class Point, std::enable_if_t<is_point<Point>::value, bool> = true>
        point(Point const& other)
          : x(boost::geometry::get<0>(other)), y(boost::geometry::get<1>(other))
        {}
#endif

        void reset(T x2, T y2)
        {
            x = std::move(x2);
            y = std::move(y2);
        }
        
        template<std::size_t N>
        T const& coord() const
        {
            return *(&x + N);
        }
        
        template<std::size_t N>
        T& coord()
        {
            return *(&x + N);
        }
        
        template<class U>
        bool operator==(point<U> const& other) const
        {
            return x == other.x && y == other.y;
        }
        
        template<class U>
        bool operator!=(point<U> const& other) const
        {
            return !operator==(other);
        }
    
        point operator-() const
        {
            return point(-x, -y);
        }

        template<class U>
        point& operator+=(point<U> const& other)
        {
            x += other.x;
            y += other.y;
            return *this;
        }

        template<class U>
        point& operator-=(point<U> const& other)
        {
            x -= other.x;
            y -= other.y;
            return *this;
        }

        template<class U, std::enable_if_t<std::is_arithmetic<U>::value, bool> = true>
        point& operator*=(U const& val)
        {
            x *= val;
            y *= val;
            return *this;
        }

        template<class U>
        point& operator*=(point<U> const& other)
        {
            x *= other.x;
            y *= other.y;
            return *this;
        }

        template<class U, std::enable_if_t<std::is_arithmetic<U>::value, bool> = true>
        point& operator/=(U const& val)
        {
            x /= val;
            y /= val;
            return *this;
        }
        
        template<class U>
        point& operator/=(point<U> const& other)
        {
            x /= other.x;
            y /= other.y;
            return *this;
        }

        template<class Archive>
        void serialize(Archive& ar, unsigned version)
        {
            ar & x & y;
        }
    };

#define NIJI_DEFINE_POINT_OEPRATOR(op)                                          \
    template<class T, class U, std::enable_if_t<std::is_arithmetic<U>::value, bool> = true>\
    inline point<std::common_type_t<T, U>> operator op(point<T> const& pt, U const& val)\
    {                                                                           \
        return point<std::common_type_t<T, U>>(pt.x op val, pt.y op val);       \
    }                                                                           \
    template<class T, class U, std::enable_if_t<std::is_arithmetic<U>::value, bool> = true>\
    inline point<std::common_type_t<T, U>> operator op(U const& val, point<T> const& pt)\
    {                                                                           \
        return point<std::common_type_t<T, U>>(val op pt.x, val op pt.y);       \
    }                                                                           \
    template<class T, class U>                                                  \
    inline point<std::common_type_t<T, U>> operator op(point<T> const& lhs, point<U> const& rhs)\
    {                                                                           \
        return point<std::common_type_t<T, U>>(lhs.x op rhs.x, lhs.y op rhs.y); \
    }                                                                           \
/***/
    NIJI_DEFINE_POINT_OEPRATOR(+)
    NIJI_DEFINE_POINT_OEPRATOR(-)
    NIJI_DEFINE_POINT_OEPRATOR(*)
    NIJI_DEFINE_POINT_OEPRATOR(/)
#undef NIJI_DEFINE_POINT_OEPRATOR

    using ipoint = point<int>;
    using fpoint = point<float>;
    using dpoint = point<double>;
}

namespace niji::points
{
    template<class T, class F>
    inline point<T> transform(point<T> const& a, point<T> const& b, F&& f)
    {
        return {f(a.x, b.x), f(a.y, b.y)};
    }
    
    template<class T>
    inline point<T> middle(point<T> const& a, point<T> const& b)
    {
        return transform(a, b, [](T a, T b) { return (a + b) / 2; });
    }
    
    template<class T>
    inline point<T> interpolate(point<T> const& a, point<T> const& b, T t)
    {
        return transform(a, b, [t](T a, T b) { return a + (b - a) * t; });
    }
}

namespace boost::geometry::traits
{
    template<class T>
    struct tag<niji::point<T>>
    {
        using type = point_tag;
    };

    template<class T>
    struct coordinate_type<niji::point<T>>
    {
        using type = T;
    };

    template<class T>
    struct coordinate_system<niji::point<T>>
    {
        using type = cs::cartesian;
    };

    template<class T>
    struct dimension<niji::point<T>>
      : boost::mpl::int_<2>
    {};

    template<class T, std::size_t Dimension>
    struct access<niji::point<T>, Dimension>
    {
        using point_type = niji::point<T>;

        static inline T get(point_type const& p)
        {
            return p.template coord<Dimension>();
        }

        static inline void set(point_type& p, T value)
        {
            p.template coord<Dimension>() = value;
        }
    };
}

#endif