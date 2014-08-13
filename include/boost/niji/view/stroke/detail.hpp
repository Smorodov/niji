/*//////////////////////////////////////////////////////////////////////////////
    Copyright (c) 2014 Jamboree

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//////////////////////////////////////////////////////////////////////////////*/
#ifndef BOOST_NIJI_VIEW_STROKE_DETAIL_HPP_INCLUDED
#define BOOST_NIJI_VIEW_STROKE_DETAIL_HPP_INCLUDED

#include <boost/niji/path.hpp>
#include <boost/niji/support/command.hpp>
#include <boost/niji/support/point.hpp>
#include <boost/niji/support/vector.hpp>
#include <boost/niji/support/bezier/basics.hpp>

#define BOOST_NIJI_MAX_QUAD_SUBDIVIDE 5

namespace boost { namespace niji { namespace detail
{
    template<class T, class Joiner, class Capper>
    struct stroker
    {
        using point_t = point<T>;
        using vector_t = vector<T>;
        using path_t = path<point_t>;

        Joiner const& _join;
        Capper const& _cap;

        T _r, _pre_magnitude, _first_magnitude;
        int _seg_count;
        path_t _outer, _inner/*, extra*/;
        point_t _prev_pt, _first_pt/*, first_outer_pt*/;
        vector_t _prev_normal, _first_normal;
        bool _prev_is_line;

        stroker(T r, Joiner const& join, Capper const& cap)
          : _join(join), _cap(cap)
          , _r(r), _pre_magnitude(), _first_magnitude()
          , _seg_count(), _prev_is_line()
        {}

        void move_to(point_t const& pt)
        {
            if (_seg_count)
                cut(false);
            _seg_count = 0;
            _first_pt = _prev_pt = pt;
        }

        void line_to(point_t const& pt)
        {
            if (vectors::is_degenerated(pt - _prev_pt))
            {
                _seg_count -= !_seg_count;
                return;
            }
            vector_t normal;
            pre_join(pt, normal, true);
            line_to_stroke(pt, normal);
            post_join(pt, normal);
        }

        void pre_join(point_t const& pt, vector_t& normal, bool curr_is_line)
        {
            using std::sqrt;
            
            normal = vectors::normal_cw(pt - _prev_pt);
            T magnitude = vectors::norm_square(normal);
            normal = normal * _r / sqrt(magnitude);

            if (_seg_count > 0) // we have a previous segment
                _join
                (
                    _outer, _inner, _prev_pt, _prev_normal, normal
                  , _r, _prev_is_line, curr_is_line
                  , std::min(_pre_magnitude, magnitude)
                );
            else
            {
                _first_normal = normal;
                //first_outer_pt = _prev_pt + normal;
                _first_magnitude = magnitude;
#ifdef JAMBOREE
                if (!curr_is_line)
#endif
                {
                    _outer.join(_prev_pt + normal);
                    _inner.join(_prev_pt - normal);
                }
            }
            _prev_is_line = curr_is_line;
            _pre_magnitude = magnitude;
        }

        void post_join(point_t const& pt, vector_t const& normal)
        {
            _prev_pt = pt;
            _prev_normal = normal;
            ++_seg_count;
        }

        void line_to_stroke(point_t const& pt, vector_t const& normal)
        {
#ifndef JAMBOREE
            _outer.join(pt + normal);
            _inner.join(pt - normal);
#endif
        }

        void cut(bool curr_is_line)
        {
            switch (_seg_count)
            {
            case -1:
                degenerated_dot();
            case 0:
                return;
            }
            // _cap the end
            _cap(_outer, _prev_pt, _prev_normal, curr_is_line);
            _outer.reverse_splice(_inner);

            // _cap the start
            _cap(_outer, _first_pt, -_first_normal, _prev_is_line);
            _outer.close();

            _inner.clear();
        }

        void close(bool curr_is_line)
        {
            if (_seg_count < 2)
            {
                if (_seg_count == -1)
                    degenerated_dot();
                return;
            }

            line_to(_first_pt);
            _join
            (
                _outer, _inner, _prev_pt, _prev_normal, _first_normal
              , _r, _prev_is_line, curr_is_line
              , std::min(_pre_magnitude, _first_magnitude)
            );
            _outer.close();
            _inner.close();
            _outer.reverse_splice(_inner);
            _outer.close();

            _inner.clear();
        }

        void quad_to(point_t const& pt1, point_t const& pt2)
        {
            bool degenerateAB = vectors::is_degenerated(pt1 - _prev_pt);
            bool degenerateBC = vectors::is_degenerated(pt2 - pt1);

            if (degenerateAB | degenerateBC)
            {
                if (degenerateAB ^ degenerateBC)
                    line_to(pt2);
                return;
            }
            vector_t normalAB, normalBC;
            pre_join(pt1, normalAB, false);
            {
                point_t pts[3] = {_prev_pt, pt1, pt2}, tmp[5];
                if (bezier::chop_quad_at_max_curvature(pts, tmp))
                {
                    normalBC = vectors::normal_cw(pts[2] - pts[1]);
                    normalBC = normalBC * _r / vectors::norm(normalBC);
                    if (vectors::too_pinchy(normalAB, normalBC))
                    {
                        _outer.join(tmp[2] + normalAB);
                        _outer.join(tmp[2] + normalBC);
                        _outer.join(tmp[4] + normalBC);
                        
                        _inner.join(tmp[2] - normalAB);
                        _inner.join(tmp[2] - normalBC);
                        _inner.join(tmp[4] - normalBC);
                        
                        // Add circle???
                    }
                    else
                    {
                        quad_to_stroke(tmp, normalAB, normalBC, BOOST_NIJI_MAX_QUAD_SUBDIVIDE);
                        vector_t normal(normalBC);
                        quad_to_stroke(tmp + 2, normal, normalBC, BOOST_NIJI_MAX_QUAD_SUBDIVIDE);
                    }
                }
                else
                    quad_to_stroke(pts, normalAB, normalBC, BOOST_NIJI_MAX_QUAD_SUBDIVIDE);
            }
            post_join(pt2, normalBC);
        }
        
        void quad_to_stroke(point_t const pts[3], vector_t const& normalAB, vector_t& normalBC, int subdivide)
        {
            using std::sqrt;
            
            normalBC = vectors::normal_cw(pts[2] - pts[1]);
            if (vectors::is_degenerated(normalBC))
            {
                line_to_stroke(pts[2], normalAB);
                normalBC = normalAB;
                return;
            }
            if (subdivide-- && vectors::too_curvy(normalAB, normalBC))
            {
                point_t tmp[5];
                vector_t normal;
                bezier::chop_quad_at_half(pts, tmp);
                quad_to_stroke(tmp, normalAB, normal, subdivide);
                quad_to_stroke(tmp + 2, normal, normalBC, subdivide);
            }
            else
            {
                normalBC = normalBC * _r / vectors::norm(normalBC);
                vector_t normal(vectors::normal_cw(pts[2] - pts[0]));
                T dot = vectors::dot(normalAB, normalBC);
                T ns = vectors::norm_square(normal);
                T s = sqrt(vectors::norm_square(normalAB) * vectors::norm_square(normalBC));
                normal = normal * _r / sqrt((dot + s) * ns / (2 * s));
                _outer.unsafe_quad_to(pts[1] + normal, pts[2] + normalBC);
                _inner.unsafe_quad_to(pts[1] - normal, pts[2] - normalBC);
            }
        }
        
        void degenerated_dot()
        {
            _cap(_outer, _prev_pt, vector_t(_r, 0), true);
            _cap(_outer, _prev_pt, vector_t(-_r, 0), true);
        }
        
        template<class Sink>
        void finish(Sink& sink, bool reversed)
        {
            if (reversed)
                _outer.reverse_iterate(sink);
            else
                _outer.iterate(sink);
            _outer.clear();
            _seg_count = 0;
        }
    };
}}}

#endif
