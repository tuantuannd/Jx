//+---------------------------------------------------------------------------
//
//  Copyright ( C ) Microsoft Corporation, 1994 - 2002.
//
//  File:       regexpr2.h
//
//  Contents:   classes for regular expression pattern matching a-la perl
//
//  Classes:    basic_rpattern_base
//
//  Functions:  rpattern::match
//              rpattern::substitute
//              match_results::cbackrefs
//              match_results::backref
//              match_results::all_backrefs
//              match_results::backref_str
//
//  Author:     Eric Niebler ( ericne@microsoft.com )
//
//----------------------------------------------------------------------------

#ifndef REGEXPR_H
#define REGEXPR_H

#ifdef _MSC_VER
  // warning C4097: typedef-name '' used as synonym for class-name
  // warning C4189: local variable is initialized but not referenced
  // warning C4514: '' : unreferenced inline function has been removed
  // warning C4702: unreachable code
  // warning C4710: function 'blah' not inlined
  // warning C4786: identifier was truncated to '255' characters in the debug information
# pragma warning( push )
# pragma warning( disable : 4097 4189 4514 4702 4710 4786 )
  extern "C" unsigned long __cdecl _exception_code();
# define REGEX_STACK_OVERFLOW 0xC00000FDL
# if _MSC_VER < 1300
   extern "C" int __cdecl _resetstkoflw(void);
# else
#  include <malloc.h> // for _resetstkoflw
# endif
#endif

#include <list>
#include <iosfwd>
#include <string>
#include <vector>
#include "syntax2.h"
#include "restack.h"

namespace regex
{

// This is the default alignment for the unsafe heterogeneous stack.
// If you are getting a compiler error in one of the unsafe_stack
// methods, then compile with -DREGEX_STACK_ALIGNMENT=16 or 32
#ifndef REGEX_STACK_ALIGNMENT
# define REGEX_STACK_ALIGNMENT sizeof( void* )
#endif

#ifndef REGEX_DEBUG
# if defined(DEBUG) | defined(_DEBUG) | defined(DBG)
#  define REGEX_DEBUG 1
# else
#  define REGEX_DEBUG 0
# endif
#endif

namespace detail
{
#if REGEX_DEBUG
    typedef hetero_stack<REGEX_STACK_ALIGNMENT,true,true,32,0>        unsafe_stack;
#else
    typedef hetero_stack<REGEX_STACK_ALIGNMENT,false,true,4096,1024>  unsafe_stack;
#endif

    // Used to initialize variables with the same value they would have
    // if they were initialized as a static global. ( Ptrs get NULL,
    // integer types get 0, etc, etc )
    template<typename T> struct static_init { static T const value; };
    template<typename T> T const static_init<T>::value = T();

    //
    // Forward declarations
    //
    template< typename CI > class  sub_expr;
    template< typename CI > class  match_group_base;
    template< typename CI > class  basic_rpattern_base_impl;
    template< typename CI > struct match_param;
    template< typename CI > struct sub_expr_base;
    template< typename CI > struct matcher_helper;

} // namespace detail

// --------------------------------------------------------------------------
//
// Class:       width_type
//
// Description: represents the width of a sub-expression
//
// Members:     m_min      - smallest number of characters a sub-expr can span
//              m_max      - largest number of characters a sub-expr can span
//
// History:     8/14/2000 - ericne - Created
//
// --------------------------------------------------------------------------
struct width_type
{
    size_t m_min;
    size_t m_max;
};

inline width_type const uninit_width()
{
    width_type const width = { size_t( -1 ), size_t( -1 ) };
    return width;
}

// Helper function for processing escape sequences
template< typename CH, typename TR, typename AL >
void process_escapes( std::basic_string<CH, TR, AL> & str, bool fPattern = false ); //throw()

// --------------------------------------------------------------------------
//
// Class:       backref_tag
//
// Description: Struct which contains a back-reference.  It is a template
//              on the iterator type.
//
// Methods:     backref_tag   - c'tor
//              operator bool - so that if( br ) is true if this br matched
//              operator!     - inverse of operator bool()
//
// Members:     reserved      - move along, nothing to see here
//
// History:     8/9/2001 - ericne - Created
//
// --------------------------------------------------------------------------
template< typename CI >
class backref_tag : public std::pair<CI, CI>
{
    struct detail_t { detail_t * d; };

    template< typename OSTREAM >
    void REGEX_CDECL _do_print( OSTREAM & sout, ... ) const
    {
        typedef typename OSTREAM::char_type CH;
        typedef typename OSTREAM::traits_type TR;
        std::ostreambuf_iterator<CH, TR> iout( sout );
        for( CI iter = first; iter != second; ++iter, ++iout )
            *iout = *iter;
    }

    template< typename OSTREAM >
    void _do_print( OSTREAM & sout, typename OSTREAM::char_type const * ) const
    {
        sout.write( first, static_cast<std::streamsize>( std::distance( first, second ) ) );
    }


public:
    typedef CI iterator_type;
    typedef typename std::iterator_traits<CI>::value_type char_type;
    typedef std::basic_string<char_type> string_type;

    explicit backref_tag(
        CI i1 = detail::static_init<CI>::value,
        CI i2 = detail::static_init<CI>::value )
      : std::pair<CI, CI>( i1, i2 ),
        matched( false ), 
        reserved1( i1 ), 
        reserved2( 0 ), 
        reserved3( false ),
        reserved4( detail::static_init<CI>::value ), 
        reserved5( detail::static_init<CI>::value ) 
    {
    }
    
    CI begin() const
    {
        return first;
    }
    
    CI end() const
    {
        return second;
    }

    string_type const str() const
    {
        return matched ? string_type( first, second ) : string_type();
    }
    
    // Use the "safe bool" idiom. This allows implicit conversion to bool,
    // but not to int. It also disallows conversion to void*.
    typedef detail_t * detail_t::* bool_type;

    operator bool_type() const //throw()
    {
        return matched ? &detail_t::d : 0;
    }

    bool operator!() const //throw()
    {
        return ! matched;
    }
    
    template< typename CH, typename TR >
    std::basic_ostream<CH, TR> & print( std::basic_ostream<CH, TR> & sout ) const
    {
        _do_print( sout, CI() );
        return sout;
    }
    
    bool   matched;

    CI     reserved1; // used for internal book-keeping
    size_t reserved2; // used for internal book-keeping
    bool   reserved3; // used for internal book-keeping
    CI     reserved4; // used for internal book-keeping
    CI     reserved5; // used for internal book-keeping
};

// --------------------------------------------------------------------------
//
// Class:       basic_match_results
//
// Description: Use this structure for returning match/substitute results
//              out from the match()/substitute() methods.
//
// Methods:     cbackrefs      -
//              backref        -
//              all_backrefs   -
//              rlength        -
//
// Members:     m_rgbackrefs   -
//
// Typedefs:    const_iterator -
//              backref_type   -
//              backref_vector -
//
// History:     8/8/2001 - ericne - Created
//
// --------------------------------------------------------------------------
template< typename CI >
struct basic_match_results
{
    typedef CI                            const_iterator;
    typedef backref_tag< const_iterator > backref_type;
    typedef std::vector< backref_type >   backref_vector;
    friend struct detail::matcher_helper<CI>;

    virtual ~basic_match_results()
    {
    }

    size_t cbackrefs() const //throw()
    {
        return m_rgbackrefs.size();
    }

    backref_type const & backref( size_t cbackref ) const //throw( std::out_of_range )
    {
        return m_rgbackrefs.at( cbackref );
    }

    backref_vector const & all_backrefs() const //throw()
    {
        return m_rgbackrefs;
    }

    size_t rstart( size_t cbackref = 0 ) const //throw( std::out_of_range )
    {
        return std::distance( ibegin, m_rgbackrefs.at( cbackref ).first );
    }

    size_t rlength( size_t cbackref = 0 ) const //throw( std::out_of_range )
    {
        return std::distance( m_rgbackrefs.at( cbackref ).first, m_rgbackrefs.at( cbackref ).second );
    }

private:
    backref_vector m_rgbackrefs;
    CI ibegin;
};

template< typename CH >
struct basic_match_results_c : public basic_match_results<CH const *>
{
    typedef basic_match_results<CH const *> base;
    typedef typename base::const_iterator const_iterator;
    typedef typename base::backref_type   backref_type;
    typedef typename base::backref_vector backref_vector;
};

template< typename CH, typename TR = std::char_traits<CH>, typename AL = std::allocator<CH> >
struct basic_subst_results : public basic_match_results<typename std::basic_string<CH, TR, AL>::const_iterator>
{
    typedef basic_match_results<typename std::basic_string<CH, TR, AL>::const_iterator> base;
    typedef typename base::const_iterator const_iterator;
    typedef typename base::backref_type   backref_type;
    typedef typename base::backref_vector backref_vector;
    typedef std::basic_string<CH, TR, AL> string_type;
    friend struct detail::matcher_helper<const_iterator>;

    basic_subst_results() 
      : m_pbackref_str( &m_backref_str )
    {
    }

    string_type const & backref_str() const //throw()
    {
        return *m_pbackref_str;
    }

private:
    string_type         m_backref_str;
    string_type const * m_pbackref_str;
};

// The REGEX_MODE is a way of controlling how matching occurs.
enum REGEX_MODE
{
    MODE_FAST, // Uses the fast, recursive algorithm. Could overflow stack.
    MODE_SAFE, // Uses the slow, iterative algorithm. Can't overflow stack.
    MODE_MIXED, // Uses a heuristic to automatically determine which algorithm
                // is the most appropriate for this pattern.

    // MS VC++ has structured exception handling, which makes the
    // consequences of a stack overflow much less severe. Because of this,
    // it is possible to use the "fast" algorithm always on MS platforms,
#ifdef _MSC_VER
    MODE_DEFAULT = MODE_FAST
#else
    MODE_DEFAULT = MODE_MIXED
#endif
};

template< typename CH >
void reset_intrinsic_charsets( CH ch = CH( 0 ) );

// This is for implementation details that really belong in the
// cpp file, but can't go there because of template strangeness.
#include "reimpl2.h"

// --------------------------------------------------------------------------
//
// Class:       basic_rpattern_base
//
// Description:
//
// Methods:     basic_rpattern_base - c'tor
//              basic_rpattern_base -
//              basic_rpattern_base -
//              init                - ( re )initialize the pattern
//              init                -
//              set_substitution    - set the substitution string
//              _find_next_group    - parse the next group of the pattern
//              _find_next          - parse the next sub_expr of the pattern
//              _find_atom          - parse the next atom of the pattern
//              _quantify           - quantify the sub_expr
//              _common_init        - perform some common initialization tasks
//              _parse_subst        - parse the substitution string
//              _add_subst_backref  - add a backref node to the subst list
//
// Members:     m_invisible_groups  - list of hidden groups
//
// Typedefs:    syntax_type         -
//              backref_type        -
//              backref_vector      -
//              string_type         -
//              size_type           -
//
// History:     8/14/2000 - ericne - Created
//              8/5/2001 - ericne - complete overhaul
//
// --------------------------------------------------------------------------
template< typename CI, typename SY >
class basic_rpattern_base : protected detail::basic_rpattern_base_impl<CI>
{
    typedef detail::basic_rpattern_base_impl<CI> impl;
public:
    typedef SY syntax_type;
    typedef typename detail::basic_rpattern_base_impl<CI>::char_type       char_type;
    typedef typename detail::basic_rpattern_base_impl<CI>::traits_type     traits_type;

    typedef typename detail::basic_rpattern_base_impl<CI>::string_type     string_type;
    typedef typename detail::basic_rpattern_base_impl<CI>::size_type       size_type;

    typedef typename detail::basic_rpattern_base_impl<CI>::backref_type    backref_type;
    typedef typename detail::basic_rpattern_base_impl<CI>::backref_vector  backref_vector;

    // Work-around for an apparant bug in gcc
    struct iter_wrap
    {
        typename string_type::iterator & ipat;
        iter_wrap( typename string_type::iterator & i ) : ipat(i) {}
    private:
        iter_wrap( iter_wrap const & );
        iter_wrap & operator=( iter_wrap const & );
    };

    void init(
        string_type const & pat,
        REGEX_FLAGS flags = NOFLAGS,
        REGEX_MODE mode = MODE_DEFAULT ); //throw( bad_regexpr, std::bad_alloc );

    void init(
        string_type const & pat,
        string_type const & subst,
        REGEX_FLAGS flags = NOFLAGS,
        REGEX_MODE mode = MODE_DEFAULT ); //throw( bad_regexpr, std::bad_alloc );

    void set_substitution(
        string_type const & subst ); //throw( bad_regexpr, std::bad_alloc );

    REGEX_FLAGS flags() const                       { return impl::flags(); }
    REGEX_MODE mode() const                         { return impl::mode(); }
    width_type get_width() const                    { return impl::get_width(); }
    size_t cgroups() const                          { return impl::cgroups(); }
    string_type const & get_pat() const             { return impl::get_pat(); }
    string_type const & get_subst() const           { return impl::get_subst(); }
    void swap( basic_rpattern_base<CI, SY> & that ) { impl::swap( that ); }

    static size_t const npos;

protected:
    basic_rpattern_base() //throw()
        : detail::basic_rpattern_base_impl<CI>()
    {
    }

    basic_rpattern_base(
        basic_rpattern_base<CI, SY> const & that ) //throw()
      : detail::basic_rpattern_base_impl<CI>( that.flags(), that.mode(), that.get_pat(), that.get_subst() )
    {
        // Don't call _normalize_string(). If that.flags()&NORMALIZE,
        // then subst has already been normalized.
        basic_rpattern_base<CI, SY>::_common_init( m_flags );
        basic_rpattern_base<CI, SY>::_parse_subst( *m_subst, m_fuses_backrefs, m_subst_list ); // must come after _common_init
    }

    explicit basic_rpattern_base(
        string_type const & pat, 
        REGEX_FLAGS flags = NOFLAGS, 
        REGEX_MODE mode = MODE_DEFAULT ) //throw( bad_regexpr, std::bad_alloc )
      : detail::basic_rpattern_base_impl<CI>( flags, mode, pat )
    {
        basic_rpattern_base<CI, SY>::_common_init( m_flags );
    }

    basic_rpattern_base( 
        string_type const & pat, 
        string_type const & subst, 
        REGEX_FLAGS flags = NOFLAGS, 
        REGEX_MODE mode = MODE_DEFAULT ) //throw( bad_regexpr, std::bad_alloc )
      : detail::basic_rpattern_base_impl<CI>( flags, mode, pat, subst )
    {
        basic_rpattern_base<CI, SY>::_common_init( m_flags );
        _normalize_string( *m_subst );
        basic_rpattern_base<CI, SY>::_parse_subst( *m_subst, m_fuses_backrefs, m_subst_list ); // must come after _common_init
    }

    basic_rpattern_base & operator=(
        basic_rpattern_base<CI, SY> const & that ) //throw( bad_regexpr, std::bad_alloc )
    {
        basic_rpattern_base<CI, SY> temp( that );
        swap( temp );
        return *this;
    }

    detail::match_group_base<CI> * _find_next_group(
        iter_wrap & iw,
        detail::match_group_base<CI> * pgroup, syntax_type & sy,
        std::vector<detail::match_group_base<CI>*> & rggroups );

    bool _find_next(
        iter_wrap & iw,
        detail::match_group_base<CI> * pgroup, syntax_type & sy,
        std::vector<detail::match_group_base<CI>*> & rggroups );

    void _find_atom(
        iter_wrap & iw,
        detail::match_group_base<CI> * pgroup,
        syntax_type & sy );

    void _quantify(
        std::auto_ptr<detail::sub_expr<CI> > & pnew,
        iter_wrap & iw,
        bool is_group,
        syntax_type & sy );

    void _add_subst_backref(
        detail::subst_node & snode, 
        size_t nbackref, 
        size_t rstart,
        bool & uses_backrefs,
        detail::subst_list_type & subst_list ) const;

    virtual void _parse_subst(
        string_type & subst,
        bool & uses_backrefs,
        detail::subst_list_type & subst_list ) const;

    virtual void _common_init( REGEX_FLAGS flags );
};

template< typename CI, typename SY >
size_t const basic_rpattern_base<CI, SY>::npos = size_t( -1 );

// VC6 is buggy in its handling of the typename keyword
#if defined(_MSC_VER) & _MSC_VER < 1300
# define REGEX_TYPENAME
#else
# define REGEX_TYPENAME typename
#endif

// --------------------------------------------------------------------------
//
// Class:       basic_rpattern
//
// Description: generic regex pattern object
//
// Methods:     basic_rpattern - c'tor
//              basic_rpattern -
//              basic_rpattern -
//              match          - match from begin iter to end iter
//              match          - match a null-terminated string
//              match          - match a std::string
//              count          - count matches from begin iter to end iter
//              count          - count matches in a null-terminated string
//              count          - count matches in a std::string
//              substitute     - do substitutions in a std::string
//              _do_match      - internal implementation
//              _do_count      - internal implementation
//
// History:     8/13/2001 - ericne - Created
//
// --------------------------------------------------------------------------
template< typename CI, typename SY = perl_syntax<REGEX_TYPENAME std::iterator_traits<CI>::value_type> >
class basic_rpattern : public basic_rpattern_base<CI, SY>
{
    typedef detail::basic_rpattern_base_impl<CI> impl;
public:
    typedef typename basic_rpattern_base<CI, SY>::syntax_type     syntax_type;
    typedef typename basic_rpattern_base<CI, SY>::char_type       char_type;
    typedef typename basic_rpattern_base<CI, SY>::traits_type     traits_type;

    typedef typename basic_rpattern_base<CI, SY>::string_type     string_type;
    typedef typename basic_rpattern_base<CI, SY>::size_type       size_type;

    typedef typename basic_rpattern_base<CI, SY>::backref_type    backref_type;
    typedef typename basic_rpattern_base<CI, SY>::backref_vector  backref_vector;

    basic_rpattern() //throw()
      : basic_rpattern_base<CI, SY>()
    {
    }

    basic_rpattern( basic_rpattern const & that )
      : basic_rpattern_base<CI, SY>( that )
    {
    }

    explicit basic_rpattern( 
        string_type const & pat,
        REGEX_FLAGS flags = NOFLAGS,
        REGEX_MODE mode = MODE_DEFAULT ) //throw( bad_regexpr, std::bad_alloc )
      : basic_rpattern_base<CI, SY>( pat, flags, mode )
    {
    }

    basic_rpattern(
        string_type const & pat,
        string_type const & subst,
        REGEX_FLAGS flags = NOFLAGS,
        REGEX_MODE mode = MODE_DEFAULT ) //throw( bad_regexpr, std::bad_alloc )
      : basic_rpattern_base<CI, SY>( pat, subst, flags, mode )
    {
    }

    basic_rpattern & operator=( basic_rpattern<CI, SY> const & that ) //throw( bad_regexpr, std::bad_alloc )
    {
        basic_rpattern_base<CI, SY>::operator=( that );
        return *this;
    }

    // CI2 must be convertible to type CI
    template< typename CI2 >
    backref_type const & match(
        CI2 ibegin,
        CI2 iend,
        basic_match_results<CI> & results ) const
    {
        // If your compile breaks here, it is because CI2 is not
        // convertible to type CI. Check the declaration of your rpattern object.
        detail::static_assert< detail::is_convertible<CI2,CI>::value > const iterator_types_are_not_convertible;
        ( void ) iterator_types_are_not_convertible;

        detail::match_param<CI> param = detail::matcher_helper<CI>::init_param( ibegin, iend, results );
        return impl::_do_match( param, false ) ? results.backref(0) : detail::static_init<backref_type>::value;
    }

    template< typename CH >
    backref_type const & match(
        CH * szbegin,
        basic_match_results<CI> & results ) const
    {
        // If your compile breaks here, it is because CI2 is not
        // convertible to type CI. Check the declaration of your rpattern object.
        typedef CH * CI2;
        detail::static_assert< detail::is_convertible<CI2,CI>::value > const iterator_types_are_not_convertible;
        ( void ) iterator_types_are_not_convertible;

        detail::match_param<CI> param = detail::matcher_helper<CI>::init_param( szbegin, (CH*)0, results );
        return impl::_do_match( param, szbegin ) ? results.backref(0) : detail::static_init<backref_type>::value;
    }

    template< typename CH, typename TR, typename AL >
    backref_type const & match(
        std::basic_string<CH, TR, AL> const & str,
        basic_match_results<CI> & results,
        size_type pos,
        size_type len ) const
    {
        // If your compile breaks here, it is because CI2 is not
        // convertible to type CI. Check the declaration of your rpattern object.
        typedef typename std::basic_string<CH, TR, AL>::const_iterator CI2;
        detail::static_assert< detail::is_convertible<CI2,CI>::value > const iterator_types_are_not_convertible;
        ( void ) iterator_types_are_not_convertible;

        detail::match_param<CI> param = detail::matcher_helper<CI>::init_param( str.begin(), str.begin(), results );

        if( len == npos || pos + len >= str.size() )
            param.istop = CI(str.end());
        else
            std::advance( param.istop, pos + len );

        std::advance( param.istart, pos );
        param.ibegin = param.istart;

        return impl::_do_match( param, false ) ? results.backref(0) : detail::static_init<backref_type>::value;
    }

    template< typename CH, typename TR, typename AL >
    backref_type const & match(
        std::basic_string<CH, TR, AL> const & str,
        basic_match_results<CI> & results ) const
    {
        return match( str, results, 0, impl::npos );
    }

    template< typename CI2 >
    size_t count( CI2 ibegin, CI2 iend ) const
    {
        // If your compile breaks here, it is because CI2 is not
        // convertible to type CI. Check the declaration of your rpattern object.
        detail::static_assert< detail::is_convertible<CI2,CI>::value > const iterator_types_are_not_convertible;
        ( void ) iterator_types_are_not_convertible;

        backref_vector rgbackrefs;
        detail::match_param<CI> param( ibegin, iend, & rgbackrefs );
        return impl::_do_count( param, false );
    }

    template< typename CH >
    size_t count( CH * szbegin ) const
    {
        // If your compile breaks here, it is because CI2 is not
        // convertible to type CI. Check the declaration of your rpattern object.
        typedef CH * CI2;
        detail::static_assert< detail::is_convertible<CI2,CI>::value > const iterator_types_are_not_convertible;
        ( void ) iterator_types_are_not_convertible;

        backref_vector rgbackrefs;
        // If your compile breaks here, it is because CH const * is not
        // convertible to type CI. Check the declaration of your rpattern object.
        detail::match_param<CI> param( szbegin, (CH*)0, & rgbackrefs );
        return impl::_do_count( param, szbegin );
    }

    template< typename CH, typename TR, typename AL >
    size_t count(
        std::basic_string<CH, TR, AL> const & str,
        size_type pos,
        size_type len ) const
    {
        // If your compile breaks here, it is because CI2 is not
        // convertible to type CI. Check the declaration of your rpattern object.
        typedef typename std::basic_string<CH, TR, AL>::const_iterator CI2;
        detail::static_assert< detail::is_convertible<CI2,CI>::value > const iterator_types_are_not_convertible;
        ( void ) iterator_types_are_not_convertible;

        backref_vector rgbackrefs;
        // If your compile breaks here, then the string you passed in has
        // a different iterator type than this rpattern type is expecting
        detail::match_param<CI> param( str.begin(), str.begin(), & rgbackrefs );

        if( len == npos || pos + len >= str.size() )
            param.istop = CI(str.end());
        else
            std::advance( param.istop, pos + len );

        std::advance( param.istart, pos );
        param.ibegin = param.istart;

        return impl::_do_count( param, false );
    }

    template< typename CH, typename TR, typename AL >
    size_t count( std::basic_string<CH, TR, AL> const & str ) const
    {
        return count( str, 0, impl::npos );
    }

    template< typename CH, typename TR, typename AL >
    size_t substitute(
        std::basic_string<CH, TR, AL> & str,
        basic_subst_results<CH, TR, AL> & results,
        size_type pos,
        size_type len ) const
    {
        // If your compile breaks here, it is because CI2 is not
        // convertible to type CI. Check the declaration of your rpattern object.
        typedef typename std::basic_string<CH, TR, AL>::const_iterator CI2;
        detail::static_assert< detail::is_convertible<CI2,CI>::value > const iterator_types_are_not_convertible;
        ( void ) iterator_types_are_not_convertible;

        return detail::matcher_helper<CI>::_Do_subst( *this, str, results, pos, len );
    }

    template< typename CH, typename TR, typename AL >
    size_t substitute(
        std::basic_string<CH, TR, AL> & str,
        basic_subst_results<CH, TR, AL> & results ) const
    {
        return substitute( str, results, 0, impl::npos );
    }
};

// --------------------------------------------------------------------------
//
// Class:       basic_rpattern_c
//
// Description: a pattern object optimized for matching C-style, NULL-
//              terminated strings. It treats the null-terminator as
//              the end-of-string condition.
//
// Methods:     basic_rpattern_c - c'tor
//              basic_rpattern_c -
//              basic_rpattern_c -
//              match            - match a null-terminated string
//              count            - count matches in a null-terminated string
//              _do_match_c      - internal implementation
//              _do_count_c      - internal implementation
//
// History:     8/13/2001 - ericne - Created
//
// --------------------------------------------------------------------------
template< typename CH, typename SY = perl_syntax<CH> >
class basic_rpattern_c : public basic_rpattern_base<CH const *, SY>
{
    typedef detail::basic_rpattern_base_impl<CH const *> impl;
public:
    typedef typename basic_rpattern_base<CH const *, SY>::syntax_type     syntax_type;
    typedef typename basic_rpattern_base<CH const *, SY>::char_type       char_type;
    typedef typename basic_rpattern_base<CH const *, SY>::traits_type     traits_type;

    typedef typename basic_rpattern_base<CH const *, SY>::string_type     string_type;
    typedef typename basic_rpattern_base<CH const *, SY>::size_type       size_type;

    typedef typename basic_rpattern_base<CH const *, SY>::backref_type    backref_type;
    typedef typename basic_rpattern_base<CH const *, SY>::backref_vector  backref_vector;

    basic_rpattern_c() //throw()
      : basic_rpattern_base<CH const *, SY>()
    {
    }

    basic_rpattern_c( basic_rpattern_c const & that )
      : basic_rpattern_base<CH const *, SY>( that )
    {
    }

    explicit basic_rpattern_c(
        string_type const & pat,
        REGEX_FLAGS flags = NOFLAGS,
        REGEX_MODE mode = MODE_DEFAULT ) //throw( bad_regexpr, std::bad_alloc )
      : basic_rpattern_base<CH const *, SY>( pat, flags, mode )
    {
    }

    basic_rpattern_c & operator=( basic_rpattern_c<CH, SY> const & that )
    {
        basic_rpattern_base<CH const *, SY>::operator=( that );
        return *this;
    }

    backref_type const & match(
        CH const * szbegin,
        basic_match_results_c<CH> & results ) const
    {
        detail::match_param<CH const *> param = detail::matcher_helper<CH const *>::init_param( szbegin, 0, results );
        return impl::_do_match( param, szbegin ) ? results.backref(0) : detail::static_init<backref_type>::value;
    }

    size_t count( CH const * szbegin ) const
    {
        backref_vector rgbackrefs;
        detail::match_param<CH const *> param( szbegin, 0, & rgbackrefs );
        return impl::_do_count( param, szbegin );
    }
};



#if defined(UNICODE) | defined(_UNICODE)
typedef wchar_t rechar_t;
#else
typedef char    rechar_t;
#endif

typedef std::basic_string<rechar_t> restring;

// On many implementations of the STL, string::iterator is not a typedef
// for char*. Rather, it is a wrapper class. As a result, the regex code 
// gets instantiated twice, once for bare pointers (rpattern_c) and once for
// the wrapped pointers (rpattern). But if there is a conversion from the 
// bare ptr to the wrapped ptr, then we only need to instantiate the template
// for the wrapped ptr, and the code will work for the bare ptrs, too.
// This can be a significant space savings.  The REGEX_FOLD_INSTANTIONS 
// macro controls this optimization. The default is "off" for backwards
// compatibility. To turn the optimization on, compile with:
// -DREGEX_FOLD_INSTANTIATIONS=1
#ifndef REGEX_FOLD_INSTANTIATIONS
#define REGEX_FOLD_INSTANTIATIONS 0
#endif

typedef ::regex::detail::select
< 
    REGEX_FOLD_INSTANTIATIONS &&
        detail::is_convertible<rechar_t const *,restring::const_iterator>::value,
    restring::const_iterator,
    rechar_t const *
>::type lpctstr_t;

// For matching against null-terminated strings
typedef basic_rpattern<lpctstr_t, perl_syntax<rechar_t> >  perl_rpattern_c;
typedef basic_rpattern<lpctstr_t, posix_syntax<rechar_t> > posix_rpattern_c;

// For matching against std::strings
typedef basic_rpattern<restring::const_iterator, perl_syntax<rechar_t> >  perl_rpattern;
typedef basic_rpattern<restring::const_iterator, posix_syntax<rechar_t> > posix_rpattern;

// Default to perl syntax
typedef perl_rpattern    rpattern;
typedef perl_rpattern_c  rpattern_c;

// typedefs for the commonly used match_results and subst_results
typedef basic_match_results<restring::const_iterator> match_results;
typedef basic_match_results<lpctstr_t>                match_results_c;
typedef basic_subst_results<rechar_t>                 subst_results;

#if defined(_MSC_VER) & _MSC_VER >= 1300
// These are no longer useful, and will go away in a future release
// You should be using the version without the _c
# pragma deprecated( basic_rpattern_c )
# pragma deprecated( basic_match_results_c )
#endif

//
// Define some classes and macros for creating function-local
// static const rpatterns in a thread-safe way
//

#if defined( _MSC_VER ) & defined( _MT )

namespace detail
{

template< typename PAT >
class rpattern_destroyer
{
    PAT           const & m_refPat;
    bool volatile const & m_fConstructed;
public:
    rpattern_destroyer( volatile bool const & fConstructed, PAT const & refPat )
        : m_fConstructed( fConstructed ), m_refPat( refPat )
    {
    }
    ~rpattern_destroyer()
    {
        if( m_fConstructed )
            ( &m_refPat )->~PAT();
    }
};

struct CRegExLock
{
    CRegExLock();
    ~CRegExLock();
};

} // namespace detail

#define STATIC_RPATTERN_EX( type, var, params ) \
    static unsigned char s_rgb_##var[ sizeof( type ) ]; \
    static bool volatile s_f_##var = false; \
    static type const & var = *reinterpret_cast<type*>( s_rgb_##var ); \
    static regex::detail::rpattern_destroyer<type> const s_des_##var( s_f_##var, var ); \
    if( ! s_f_##var ) \
    { \
    regex::detail::CRegExLock objLock; \
        if( ! s_f_##var ) \
        { \
            new( static_cast<void*>( s_rgb_##var ) ) type params; \
            s_f_##var = true; \
        } \
    }

#else

#define STATIC_RPATTERN_EX( type, var, params ) \
    static type const var params;

#endif

#define STATIC_RPATTERN( var, params ) \
    STATIC_RPATTERN_EX( regex::rpattern, var, params )

#define STATIC_RPATTERN_C( var, params ) \
    STATIC_RPATTERN_EX( regex::rpattern_c, var, params )


//
// ostream inserter operator for back-references
//
template< typename CH, typename TR, typename CI >
inline std::basic_ostream<CH, TR> & operator<<(
    std::basic_ostream<CH, TR> & sout,
    backref_tag<CI> const & br )
{
    return br.print( sout );
}

} // namespace regex


//
// specializations for std::swap
//
namespace std
{
    template<> 
    inline void swap( regex::detail::regex_arena & left, regex::detail::regex_arena & right )
    {
        left.swap( right );
    }

    template< typename CI, typename SY >
    inline void swap( regex::basic_rpattern_base<CI, SY> & left, regex::basic_rpattern_base<CI, SY> & right )
    {
        left.swap( right );
    }
}

#ifdef _MSC_VER
#pragma warning( pop )
#endif

#endif
