/*//////////////////////////////////////////////////////////////////////////////
    Copyright (c) 2014 Jamboree

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//////////////////////////////////////////////////////////////////////////////*/
#ifndef BOOST_NIJI_TRANSFORM_AFFINE_HPP_INCLUDED
#define BOOST_NIJI_TRANSFORM_AFFINE_HPP_INCLUDED

#include <boost/type_traits/decay.hpp>
#include <boost/niji/support/traits.hpp>
#include <boost/niji/support/point.hpp>

namespace boost { namespace niji { namespace transforms
{
    template<class Base>
    struct affine_inverse
    {
        using result_type = typename decay<Base>::type::result_type;
        
        affine_inverse(Base base)
          : base(base)
        {}
        
        template<class Point>
        result_type operator()(Point const& pt) const
        {
            using geometry::get;

            auto x = get<0>(pt), y = get<1>(pt)
               , d = base.determinant()
               , a = (x - base.tx) / d
               , b = (y - base.ty) / d;
                  
            return {a * base.sy - b * base.shx, b * base.sx - a * base.shy};
        }

        Base base;
    };
    
    template<class T>
    struct affine
    {
        using result_type = point<T>;

        T sx, shx, tx, shy, sy, ty;

        affine()
          : sx(1), shx(), tx()
          , shy(), sy(1), ty()
        {}

        void set_sin_cos(T s, T c)
        {
            sx  = c;
            shx = -s;
            shy = s;
            sy  = c;
        }
        
        affine& translate(T x, T y)
        {
            tx += x;
            ty += y;
            return *this;
        }

        affine& pre_translate(T x, T y)
        {
            tx += sx * x + shx * y;
            ty += shy * x + sy * y;
            return *this;
        }

        affine& scale(T s)
        {
            return scale(s, s);
        }

        affine& scale(T x, T y)
        {
            sx  *= x;
            shx *= x;
            tx  *= x;
            sy  *= y;
            shy *= y;
            ty  *= y;
            return *this;
        }

        affine& pre_scale(T s)
        {
            return pre_scale(s, s);
        }

        affine& pre_scale(T x, T y)
        {
            sx  *= x;
            shx *= y;
            sy  *= y;
            shy *= x;
            return *this;
        }

        affine& skew(T x, T y)
        {
            T sx0 = sx, shx0 = shx, tx0 = tx;

            sx  += x * shy;
            shx += x * sy;
            tx  += x * ty;
            sy  += y * shx0;
            shy += y * sx0;
            ty  += y * tx0;
            return *this;
        }

        affine& pre_skew(T x, T y)
        {
            T sx0 = sx, sy0 = sy;

            sx  += y * shx;
            shx += x * sx0;
            sy  += x * shy;
            shy += y * sy0;
            return *this;
        }

        affine& rotate(T a)
        {
            using std::sin;
            using std::cos;

            return rotate(sin(a), cos(a));
        }

        affine& rotate(T sa, T ca)
        {
            T sx0 = sx, shx0 = shx, tx0 = tx;

            sx  = sx  * ca - shy * sa;
            shx = shx * ca - sy * sa;
            tx  = tx  * ca - ty * sa;
            sy  = shx0 * sa + sy * ca;
            shy = sx0  * sa + shy * ca;
            ty  = tx0  * sa + ty * ca;
            return *this;
        }

        affine& pre_rotate(T a)
        {
            using std::sin;
            using std::cos;

            return pre_rotate(sin(a), cos(a));
        }

        affine& pre_rotate(T sa, T ca)
        {
            T sx0 = sx, sy0 = sy;
            
            sx  = sx  * ca + shx * sa;
            shx = shx * ca - sx0 * sa;
            sy  = sy * ca - shy * sa;
            shy = shy * ca + sy0  * sa;
            return *this;
        }

        affine& flip_x()
        {
            sx  = -sx;
            shx = -shx;
            tx  = -tx;
            return *this;
        }
        
        affine& pre_flip_x()
        {
            sx  = -sx;
            shy = -shy;
            return *this;
        }

        affine& flip_y()
        {
            sy  = -sy;
            shy = -shy;
            ty  = -ty;
            return *this;
        }
        
        affine& pre_flip_y()
        {
            sy  = -sy;
            shx = -shx;
            return *this;
        }
        
        affine& invert()
        {
            T d  = determinant();
            T ty0 = ty, sy0 = sy;

            sy  =  sx  / d;
            shy = -shy / d;
            shx = -shx / d;
            sx  =  sy0 / d;
            
            ty  = -tx * shy - ty * sy;
            tx  = -tx * sx  - ty0 * shx;
            return *this;
        }
                
        affine& reset()
        {
            sx  = 1;
            shx = 0;
            tx  = 0;
            sy  = 1;
            shy = 0;
            ty  = 0;
            return *this;
        }

        affine& append(affine const& m)
        {
            T sx0 = sx, shx0 = shx, tx0 = tx;

            sx = sx * m.sx + shy * m.shx;
            shx = shx * m.sx + sy * m.shx;
            shy = sx0 * m.shy + shy * m.sy;
            sy = shx0 * m.shy + sy * m.sy;
            tx = tx * m.sx + ty * m.shx + m.tx;
            ty = tx0 * m.shy + ty * m.sy + m.ty;
            return *this;
        }

        affine& prepend(affine const& m)
        {
            T shy0 = shy;

            tx = m.tx * sx + m.ty * shx + tx;
            ty = m.tx * shy + m.ty * sy + ty;
            sx = m.sx * sx + m.shy * shx;
            shx = (m.shx + m.sy) * shx;
            shy = m.sx * shy + m.shy * sy;
            sy = m.shx * shy0 + m.sy * sy;
            return *this;
        }
        
        T determinant() const
        {
            return sx * sy - shy * shx;
        }

        affine_inverse<affine const&> operator~() const&
        {
            return *this;
        }
        
        affine_inverse<affine> operator~() &&
        {
            return std::move(*this);
        }

        template<class Point>
        point<T> operator()(Point const& pt) const
        {
            using geometry::get;

            auto x = get<0>(pt), y = get<1>(pt);
            return {sx * x + shx * y + tx, sy * y + shy * x + ty};
        }
    };
}}}

#endif
