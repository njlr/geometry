// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2014, Oracle and/or its affiliates.

// Licensed under the Boost Software License version 1.0.
// http://www.boost.org/users/license.html

// Contributed and/or modified by Menelaos Karavelas, on behalf of Oracle

#ifndef BOOST_GEOMETRY_TEST_SET_OPS_COMMON_HPP
#define BOOST_GEOMETRY_TEST_SET_OPS_COMMON_HPP


#include <boost/range.hpp>
#include <boost/geometry/policies/compare.hpp>
#include <boost/geometry/algorithms/equals.hpp>
#include <boost/geometry/algorithms/reverse.hpp>
#include <boost/geometry/multi/algorithms/reverse.hpp>



namespace bg = ::boost::geometry;



template <typename Linestring1, typename Linestring2>
struct ls_less
{
    typedef typename boost::range_iterator<Linestring1 const>::type Iterator1;
    typedef typename boost::range_iterator<Linestring2 const>::type Iterator2;

    typedef bg::less<typename bg::point_type<Linestring1>::type> point_less;

    bool operator()(Linestring1 const& linestring1,
                    Linestring2 const& linestring2) const
    {
        if ( boost::size(linestring1) != boost::size(linestring2) )
            return boost::size(linestring1) < boost::size(linestring2);

        Iterator1 it1 = boost::begin(linestring1);
        Iterator2 it2 = boost::begin(linestring2);
        point_less less;
        for (; it1 != boost::end(linestring1); ++it1, ++it2)
        {
            if ( less(*it1, *it2) )
                return true;
            if ( less(*it2, *it1) )
                return false;
        }
        return false;
    }
};




template <typename MultiLinestring1, typename MultiLinestring2>
struct multilinestring_equals
{
    static inline
    bool apply(MultiLinestring1 const& multilinestring1,
               MultiLinestring2 const& multilinestring2)
    {
        MultiLinestring1 mls1 = multilinestring1;
        MultiLinestring2 mls2 = multilinestring2;
        if ( boost::size(mls1) != boost::size(mls2) )
        {
            return false;
        }

        typedef typename boost::range_iterator
            <
                MultiLinestring1 const
            >::type ls1_iterator;

        typedef typename boost::range_iterator
            <
                MultiLinestring2 const
            >::type ls2_iterator;

        typedef typename boost::range_value<MultiLinestring1>::type Linestring1;

        typedef typename boost::range_value<MultiLinestring2>::type Linestring2;

        typedef typename boost::range_iterator
            <
                Linestring1 const
            >::type point1_iterator;

        typedef typename boost::range_iterator
            <
                Linestring2 const
            >::type point2_iterator;

        typedef ls_less<Linestring1, Linestring2> linestring_less;

        std::sort(boost::begin(mls1), boost::end(mls1), linestring_less());
        std::sort(boost::begin(mls2), boost::end(mls2), linestring_less());

        ls1_iterator it1 = boost::begin(mls1);
        ls2_iterator it2 = boost::begin(mls2);
        for (; it1 != boost::end(mls1); ++it1, ++it2)
        {
            if ( boost::size(*it1) != boost::size(*it2) )
            {
                return false;
            }
            point1_iterator pit1 = boost::begin(*it1);
            point2_iterator pit2 = boost::begin(*it2);
            for (; pit1 != boost::end(*it1); ++pit1, ++pit2)
            {
                if ( !bg::equals(*pit1, *pit2) )
                {
                    return false;
                }
            }
        }
        return true;
    }
};




class equals
{
private:
    template <typename Linestring, typename OutputIterator>
    static inline OutputIterator
    isolated_point_to_segment(Linestring const& linestring, OutputIterator oit)
    {
        BOOST_ASSERT( boost::size(linestring) == 1 );

        *oit++ = *boost::begin(linestring);
        *oit++ = *boost::begin(linestring);
        return oit;
    }


    template <typename MultiLinestring, typename OutputIterator>
    static inline OutputIterator
    convert_isolated_points_to_segments(MultiLinestring const& multilinestring,
                                        OutputIterator oit)
    {
        BOOST_AUTO_TPL(it, boost::begin(multilinestring));

        for (; it != boost::end(multilinestring); ++it)
        {
            if ( boost::size(*it) == 1 )
            {
                typename boost::range_value<MultiLinestring>::type linestring;
                isolated_point_to_segment(*it, std::back_inserter(linestring));
                *oit++ = linestring;
            }
            else
            {
                *oit++ = *it;
            }
        }
        return oit;
    }


    template <typename MultiLinestring1, typename MultiLinestring2>
    static inline bool apply_base(MultiLinestring1 const& multilinestring1,
                                  MultiLinestring2 const& multilinestring2)
    {
        typedef multilinestring_equals
            <
                MultiLinestring1, MultiLinestring2
            > mls_equals;

        if ( mls_equals::apply(multilinestring1, multilinestring2) )
        {
            return true;
        }

        MultiLinestring1 reverse_multilinestring1 = multilinestring1;
        bg::reverse(reverse_multilinestring1);
        if ( mls_equals::apply(reverse_multilinestring1, multilinestring2) )
        {
            return true;
        }

        MultiLinestring2 reverse_multilinestring2 = multilinestring2;
        bg::reverse(reverse_multilinestring2);
        if ( mls_equals::apply(multilinestring1, reverse_multilinestring2) )
        {
            return true;
        }

        return mls_equals::apply(reverse_multilinestring1,
                                 reverse_multilinestring2);
    }



public:
    template <typename MultiLinestring1, typename MultiLinestring2>
    static inline bool apply(MultiLinestring1 const& multilinestring1,
                             MultiLinestring2 const& multilinestring2)
    {
#ifndef BOOST_GEOMETRY_ALLOW_ONE_POINT_LINESTRINGS
        MultiLinestring1 converted_multilinestring1;
        convert_isolated_points_to_segments
            (multilinestring1, std::back_inserter(converted_multilinestring1));
        MultiLinestring2 converted_multilinestring2;
        convert_isolated_points_to_segments
            (multilinestring2, std::back_inserter(converted_multilinestring2));
        return apply_base(converted_multilinestring1,
                          converted_multilinestring2);
#else
        return apply_base(multilinestring1, multilinestring2);
#endif
    }
};


#endif // BOOST_GEOMETRY_TEST_SET_OPS_COMMON_HPP
