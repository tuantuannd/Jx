//+---------------------------------------------------------------------------
//
//  Copyright ( C ) Microsoft Corporation, 1994 - 2002.
//
//  File:       regexpr2.cpp
//
//  Contents:   implementation for rpattern methods, definitions for all the
//              subexpression types used to perform the matching, the
//              charset class definition .
//
//  Classes:    too many to list here
//
//  Functions:
//
//  Author:     Eric Niebler ( ericne@microsoft.com )
//
//  History:    12-11-1998   ericne   Created
//              01-05-2001   ericne   Removed dependency on VC's choice
//                                    of STL iterator types.
//              08-15-2001   ericne   Removed regexpr class, moved match
//                                    state to match_results container.
//              09-17-2001   nathann  Add DEBUG_HEAP_SUPPORT
//              11-16-2001   ericne   Add stack-conservative algorithm
//
//----------------------------------------------------------------------------

#ifdef _MSC_VER
// unlimited inline expansion ( compile with /Ob1 or /Ob2 )
# pragma inline_recursion( on )
# pragma inline_depth( 255 )
// warning C4127: conditional expression is constant
// warning C4355: 'this' : used in base member initializer list
// warning C4702: unreachable code
// warning C4710: function 'blah' not inlined
// warning C4786: identifier was truncated to '255' characters in the debug information
# pragma warning( disable : 4127 4355 4702 4710 4786 )
#endif

#include <cctype>
#include <cwctype>
#include <cassert>
#include <malloc.h>
#include <algorithm>
#include <functional>

#if defined( _MSC_VER ) & defined( _MT )
# include <windows.h>
#endif

// If the implementation file has been included in the header, then we
// need to mark some functions as inline to prevent them from being multiply
// defined.  But if the implementation file is not included in the header,
// we can't mark them as inline, otherwise the linker won't find them.
#ifdef REGEXPR_H
# define REGEXPR_H_INLINE inline
#else
# define REGEXPR_H_INLINE
# include "regexpr2.h"
#endif

#ifndef alloca
# define alloca _alloca
#endif

// Rather non-portable code below. The flags to the _isctype
// CRT routine, and even the _isctype routine itself, are
// not standard. This works for me on VC and on my linux box,
// but it probably won't work for everyone.  :-(
#ifndef _MSC_VER
# define __assume( x ) assert( false ); return NULL;
# define _UPPER   _ISupper
# define _LOWER   _ISlower
# define _ALPHA   _ISalpha
# define _DIGIT   _ISdigit
# define _HEX     _ISxdigit
# define _SPACE   _ISspace
# define _PRINT   _ISprint
# define _GRAPH   _ISgraph
# define _BLANK   _ISblank
# define _CONTROL _IScntrl
# define _PUNCT   _ISpunct
# define _ALNUM   _ISalnum
#else
# define _ALNUM ( _UPPER|_LOWER|_DIGIT )
# define _PRINT ( _BLANK|_PUNCT|_UPPER|_LOWER|_DIGIT )
# define _GRAPH ( _PUNCT|_UPPER|_LOWER|_DIGIT )
#endif

namespace regex
{

namespace detail
{

// For portably assigning a bare ptr to an auto_ptr.
// (don't use reset() because some STL implementations
// don't support it.)
template< typename T, typename U >
inline void assign_auto_ptr( std::auto_ptr<T> & lhs, U * rhs )
{
    std::auto_ptr<T> temp( rhs );
    lhs = temp;
}

// VC's STL member function adapters don't handle const member functions,
// so explicitly handle that special case with const_mem_fun1_t
template<class R, class Ty, class A>
class const_mem_fun1_t : public std::binary_function<Ty const *, A, R>
{
    R ( Ty::*m_p )( A ) const;
public:
    explicit const_mem_fun1_t( R ( Ty::*p )( A ) const )
        : m_p( p ) {}
    R operator()( Ty const * p, A arg ) const
    {
        return ( p->*m_p )( arg );
    }
};

template<class R, class Ty, class A>
inline const_mem_fun1_t<R, Ty, A> mem_fun( R ( Ty::*p )( A ) const )
{
    return const_mem_fun1_t<R, Ty, A>( p );
}

// On some systems, isctype is implemented as a macro. I need
// a function so that I can bind args and use it in algorithms.
#ifdef _isctype
# error _isctype is a macro. It needs to be a function.
#endif
#ifdef __isctype
 inline int _isctype( int c, int type ) { return __isctype( c, type ); }
#endif

#if defined( _MSC_VER ) & defined( _MT )

// Global critical section used to synchronize the creation of static const patterns
class CRegExCritSect : private CRITICAL_SECTION
{
    friend struct CRegExLock;
    CRegExCritSect( CRegExCritSect const & );
    CRegExCritSect()  { InitializeCriticalSection( this ); }
    void Enter()      { EnterCriticalSection( this ); }
    void Leave()      { LeaveCriticalSection( this ); }
    static CRegExCritSect & Instance()
    {
        static CRegExCritSect s_objRegExCritSect;
        return s_objRegExCritSect;
    }
public:
    ~CRegExCritSect() { DeleteCriticalSection( this ); }
};

REGEXPR_H_INLINE CRegExLock::CRegExLock()
{
    CRegExCritSect::Instance().Enter();
}

REGEXPR_H_INLINE CRegExLock::~CRegExLock()
{
    CRegExCritSect::Instance().Leave();
}

#endif

template< typename II, typename CI >
inline size_t parse_int( II & istr, CI iend, size_t const max_ = unsigned( -1 ) )
{
    typedef typename std::iterator_traits<II>::value_type CH;
    size_t retval = 0;
    while( iend != istr && REGEX_CHAR(CH,'0') <= *istr && REGEX_CHAR(CH,'9') >= *istr && max_ > retval )
    {
        retval *= 10;
        retval += ( size_t )( *istr - REGEX_CHAR(CH,'0') );
        ++istr;
    }
    if( max_ < retval )
    {
        retval /= 10;
        --istr;
    }
    return retval;
}

// Here is the implementation for the regex_arena class.
// It takes advantage of the fact that all subexpression objects
// allocated during pattern compilation will be freed all at once.
// The sub_expr, custom_charset and basic_rpattern classes all must
// cooperate with this degenerate allocation scheme.  But it is fast
// and effective.  My patterns compile 40% faster with it.  YMMV.

// NathanN:
// By defining the symbol REGEX_DEBUG_HEAP the allocator object
// no longer sub allocates memory.  This enables heap checking tools like
// AppVerifier & PageHeap to find errors like buffer overruns
#ifndef REGEX_DEBUG_HEAP
# if REGEX_DEBUG
#  define REGEX_DEBUG_HEAP 1
# else
#  define REGEX_DEBUG_HEAP 0
# endif
#endif

REGEXPR_H_INLINE size_t DEFAULT_BLOCK_SIZE()
{
#   if REGEX_DEBUG_HEAP
    // put each allocation in its own block
    return 1;
#   else
    // put multiple allocation in each block
    return 352;
#   endif
}

struct regex_arena::block
{
    block * m_pnext;
    size_t m_offset;
    enum { HEADER_SIZE = sizeof( block* ) + sizeof( size_t ) };
    unsigned char m_data[ 1 ];
};

inline void regex_arena::_new_block( size_t size )
{
    size_t blocksize = (std::max)( m_default_size, size ) + block::HEADER_SIZE;
    block * pnew = static_cast<block*>( ::operator new( blocksize ) );
    pnew->m_offset = 0;
    pnew->m_pnext  = m_pfirst;
    m_pfirst = pnew;
}

REGEXPR_H_INLINE regex_arena::regex_arena( size_t default_size )
    : m_pfirst( NULL ), m_default_size( default_size )
{
}

REGEXPR_H_INLINE regex_arena::~regex_arena()
{
    deallocate();
}

REGEXPR_H_INLINE void regex_arena::deallocate()
{
    for( block * pnext; m_pfirst; m_pfirst = pnext )
    {
        pnext = m_pfirst->m_pnext;
        ::operator delete( static_cast<void*>( m_pfirst ) );
    }
}

struct not_pod
{
    virtual ~not_pod() {}
};

REGEXPR_H_INLINE void * regex_arena::allocate( size_t size )
{
    if( 0 == size )
        size = 1;

    if( NULL == m_pfirst || m_pfirst->m_offset + size > m_default_size )
        _new_block( size );

    void * pnew = m_pfirst->m_data + m_pfirst->m_offset;

    // ensure returned pointers are always suitably aligned
    m_pfirst->m_offset += ( ( size + alignof<not_pod>::value - 1 )
                            & ~( alignof<not_pod>::value - 1 ) );

    return pnew;
}

REGEXPR_H_INLINE size_t regex_arena::max_size() const
{
    return size_t( -1 );
}

REGEXPR_H_INLINE void regex_arena::swap( regex_arena & that )
{
    std::swap( m_pfirst, that.m_pfirst );
    std::swap( m_default_size, that.m_default_size );
}

template< typename T >
inline void regex_destroy( T * pt ) { pt; pt->~T(); }
inline void regex_destroy( char * ) {}
inline void regex_destroy( wchar_t * ) {}

////
// regex_allocator is a proper STL allocator.  It is a thin
// wrapper around the regex_arrena object.  Note that deallocate
// does nothing.  Memory isn't freed until the arena object
// gets destroyed.
template< typename T >
struct regex_allocator
{
    typedef size_t size_type;
    typedef ptrdiff_t difference_type;
    typedef T *pointer;
    typedef T const *const_pointer;
    typedef T & reference;
    typedef T const & const_reference;
    typedef T value_type;
    regex_allocator( regex_arena & arena )
        : m_arena( arena ) { align_check(); }
#if !defined(_MSC_VER) | _MSC_VER >= 1300
    regex_allocator( regex_allocator const & alloc )
        : m_arena( alloc.m_arena ) { align_check(); }
#endif
    template< typename U >
    regex_allocator( regex_allocator<U> const & alloc )
        : m_arena( alloc.m_arena ) { align_check(); }
    pointer address( reference x ) const
        {return &x;}
    const_pointer address( const_reference x ) const
        {return &x;}
    pointer allocate( size_type size, void const * =0 )
        {return static_cast<pointer>( m_arena.allocate( size * sizeof( T ) ) ); }
    char *_Charalloc( size_type size )
        {return static_cast<char*>( m_arena.allocate( size ) ); }
    void deallocate( void *, size_type )
        {}
    void construct( pointer p, T const & t )
        {new( ( void* )p ) T( t );}
    void destroy( pointer p )
        {regex_destroy( p );}
    size_t max_size() const
        {size_t size = m_arena.max_size() / sizeof( T );
         return ( 0 < size ? size : 1 );}
    template< typename U > struct rebind
        {typedef regex_allocator<U> other;};

    // BUGBUG after rpattern::swap, all regex_allocator
    // objects refer to the wrong arena.
    regex_arena & m_arena;
private:
    regex_allocator & operator=( regex_allocator const & );
    static void align_check()
    {
        // The regex_arena uses not_pod to align memory. Use a compile-time
        // assertion to make sure that T does not have worse alignment than not_pod.
        static_assert<( ( size_t ) alignof<T>::value <= ( size_t ) alignof<not_pod>::value )> const align_check;
        ( void ) align_check;
    }
};

template< typename T, typename U >
inline bool operator==( regex_allocator<T> const & rhs, regex_allocator<U> const & lhs )
    {return &rhs.m_arena == &lhs.m_arena;}

template< typename T, typename U >
inline bool operator!=( regex_allocator<T> const & rhs, regex_allocator<U> const & lhs )
    {return &rhs.m_arena != &lhs.m_arena;}

// Use the regex allocator by default because it makes pattern compilation
// and clean-up go much faster. If you define REGEX_NO_ALLOCATOR, though,
// you can save some code bloat.

// BUGBUG This is actually implementation-dependent. regex_allocator is
// not a 100% compliant STL allocator. It does not have a default c'tor, and
// all regex_allocator<T> instances do not necessarily compare equal for
// any type T. To be truly portable, I would need to write my own containers
// that do not make any of these assumptions.  Sigh.  Alternatively, I could
// just give up passing regex_allocators to STL containers by compiling with
// REGEX_NO_ALLOCATOR defined, but this make compiling patterns slow.  :-(

#ifndef REGEX_NO_ALLOCATOR
# define REGEX_ALLOCATOR regex_allocator
#else
# define REGEX_ALLOCATOR std::allocator
#endif

#if defined(_MSC_VER) & _MSC_VER < 1300

# ifndef REGEX_NO_ALLOCATOR
#  define MAKE_ALLOCATOR(type,arena) regex_allocator<type>(arena)
# else
#  define MAKE_ALLOCATOR(type,arena) std::allocator<type>()
# endif

#else
// Define an allocator factory that can create
// an allocator from a regex_arena.
template< typename AL >
struct allocator_factory
{
    template< typename T >
    static typename AL::template rebind<T>::other create( regex_arena & )
    {
        typedef typename AL::template rebind<T>::other other;
        return other();
    }
};
template<>
struct allocator_factory< regex_allocator<char> >
{
    template< typename T >
    static regex_allocator<T> create( regex_arena & arena )
    {
        return regex_allocator<T>( arena );
    }
};
#define MAKE_ALLOCATOR(type,arena) allocator_factory<REGEX_ALLOCATOR<char> >::template create<type>( arena )
#endif

// This class is used to speed up character set matching by providing
// a bitset that spans the ASCII range. std::bitset is not used because
// the range-checking slows it down.
// Note: The division and modulus operations are optimized by the compiler
// into bit-shift operations.
class ascii_bitvector
{
    typedef unsigned int elem_type;

    enum { CBELEM = CHAR_BIT * sizeof( elem_type ), // count of bits per element
           CELEMS = ( UCHAR_MAX+1 ) / CBELEM };       // number of element in array
    elem_type m_rg[ CELEMS ];

    // Used to inline operations like: bv1 |= ~bv2; without creating temp bit vectors.
    struct not_ascii_bitvector
    {
        ascii_bitvector const & m_ref;
        not_ascii_bitvector( ascii_bitvector const & ref )
            : m_ref( ref ) {}
    private:
        not_ascii_bitvector & operator=( not_ascii_bitvector const & );
    };
public:
    ascii_bitvector()
        { zero(); }

    void zero()
        { memset( m_rg, 0, CELEMS * sizeof( elem_type ) ); }

    void set( unsigned char ch )
        { m_rg[ ( ch / CBELEM ) ] |= ( ( elem_type )1U << ( ch % CBELEM ) ); }

    bool operator[]( unsigned char ch ) const
        { return 0 != ( m_rg[ ( ch / CBELEM ) ] & ( ( elem_type )1U << ( ch % CBELEM ) ) ); }

    not_ascii_bitvector const operator~() const
        { return not_ascii_bitvector( *this ); }

    ascii_bitvector & operator|=( ascii_bitvector const & that )
        { for( int i=0; i<CELEMS; ++i )
              m_rg[ i ] |= that.m_rg[ i ];
          return *this; }

    ascii_bitvector & operator|=( not_ascii_bitvector const & that )
        { for( int i=0; i<CELEMS; ++i )
              m_rg[ i ] |= ~that.m_ref.m_rg[ i ];
          return *this; }

    ascii_bitvector & operator=( ascii_bitvector const & that )
        { for( int i=0; i<CELEMS; ++i )
              m_rg[ i ] = that.m_rg[ i ];
          return *this; }

    ascii_bitvector & operator=( not_ascii_bitvector const & that )
        { for( int i=0; i<CELEMS; ++i )
              m_rg[ i ] = ~that.m_ref.m_rg[ i ];
          return *this; }
};

typedef std::pair<wchar_t, wchar_t> range_type;

// determines if one range is less then another.
// used in binary search of range vector
struct range_less : public std::binary_function< range_type, range_type, bool >
{
    inline bool operator()( range_type const & rg1, range_type const & rg2 ) const
    {
        return rg1.second < rg2.first;
    }
};

struct charset;

template< typename A1, typename A2, typename A3 >
struct charset_t
{
    bool                            m_fcompliment;
    bool                            m_fskip_extended_check;
    ascii_bitvector                 m_ascii_bitvector;
    wctype_t                        m_posixcharson;
    std::vector<range_type, A1>     m_range_vector;
    std::list<wctype_t, A2>         m_posixcharsoff;
    std::list<charset const *, A3>  m_nestedcharsets;
    
    charset_t( A1 const & a1 = A1(), A2 const & a2 = A2(), A3 const & a3 = A3() )
        : m_fcompliment( false ),
          m_fskip_extended_check( false ),
          m_ascii_bitvector(),
          m_posixcharson( 0 ),
          m_range_vector( a1 ),
          m_posixcharsoff( a2 ),
          m_nestedcharsets( a3 )
    {
    }

    // We'll be inheriting from this, so a virtual d'tor is regretably necessary.
    virtual ~charset_t()
    {
    }

    void clear()
    {
        m_fcompliment = false;
        m_fskip_extended_check = false;
        m_ascii_bitvector.zero();
        m_posixcharson = 0;
        m_range_vector.clear();
        m_posixcharsoff.clear();
        m_nestedcharsets.clear();
    }

    void add_range( range_type rg )
    {
        // Prevent excessive reallocs by reserving in blocks of 5
        if( m_range_vector.capacity() == m_range_vector.size() )
            m_range_vector.reserve( m_range_vector.size() + 5 );
        m_range_vector.push_back( rg );
    }

    // merge one charset into another
    charset_t & operator|=( charset const & that )
    {
        if( that.m_fcompliment )
        {
            // If no posix-style character sets are used, then we can merge this
            // nested character set directly into the enclosing character set.
            if( 0 == that.m_posixcharson     &&
                that.m_posixcharsoff.empty() &&
                that.m_nestedcharsets.empty() )
            {
                m_ascii_bitvector |= ~ that.m_ascii_bitvector;

                // append the inverse of that.m_range_vector to this->m_range_vector
                wchar_t chlow = UCHAR_MAX;
                typedef std::vector<range_type>::const_iterator VCI;
                for( VCI prg = that.m_range_vector.begin(); prg != that.m_range_vector.end(); ++prg )
                {
                    if( UCHAR_MAX + 1 != prg->first )
                        add_range( range_type( wchar_t( chlow+1 ), wchar_t( prg->first-1 ) ) );
                    chlow = prg->second;
                }
                if( WCHAR_MAX != chlow )
                    add_range( range_type( wchar_t( chlow+1 ), WCHAR_MAX ) );
            }
            else
            {
                // There is no simple way to merge this nested character
                // set into the enclosing character set, so we must save
                // a pointer to the nested character set in a list.
                m_nestedcharsets.push_back( & that );
            }
        }
        else
        {
            m_ascii_bitvector |= that.m_ascii_bitvector;
            m_range_vector.insert( m_range_vector.end(),
                that.m_range_vector.begin(),
                that.m_range_vector.end() );

            m_posixcharson |= that.m_posixcharson;
            std::copy( that.m_posixcharsoff.begin(),
                       that.m_posixcharsoff.end() ,
                       std::back_inserter( m_posixcharsoff ) );

            std::copy( that.m_nestedcharsets.begin(),
                       that.m_nestedcharsets.end(),
                       std::back_inserter( m_nestedcharsets ) );
        }
        return *this;
    }

    // Note overloading based on second parameter
    void set_bit( char ch, bool const fnocase )
    {
        if( fnocase )
        {
            m_ascii_bitvector.set( static_cast<unsigned char>( regex_tolower( ch ) ) );
            m_ascii_bitvector.set( static_cast<unsigned char>( regex_toupper( ch ) ) );
        }
        else
        {
            m_ascii_bitvector.set( static_cast<unsigned char>( ch ) );
        }
    }

    // Note overloading based on second parameter
    void set_bit( wchar_t ch, bool const fnocase )
    {
        if( UCHAR_MAX >= ch )
            set_bit( static_cast<char>( ch ), fnocase );
        else
            add_range( range_type( ch, ch ) );
    }

    // Note overloading based on second parameter
    void set_bit_range( char ch1, char ch2, bool const fnocase )
    {
        if( static_cast<unsigned char>( ch1 ) > static_cast<unsigned char>( ch2 ) )
            throw bad_regexpr( "invalid range specified in character set" );

        if( fnocase )
        {
            // i is unsigned int to prevent overflow if ch2 is UCHAR_MAX
            for( unsigned int i = static_cast<unsigned char>( ch1 ); 
                 i <= static_cast<unsigned char>( ch2 ); ++i )
            {
                m_ascii_bitvector.set( static_cast<unsigned char>( regex_toupper( (char)i ) ) );
                m_ascii_bitvector.set( static_cast<unsigned char>( regex_tolower( (char)i ) ) );
            }
        }
        else
        {
            // i is unsigned int to prevent overflow if ch2 is UCHAR_MAX
            for( unsigned int i = static_cast<unsigned char>( ch1 ); 
                 i <= static_cast<unsigned char>( ch2 ); ++i )
            {
                m_ascii_bitvector.set( static_cast<unsigned char>( i ) );
            }
        }
    }

    // Note overloading based on second parameter
    void set_bit_range( wchar_t ch1, wchar_t ch2, bool const fnocase )
    {
        if( ch1 > ch2 )
            throw bad_regexpr( "invalid range specified in character set" );

        if( UCHAR_MAX >= ch1 )
            set_bit_range( static_cast<char>( ch1 ), static_cast<char>( (std::min)( static_cast<wchar_t>( UCHAR_MAX ), ch2 ) ), fnocase );

        if( UCHAR_MAX < ch2 )
            add_range( range_type( (std::max)( static_cast<wchar_t>( UCHAR_MAX+1 ), ch1 ), ch2 ) );
    }

    void optimize( type2type<wchar_t> )
    {
        // this sorts on range_type.first ( uses operator<() for pair templates )
        std::sort( m_range_vector.begin(), m_range_vector.end() );

        // This merges ranges that overlap
        for( size_t index = 1; index < m_range_vector.size(); )
        {
            if( m_range_vector[ index ].first <= m_range_vector[ index-1 ].second + 1 )
            {
                m_range_vector[ index-1 ].second = (std::max)(
                    m_range_vector[ index-1 ].second, m_range_vector[ index ].second );
                m_range_vector.erase( m_range_vector.begin() + index );
            }
            else
                ++index;
        }

        // For the ASCII range, merge the m_posixcharson info
        // into the ascii_bitvector
        if( m_posixcharson )
        {
            // BUGBUG this is kind of expensive. Think of a better way.
            for( unsigned int i=0; i<=UCHAR_MAX; ++i )
                if( _isctype( i, m_posixcharson ) )
                    m_ascii_bitvector.set( static_cast<unsigned char>( i ) );
        }

        // m_fskip_extended_check is a cache which tells us whether we
        // need to check the m_posixcharsoff and m_nestedcharsets vectors,
        // which would only be used in nested user-defined character sets
        m_fskip_extended_check = m_posixcharsoff.empty() && m_nestedcharsets.empty();
    }

    void optimize( type2type<char> )
    {
        optimize( type2type<wchar_t>() );

        // the posixcharson info was merged into the ascii bitvector,
        // so we don't need to ever call _isctype ever again.
        m_posixcharson = 0;
    }

#define DECLARE_EXTENDED_CHECK(FUN,CHAR,CTYPE,PMF)\
    bool FUN( CHAR ch ) const\
    {\
        if( m_fskip_extended_check )\
        {\
            assert( m_posixcharsoff.empty() && m_nestedcharsets.empty() );\
            return false;\
        }\
        return ( m_posixcharsoff.end() !=\
                 std::find_if( m_posixcharsoff.begin(), m_posixcharsoff.end(),\
                               std::not1( std::bind1st( std::ptr_fun( CTYPE ), ch ) ) ) )\
            || ( m_nestedcharsets.end() !=\
                 std::find_if( m_nestedcharsets.begin(), m_nestedcharsets.end(),\
                               std::bind2nd( regex::detail::mem_fun( PMF ), ch ) ) );\
    }

    DECLARE_EXTENDED_CHECK(extended_check_narrow,char,_isctype,&charset::in_narrow)
    DECLARE_EXTENDED_CHECK(extended_check_wide_with_case,wchar_t,iswctype,&charset::in_wide_with_case)
    DECLARE_EXTENDED_CHECK(extended_check_wide_no_case,wchar_t,iswctype,&charset::in_wide_no_case)
#undef DECLARE_EXTENDED_CHECK

    // Note overloading based on parameter
    bool in_narrow( char ch ) const
    {
        // Whoops, forgot to call optimize() on this charset
        assert( 0 == m_posixcharson );

        return m_fcompliment !=
               (
                    ( m_ascii_bitvector[ static_cast<unsigned char>( ch ) ] )
                 || ( extended_check_narrow( ch ) )
               );
    }

    inline bool in_range_vector_with_case( wchar_t ch ) const
    {
        return std::binary_search( m_range_vector.begin(), m_range_vector.end(),
            range_type( ch, ch ), range_less() );
    }

    inline bool in_range_vector_no_case( wchar_t ch ) const
    {
        wchar_t const chup = regex_toupper( ch );
        if( std::binary_search( m_range_vector.begin(), m_range_vector.end(),
                range_type( chup, chup ), range_less() ) )
            return true;

        wchar_t const chlo = regex_tolower( ch );
        if( chup != chlo &&
            std::binary_search( m_range_vector.begin(), m_range_vector.end(),
                range_type( chlo, chlo ), range_less() ) )
            return true;

        return false;
    }

    // Note overloading based on parameter
    bool in_wide_with_case( wchar_t ch ) const
    {
        // use range_match_type to see if this character is within one of the
        // ranges stored in m_rgranges.
        return m_fcompliment !=
               (
                    ( ( UCHAR_MAX >= ch ) ?
                      ( m_ascii_bitvector[ static_cast<unsigned char>( ch ) ] ) :
                      (    ( ! m_range_vector.empty() && in_range_vector_with_case( ch ) )
                        || ( m_posixcharson && iswctype( ch, m_posixcharson ) ) ) )
                 || ( extended_check_wide_with_case( ch ) )
               );
    }

    // Note overloading based on parameter
    bool in_wide_no_case( wchar_t ch ) const
    {
        // use range_match_type to see if this character is within one of the
        // ranges stored in m_rgranges.
        return m_fcompliment !=
               (
                    ( ( UCHAR_MAX >= ch ) ?
                      ( m_ascii_bitvector[ static_cast<unsigned char>( ch ) ] ) :
                      (    ( ! m_range_vector.empty() && in_range_vector_no_case( ch ) )
                        || ( m_posixcharson && iswctype( ch, m_posixcharson ) ) ) )
                 || ( extended_check_wide_no_case( ch ) )
               );
    }

    bool in( char ch, true_t ) const
    {
        return in_narrow( ch );
    }

    bool in( char ch, false_t ) const
    {

        return in_narrow( ch );
    }

    bool in( wchar_t ch, true_t ) const
    {
        return in_wide_with_case( ch );
    }

    bool in( wchar_t ch, false_t ) const
    {
        return in_wide_no_case( ch );
    }

private:
    charset_t & operator=( charset_t const & that );
    charset_t( charset_t const & that );
};

// Intrinsic character sets are allocated on the heap with the standard allocator.
// They are either the built-in character sets, or the user-defined ones.
struct charset : public charset_t< std::allocator<range_type>,
                                   std::allocator<wctype_t>,
                                   std::allocator<charset const *> >
{
    charset()
    {
    }
private:
    charset( charset const & );
    charset & operator=( charset const & );
};

// charset is no longer an incomplete type so we now
// know how to destroy one. free_charset() is used in syntax2.h
REGEXPR_H_INLINE void free_charset( charset const * pcharset )
{
    delete pcharset;
}

// Custom character sets are the ones that appear in patterns between
// square brackets.  They are allocated in a regex_arena to speed up
// pattern compilation and to make rpattern clean-up faster.
struct custom_charset : public charset_t< REGEX_ALLOCATOR< range_type >,
                                          REGEX_ALLOCATOR< wctype_t >,
                                          REGEX_ALLOCATOR< charset const * > >
{
    typedef REGEX_ALLOCATOR< range_type > A1;
    typedef REGEX_ALLOCATOR< wctype_t > A2;
    typedef REGEX_ALLOCATOR< charset const * > A3;

    static void * operator new( size_t size, regex_arena & arena )
    {
        return arena.allocate( size );
    }
    static void operator delete( void *, regex_arena & ) {}
    static void operator delete( void * ) {}

    custom_charset( regex_arena & arena )
        : charset_t<A1, A2, A3>( MAKE_ALLOCATOR(range_type,arena),
                                 MAKE_ALLOCATOR(wctype_t,arena),
                                 MAKE_ALLOCATOR(charset const*,arena) )
    {
    }
private:
    custom_charset( custom_charset const & );
    custom_charset & operator=( custom_charset const & );
};

template< typename CH >
class intrinsic_charsets
{
    struct intrinsic_charset : public charset
    {
        intrinsic_charset( bool fcompliment, wctype_t desc, char const * sz )
        {
            reset( fcompliment, desc, sz );
        }
        void reset( bool fcompliment, wctype_t desc, char const * sz )
        {
            clear();
            m_fcompliment = fcompliment;
            m_fskip_extended_check = true;
            _set( desc, type2type<CH>() );
            for( ; *sz; ++sz )
                m_ascii_bitvector.set( static_cast<unsigned char>( *sz ) );
        }
    protected:
        void _set( wctype_t desc, type2type<char> )
        {
            m_ascii_bitvector.zero();
            for( unsigned int i=0; i<=UCHAR_MAX; ++i )
                if( _isctype( i, desc ) )
                    m_ascii_bitvector.set( static_cast<unsigned char>( i ) );
        }
        void _set( wctype_t desc, type2type<wchar_t> )
        {
            _set( desc, type2type<char>() );
            m_posixcharson = desc;
        }
    private:
        intrinsic_charset( intrinsic_charset const & );
        intrinsic_charset & operator=( intrinsic_charset const & );
    };

    static intrinsic_charset & _get_word_charset()
    {
        static intrinsic_charset s_word_charset( false, _ALPHA|_DIGIT, "_" );
        return s_word_charset;
    }
    static intrinsic_charset & _get_digit_charset()
    {
        static intrinsic_charset s_digit_charset( false, _DIGIT, "" );
        return s_digit_charset;
    }
    static intrinsic_charset & _get_space_charset()
    {
        static intrinsic_charset s_space_charset( false, _SPACE, "" );
        return s_space_charset;
    }
    static intrinsic_charset & _get_not_word_charset()
    {
        static intrinsic_charset s_not_word_charset( true, _ALPHA|_DIGIT, "_" );

        return s_not_word_charset;
    }
    static intrinsic_charset & _get_not_digit_charset()
    {
        static intrinsic_charset s_not_digit_charset( true, _DIGIT, "" );
        return s_not_digit_charset;
    }
    static intrinsic_charset & _get_not_space_charset()
    {
        static intrinsic_charset s_not_space_charset( true, _SPACE, "" );
        return s_not_space_charset;
    }
public:
    static charset const & get_word_charset()
    {
        return _get_word_charset();
    }
    static charset const & get_digit_charset()
    {
        return _get_digit_charset();
    }
    static charset const & get_space_charset()
    {
        return _get_space_charset();
    }
    static charset const & get_not_word_charset()
    {
        return _get_not_word_charset();
    }
    static charset const & get_not_digit_charset()
    {
        return _get_not_digit_charset();
    }
    static charset const & get_not_space_charset()
    {
        return _get_not_space_charset();
    }
    static void reset()
    {
        _get_word_charset().reset( false, _ALPHA|_DIGIT, "_" );
        _get_digit_charset().reset( false, _DIGIT, "" );
        _get_space_charset().reset( false, _SPACE, "" );
        _get_not_word_charset().reset( true, _ALPHA|_DIGIT, "_" );
        _get_not_digit_charset().reset( true, _DIGIT, "" );
        _get_not_space_charset().reset( true, _SPACE, "" );
    }
};

//
// Operator implementations
//

// Evaluates the beginning-of-string condition
template< typename CSTRINGS >
struct bos_t
{
    template< typename CI >
    static bool eval( match_param<CI> const & param, CI iter )
    {
        return param.ibegin == iter;
    }
    template< typename U > struct rebind { typedef bos_t<U> other; };
};

// Find the beginning of a line, either beginning of a string, or the character
// immediately following a newline
template< typename CSTRINGS >
struct bol_t
{
    template< typename CI >
    static bool eval( match_param<CI> const & param, CI iter )
    {
        typedef typename std::iterator_traits<CI>::value_type CH;
        typedef std::char_traits<CH> traits_type;

        return param.ibegin == iter || traits_type::eq( REGEX_CHAR(CH,'\n'), *--iter );
    }
    template< typename U > struct rebind { typedef bol_t<U> other; };
};

// Evaluates end-of-string condition for string's
template< typename CSTRINGS >
struct eos_t
{
    template< typename CI >
    static bool eval( match_param<CI> const & param, CI iter )
    {
        return param.istop == iter;
    }
    template< typename U > struct rebind { typedef eos_t<U> other; };
};
template<>
struct eos_t<true_t>
{
    template< typename CI >
    static bool eval( match_param<CI> const &, CI iter )
    {
        typedef typename std::iterator_traits<CI>::value_type CH;
        typedef std::char_traits<CH> traits_type;

        return traits_type::eq( REGEX_CHAR(CH,'\0'), *iter );
    }
    template< typename U > struct rebind { typedef eos_t<U> other; };
};

// Evaluates end-of-line conditions, either the end of the string, or a
// newline character.
template< typename CSTRINGS >
struct eol_t
{
    template< typename CI >
    static bool eval( match_param<CI> const & param, CI iter )
    {
        typedef typename std::iterator_traits<CI>::value_type CH;
        typedef std::char_traits<CH> traits_type;

        return param.istop == iter
            || traits_type::eq( REGEX_CHAR(CH,'\n'), *iter );
    }
    template< typename U > struct rebind { typedef eol_t<U> other; };
};
template<>
struct eol_t<true_t>
{
    template< typename CI >
    static bool eval( match_param<CI> const &, CI iter )
    {
        typedef typename std::iterator_traits<CI>::value_type CH;
        typedef std::char_traits<CH> traits_type;

        return traits_type::eq( REGEX_CHAR(CH,'\0'), *iter )
            || traits_type::eq( REGEX_CHAR(CH,'\n'), *iter );
    }
    template< typename U > struct rebind { typedef eol_t<U> other; };
};

// Evaluates perl's end-of-string conditions, either the end of the string, or a
// newline character followed by end of string. ( Only used by $ and /Z assertions )
template< typename CSTRINGS >
struct peos_t
{
    template< typename CI >
    static bool eval( match_param<CI> const & param, CI iter )
    {
        typedef typename std::iterator_traits<CI>::value_type CH;
        typedef std::char_traits<CH> traits_type;

        return param.istop == iter
            || ( traits_type::eq( REGEX_CHAR(CH,'\n'), *iter ) && param.istop == ++iter );
    }
    template< typename U > struct rebind { typedef peos_t<U> other; };
};
template<>
struct peos_t<true_t>
{
    template< typename CI >
    static bool eval( match_param<CI> const &, CI iter )
    {
        typedef typename std::iterator_traits<CI>::value_type CH;
        typedef std::char_traits<CH> traits_type;

        return traits_type::eq( REGEX_CHAR(CH,'\0'), *iter )
            || ( traits_type::eq( REGEX_CHAR(CH,'\n'), *iter ) 
                && traits_type::eq( REGEX_CHAR(CH,'\0'), *++iter ) );
    }
    template< typename U > struct rebind { typedef peos_t<U> other; };
};

// compare two characters, case-sensitive
template< typename CH >
struct ch_neq_t
{
    typedef CH char_type;
    typedef std::char_traits<char_type> traits_type;
    
    static bool eval( register CH ch1, register CH ch2 )
    {
        return ! traits_type::eq( ch1, ch2 );
    }
};

// Compare two characters, disregarding case
template< typename CH >
struct ch_neq_nocase_t
{
    typedef CH char_type;
    typedef std::char_traits<char_type> traits_type;
    
    static bool eval( register CH ch1, register CH ch2 )
    {
        return ! traits_type::eq( regex_toupper( ch1 ), regex_toupper( ch2 ) );
    }
};

//
// helper functions for dealing with widths.
//
inline size_t width_add( size_t a, size_t b )
{
    return ( size_t( -1 ) == a || size_t( -1 ) == b ? size_t( -1 ) : a + b );
}

inline size_t width_mult( size_t a, size_t b )
{
    if( 0 == a || 0 == b )
        return 0;

    if( size_t( -1 ) == a || size_t( -1 ) == b )
        return size_t( -1 );

    return a * b;
}

inline bool operator==( width_type const & rhs, width_type const & lhs )
{
    return ( rhs.m_min == lhs.m_min && rhs.m_max == lhs.m_max );
}

inline bool operator!=( width_type const & rhs, width_type const & lhs )
{
    return ( rhs.m_min != lhs.m_min || rhs.m_max != lhs.m_max );

}

inline width_type operator+( width_type const & rhs, width_type const & lhs )
{
    width_type width = { width_add( rhs.m_min, lhs.m_min ), width_add( rhs.m_max, lhs.m_max ) };
    return width;
}

inline width_type operator*( width_type const & rhs, width_type const & lhs )
{
    width_type width = { width_mult( rhs.m_min, lhs.m_min ), width_mult( rhs.m_max, lhs.m_max ) };
    return width;
}

inline width_type & operator+=( width_type & rhs, width_type const & lhs )
{
    rhs.m_min = width_add( rhs.m_min, lhs.m_min );
    rhs.m_max = width_add( rhs.m_max, lhs.m_max );
    return rhs;
}

inline width_type & operator*=( width_type & rhs, width_type const & lhs )
{
    rhs.m_min = width_mult( rhs.m_min, lhs.m_min );
    rhs.m_max = width_mult( rhs.m_max, lhs.m_max );
    return rhs;
}

width_type const zero_width = { 0, 0 };
width_type const worst_width = { 0, size_t( -1 ) };

template< typename CI >
struct width_param
{
    int cookie;
    width_type const total_width;
    std::vector<match_group_base<CI>*> & rggroups;
    std::list<size_t> const & invisible_groups;

    bool first_pass() const { return uninit_width() == total_width; }

    width_param(
        int cookie,
        width_type const & total_width,
        std::vector<match_group_base<CI>*> & rggroups,
        std::list<size_t> const & invisible_groups )
        : cookie( cookie ),
          total_width( total_width ),
          rggroups( rggroups ),
          invisible_groups( invisible_groups )
    {
    }
private:
    width_param & operator=( width_param const & );
};

// --------------------------------------------------------------------------
//
// Class:       sub_expr
//
// Description: patterns are "compiled" into a directed graph of sub_expr
//              structs.  Matching is accomplished by traversing this graph.
//
// Methods:     sub_expr             - construct a sub_expr
//              recursive_match_this_ - does this sub_expr match at the given location
//              width_this           - what is the width of this sub_expr
//              ~sub_expr            - recursively delete the sub_expr graph
//              next                 - pointer to the next node in the graph
//              next                 - pointer to the next node in the graph
//              recursive_match_next_ - match the rest of the graph
//              recursive_match_all_  - recursive_match_this_ and recursive_match_next_
//              is_assertion         - true if this sub_expr is a zero-width assertion
//              get_width            - find the width of the graph at this sub_expr
//
// Members:     m_pnext      - pointer to the next node in the graph
//
// History:     8/14/2000 - ericne - Created
//
// --------------------------------------------------------------------------
template< typename CI >
class sub_expr : public sub_expr_base<CI>
{
    sub_expr * m_pnext;

protected:
    // Only derived classes can instantiate sub_expr's
    sub_expr() 
        : m_pnext( NULL )
    {
    }

public:
    typedef CI const_iterator;
    typedef typename std::iterator_traits<CI>::value_type char_type;
    typedef std::char_traits<char_type> traits_type;

    virtual ~sub_expr()
    {
        delete m_pnext;
    }

    sub_expr ** pnext()
    {
        return & m_pnext;
    }

    sub_expr const * next() const
    {
        return m_pnext;
    }

    virtual sub_expr<CI> * quantify( size_t, size_t, bool, regex_arena & )
    {
        throw bad_regexpr( "sub-expression cannot be quantified" );
    }

    // Match this object and all subsequent objects
    // If recursive_match_all_ returns false, it must not change any of param's state
    virtual bool recursive_match_all_( match_param<CI> & param, CI icur ) const
    {
        return ( recursive_match_this_( param, icur ) && recursive_match_next_( param, icur, false_t() ) );
    }

    virtual bool recursive_match_all_c( match_param<CI> & param, CI icur ) const // for C-style strings
    {
        return ( recursive_match_this_c( param, icur ) && recursive_match_next_( param, icur, true_t() ) );
    }

    // match this object only
    virtual bool recursive_match_this_( match_param<CI> &, CI & ) const
    {
        return true;
    }

    virtual bool recursive_match_this_c( match_param<CI> &, CI & ) const // for C-style strings
    {
        return true;
    }

    // Match all subsequent objects
    template< typename CSTRINGS >
    bool recursive_match_next_( match_param<CI> & param, CI icur, CSTRINGS ) const
    {
        return ( m_pnext ) ? m_pnext->recursive_match_all_( param, icur, CSTRINGS() ) :
            ! ( param.no0len && param.istart == icur );
    }

    virtual bool iterative_match_this_( match_param<CI> & param ) const
    {
        param.next = next();
        return true;
    }

    virtual bool iterative_match_this_c( match_param<CI> & param ) const // for C-style strings
    {
        param.next = next();
        return true;
    }

    virtual bool iterative_rematch_this_( match_param<CI> & ) const
    {
        return false;
    }

    virtual bool iterative_rematch_this_c( match_param<CI> & ) const // for C-style strings
    {
        return false;
    }

    virtual bool is_assertion() const
    {
        return false;
    }

    width_type get_width( width_param<CI> & param )
    {
        width_type temp_width = width_this( param );

        if( m_pnext )
            temp_width += m_pnext->get_width( param );

        return temp_width;
    }

    virtual width_type width_this( width_param<CI> & ) = 0;

    template< typename CSTRINGS >
    bool iterative_match_this_( match_param<CI> & param, CSTRINGS ) const
        { return sub_expr_base<CI>::iterative_match_this_( param, CSTRINGS() ); }

    template< typename CSTRINGS >
    bool iterative_rematch_this_( match_param<CI> & param, CSTRINGS ) const
        { return sub_expr_base<CI>::iterative_rematch_this_( param, CSTRINGS() ); }

    template< typename CSTRINGS >
    bool recursive_match_all_( match_param<CI> & param, CI icur, CSTRINGS ) const
        { return sub_expr_base<CI>::recursive_match_all_( param, icur, CSTRINGS() ); }
};

// Base class for sub-expressions which are zero-width
// ( i.e., assertions eat no characters during matching )
// Assertions cannot be quantified.
template< typename CI >
class assertion : public sub_expr<CI>
{
public:
    typedef typename sub_expr<CI>::const_iterator const_iterator;
    typedef typename sub_expr<CI>::char_type      char_type;

    virtual bool is_assertion() const
        { return true; }
    virtual width_type width_this( width_param<CI> & )
        { return zero_width; }
};

template< typename CI, typename OP >
class assert_op : public assertion<CI>
{
public:
    virtual bool recursive_match_all_( match_param<CI> & param, CI icur ) const
    {
        return ( assert_op::recursive_match_this_( param, icur ) && recursive_match_next_( param, icur, false_t() ) );
    }
    virtual bool recursive_match_all_c( match_param<CI> & param, CI icur ) const
    {
        return ( assert_op::recursive_match_this_c( param, icur ) && recursive_match_next_( param, icur, true_t() ) );
    }
    virtual bool recursive_match_this_( match_param<CI> & param, CI & icur ) const
    {
        typedef typename OP::template rebind<false_t>::other op_t;
        return op_t::eval( param, icur );
    }
    virtual bool recursive_match_this_c( match_param<CI> & param, CI & icur ) const
    {
        typedef typename OP::template rebind<true_t>::other op_t;
        return op_t::eval( param, icur );
    }
    virtual bool iterative_match_this_( match_param<CI> & param ) const
    {
        param.next = next();
        typedef typename OP::template rebind<false_t>::other op_t;
        return op_t::eval( param, param.icur );
    }
    virtual bool iterative_match_this_c( match_param<CI> & param ) const
    {
        param.next = next();
        typedef typename OP::template rebind<true_t>::other op_t;
        return op_t::eval( param, param.icur );
    }
};

template< typename CI >
inline assertion<CI> * create_bos( REGEX_FLAGS, regex_arena & arena )
{
    return new( arena ) assert_op<CI, bos_t<true_t> >();
}

template< typename CI >
inline assertion<CI> * create_eos( REGEX_FLAGS, regex_arena & arena )
{
    return new( arena ) assert_op<CI, peos_t<true_t> >();
}

template< typename CI >
inline assertion<CI> * create_eoz( REGEX_FLAGS, regex_arena & arena )
{
    return new( arena ) assert_op<CI, eos_t<true_t> >();
}

template< typename CI >
inline assertion<CI> * create_bol( REGEX_FLAGS flags, regex_arena & arena )
{
    switch( MULTILINE & flags )
    {
    case 0:
        return new( arena ) assert_op<CI, bos_t<true_t> >();
    case MULTILINE:
        return new( arena ) assert_op<CI, bol_t<true_t> >();
    default:
        __assume( 0 ); // tells the compiler that this is unreachable
    }
}

template< typename CI >
inline assertion<CI> * create_eol( REGEX_FLAGS flags, regex_arena & arena )
{
    switch( MULTILINE & flags )
    {
    case 0:
        return new( arena ) assert_op<CI, peos_t<true_t> >();
    case MULTILINE:
        return new( arena ) assert_op<CI, eol_t<true_t> >();
    default:
        __assume( 0 ); // tells the compiler that this is unreachable
    }
}

template< typename CI, typename SUB_EXPR = sub_expr<CI> >
class match_wrapper : public sub_expr<CI>
{
    match_wrapper & operator=( match_wrapper const & );
public:
    match_wrapper( SUB_EXPR * psub )
        : m_psub( psub )
    {
    }

    virtual ~match_wrapper()
    {
        _cleanup();
    }

    virtual width_type width_this( width_param<CI> & param )
    {
        return m_psub->width_this( param );
    }

protected:
    void _cleanup()
    {
        delete m_psub;
        m_psub = NULL;
    }

    SUB_EXPR * m_psub;
};

template< typename CI, typename SUB_EXPR = sub_expr<CI> >
class match_quantifier : public match_wrapper<CI, SUB_EXPR>
{
    match_quantifier & operator=( match_quantifier const & );
public:
    match_quantifier( SUB_EXPR * psub, size_t lbound, size_t ubound )
        : match_wrapper<CI, SUB_EXPR>( psub ), m_lbound( lbound ), m_ubound( ubound )
    {
    }

    virtual width_type width_this( width_param<CI> & param )
    {
        width_type this_width = match_wrapper<CI, SUB_EXPR>::width_this( param );
        width_type quant_width = { m_lbound, m_ubound };
        return this_width * quant_width;
    }

protected:
    size_t const m_lbound;
    size_t const m_ubound;
};

template< typename CI, typename SUB_EXPR >
class atom_quantifier : public match_quantifier<CI, SUB_EXPR>
{
    atom_quantifier & operator=( atom_quantifier const & );
public:
    atom_quantifier( SUB_EXPR * psub, size_t lbound, size_t ubound )
        : match_quantifier<CI, SUB_EXPR>( psub, lbound, ubound ) 
    {
    }
protected:
    void _push_frame( unsafe_stack * pstack, CI curr, size_t count ) const
    {
        std::pair<CI, size_t> p( curr, count );
        pstack->push( p );
    }

    void _pop_frame( match_param<CI> & param ) const
    {
        std::pair<CI, size_t> p;
        param.pstack->pop( p );
        param.icur = p.first;
    }
};

//template< typename CSTRINGS >
//struct inliner
//{
//    template< typename SUB_EXPR, typename CI >
//    __forceinline static bool iterative_match_this_( SUB_EXPR const * psub, match_param<CI> & param )
//    {
//        return psub->SUB_EXPR::iterative_match_this_( param );
//    }
//};
//template<>
//struct inliner<true_t>
//{
//    template< typename SUB_EXPR, typename CI >
//    __forceinline static bool iterative_match_this_( SUB_EXPR const * psub, match_param<CI> & param )
//    {
//        return psub->SUB_EXPR::iterative_match_this_c( param );
//    }
//};

template< typename CI, typename SUB_EXPR >
class max_atom_quantifier : public atom_quantifier<CI, SUB_EXPR>
{
    max_atom_quantifier & operator=( max_atom_quantifier const & );

public:
    max_atom_quantifier( SUB_EXPR * psub, size_t lbound, size_t ubound )
        : atom_quantifier<CI, SUB_EXPR>( psub, lbound, ubound )
    {
    }

    // Why a macro instead of a template, you ask?  Performance.  Due to a known
    // bug in the VC7 inline heuristic, I cannot get VC7 to inline the calls to 
    // m_psub methods unless I use these macros.  And the performance win is 
    // nothing to sneeze at. It's on the order of a 25% speed up to use a macro 
    // here instead of a template.
#define DECLARE_RECURSIVE_MATCH_ALL(cstrings,ext)                                                           \
    virtual bool recursive_match_all ## ext( match_param<CI> & param, CI icur ) const                       \
    {                                                                                                       \
        /* In an ideal world, istart and cdiff would be members of a union */                               \
        /* to conserve stack, but I don't know if CI is a POD type or not. */                               \
        CI        istart   = icur;                                                                          \
        ptrdiff_t cdiff    = 0; /* must be a signed integral type */                                        \
        size_t    cmatches = 0;                                                                             \
        /* greedily match as much as we can*/                                                               \
        if( m_ubound && m_psub->SUB_EXPR::recursive_match_this ## ext( param, icur ) )                      \
        {                                                                                                   \
            if( 0 == ( cdiff = -std::distance( istart, icur ) ) )                                           \
                return recursive_match_next_( param, icur, cstrings() );                                    \
            while( ++cmatches < m_ubound && m_psub->SUB_EXPR::recursive_match_this ## ext( param, icur ) ); \
        }                                                                                                   \
        if( m_lbound > cmatches )                                                                           \
            return false;                                                                                   \
        /* try matching the rest of the pattern, and back off if necessary */                               \
        for( ; ; --cmatches, std::advance( icur, ( int ) cdiff ) )                                          \
        {                                                                                                   \
            if( recursive_match_next_( param, icur, cstrings() ) )                                          \
                return true;                                                                                \
            if( m_lbound == cmatches )                                                                      \
                return false;                                                                               \
        }                                                                                                   \
    }

#define DECLARE_ITERATIVE_MATCH_THIS(ext)                                                                   \
    virtual bool iterative_match_this ## ext( match_param<CI> & param ) const                               \
    {                                                                                                       \
        CI istart = param.icur;                                                                             \
        size_t cmatches = 0;                                                                                \
        if( m_ubound && m_psub->SUB_EXPR::iterative_match_this ## ext( param ) )                            \
        {                                                                                                   \
            if( 0 == std::distance( istart, param.icur ) )                                                  \
            {                                                                                               \
                cmatches = m_lbound;                                                                        \
            }                                                                                               \
            else                                                                                            \
            {                                                                                               \
                while( ++cmatches < m_ubound && m_psub->SUB_EXPR::iterative_match_this ## ext( param ) );   \
            }                                                                                               \
        }                                                                                                   \
        if( cmatches >= m_lbound )                                                                          \
        {                                                                                                   \
            _push_frame( param.pstack, istart, cmatches );                                                  \
            param.next = next();                                                                            \
            return true;                                                                                    \
        }                                                                                                   \
        param.icur = istart;                                                                                \
        return false;                                                                                       \
    }

#define DECLARE_ITERATIVE_REMATCH_THIS(ext)                                                                 \
    virtual bool iterative_rematch_this ## ext( match_param<CI> & param ) const                             \
    {                                                                                                       \
        size_t & cmatches = param.pstack->top( static_init<std::pair<CI, size_t> >::value ).second;         \
        if( m_lbound != cmatches )                                                                          \
        {                                                                                                   \
            --cmatches;                                                                                     \
            m_psub->SUB_EXPR::iterative_rematch_this ## ext( param );                                       \
            param.next = next();                                                                            \
            return true;                                                                                    \
        }                                                                                                   \
        _pop_frame( param );                                                                                \
        return false;                                                                                       \
    }

    DECLARE_RECURSIVE_MATCH_ALL(false_t,_)
    DECLARE_RECURSIVE_MATCH_ALL(true_t,_c)
    DECLARE_ITERATIVE_MATCH_THIS(_)
    DECLARE_ITERATIVE_MATCH_THIS(_c)
    DECLARE_ITERATIVE_REMATCH_THIS(_)
    DECLARE_ITERATIVE_REMATCH_THIS(_c)

    //template< typename CSTRINGS >
    //__forceinline bool _iterative_match_this( match_param<CI> & param ) const
    //{
    //    CI istart = param.icur;
    //    size_t cmatches = 0;
    //    if( m_ubound && inliner<CSTRINGS>::iterative_match_this_( m_psub, param ) )
    //    {
    //        if( 0 == std::distance( istart, param.icur ) )
    //        {
    //            cmatches = m_lbound;
    //        }
    //        else
    //        {
    //            while( ++cmatches < m_ubound && inliner<CSTRINGS>::iterative_match_this_( m_psub, param ) );
    //        }
    //    }
    //    if( cmatches >= m_lbound )
    //    {
    //        _push_frame( param.pstack, istart, cmatches );
    //        param.next = next();
    //        return true;
    //    }
    //    param.icur = istart;
    //    return false;
    //}

    //virtual bool iterative_match_this_( match_param<CI> & param ) const
    //{
    //    return _iterative_match_this<false_t>( param );
    //}
    //virtual bool iterative_match_this_c( match_param<CI> & param ) const
    //{
    //    return _iterative_match_this<true_t>( param );
    //}

#undef DECLARE_RECURSIVE_MATCH_ALL
#undef DECLARE_ITERATIVE_MATCH_THIS
#undef DECLARE_ITERATIVE_REMATCH_THIS
};

template< typename CI, typename SUB_EXPR >
class min_atom_quantifier : public atom_quantifier<CI, SUB_EXPR>
{
    min_atom_quantifier & operator=( min_atom_quantifier const & );
    
public:
    min_atom_quantifier( SUB_EXPR * psub, size_t lbound, size_t ubound )
        : atom_quantifier<CI, SUB_EXPR>( psub, lbound, ubound )
    {
    }

    // Why a macro instead of a template, you ask?  Performance.  Due to a known
    // bug in the VC7 inline heuristic, I cannot get VC7 to inline the calls to 
    // m_psub methods unless I use these macros.  And the performance win is 
    // nothing to sneeze at. It's on the order of a 25% speed up to use a macro 
    // here instead of a template.
#define DECLARE_RECURSIVE_MATCH_ALL(cstrings,ext)                                                           \
    virtual bool recursive_match_all ## ext( match_param<CI> & param, CI icur ) const                       \
    {                                                                                                       \
        CI     icur_tmp = icur;                                                                             \
        size_t cmatches = 0;                                                                                \
        if( m_psub->SUB_EXPR::recursive_match_this ## ext( param, icur_tmp ) )                              \
        {                                                                                                   \
            if( icur_tmp == icur )                                                                          \
                return recursive_match_next_( param, icur, cstrings() );                                    \
            if( m_lbound )                                                                                  \
            {                                                                                               \
                icur = icur_tmp;                                                                            \
                ++cmatches;                                                                                 \
            }                                                                                               \
            for( ; cmatches < m_lbound; ++cmatches )                                                        \
            {                                                                                               \
                if( ! m_psub->SUB_EXPR::recursive_match_this ## ext( param, icur ) )                        \
                    return false;                                                                           \
            }                                                                                               \
        }                                                                                                   \
        else if( m_lbound )                                                                                 \
        {                                                                                                   \
            return false;                                                                                   \
        }                                                                                                   \
        do                                                                                                  \
        {                                                                                                   \
            if( recursive_match_next_( param, icur, cstrings() ) )                                          \
                return true;                                                                                \
        }                                                                                                   \
        while( cmatches < m_ubound &&                                                                       \
             ( ++cmatches, m_psub->SUB_EXPR::recursive_match_this ## ext( param, icur ) ) );                \
        return false;                                                                                       \
    }

#define DECLARE_ITERATIVE_MATCH_THIS(ext)                                                                   \
    virtual bool iterative_match_this ## ext( match_param<CI> & param ) const                               \
    {                                                                                                       \
        CI istart = param.icur;                                                                             \
        size_t cmatches = 0;                                                                                \
        if( m_psub->SUB_EXPR::iterative_match_this ## ext( param ) )                                        \
        {                                                                                                   \
            if( 0 == std::distance( istart, param.icur ) )                                                  \
            {                                                                                               \
                cmatches = m_ubound;                                                                        \
            }                                                                                               \
            else if( m_lbound )                                                                             \
            {                                                                                               \
                for( ++cmatches; cmatches < m_lbound; ++cmatches )                                          \
                {                                                                                           \
                    if( ! m_psub->SUB_EXPR::iterative_match_this ## ext( param ) )                          \
                    {                                                                                       \
                        param.icur = istart;                                                                \
                        return false;                                                                       \
                    }                                                                                       \
                }                                                                                           \
            }                                                                                               \
            else                                                                                            \
            {                                                                                               \
                param.icur = istart;                                                                        \
            }                                                                                               \
        }                                                                                                   \
        else if( m_lbound )                                                                                 \
        {                                                                                                   \
            return false;                                                                                   \
        }                                                                                                   \
        _push_frame( param.pstack, istart, cmatches );                                                      \
        param.next = next();                                                                                \
        return true;                                                                                        \
    }

#define DECLARE_ITERATIVE_REMATCH_THIS(ext)                                                                 \
    virtual bool iterative_rematch_this ## ext( match_param<CI> & param ) const                             \
    {                                                                                                       \
        size_t & cmatches = param.pstack->top( static_init<std::pair<CI, size_t> >::value ).second;         \
        if( cmatches == m_ubound || ! m_psub->SUB_EXPR::iterative_match_this ## ext( param ) )              \
        {                                                                                                   \
            _pop_frame( param );                                                                            \
            return false;                                                                                   \
        }                                                                                                   \
        ++cmatches;                                                                                         \
        param.next = next();                                                                                \
        return true;                                                                                        \
    }

    DECLARE_RECURSIVE_MATCH_ALL(false_t,_)
    DECLARE_RECURSIVE_MATCH_ALL(true_t,_c)
    DECLARE_ITERATIVE_MATCH_THIS(_)
    DECLARE_ITERATIVE_MATCH_THIS(_c)
    DECLARE_ITERATIVE_REMATCH_THIS(_)
    DECLARE_ITERATIVE_REMATCH_THIS(_c)

#undef DECLARE_RECURSIVE_MATCH_ALL
#undef DECLARE_ITERATIVE_MATCH_THIS
#undef DECLARE_ITERATIVE_REMATCH_THIS
};

template< typename CI >
class match_char : public sub_expr<CI>
{
    match_char & operator=( match_char const & );
public:
    typedef typename sub_expr<CI>::char_type char_type;

    match_char( char_type const & ch )
        : m_ch( ch ) 
    {
    }

    virtual width_type width_this( width_param<CI> & )
    {
        width_type width = { 1, 1 };
        return width;
    }
    virtual bool iterative_rematch_this_( match_param<CI> & param ) const
    {
        --param.icur;
        return false;
    }
    virtual bool iterative_rematch_this_c( match_param<CI> & param ) const
    {
        --param.icur;
        return false;
    }
protected:
    char_type const m_ch;
};

template< typename CI >
class match_char_t : public match_char<CI>
{
    match_char_t & operator=( match_char_t const & );

public:
    typedef typename match_char<CI>::char_type char_type;

    match_char_t( char_type ch )
        : match_char<CI>( ch ) 
    {
    }

    virtual sub_expr<CI> * quantify( size_t lbound, size_t ubound, bool greedy, regex_arena & arena )
    {
        if( greedy )
            return new( arena ) max_atom_quantifier<CI, match_char_t<CI> >( this, lbound, ubound );
        else
            return new( arena ) min_atom_quantifier<CI, match_char_t<CI> >( this, lbound, ubound );
    }
    virtual bool recursive_match_all_( match_param<CI> & param, CI icur ) const
    {
        return ( match_char_t::recursive_match_this_( param, icur ) && recursive_match_next_( param, icur, false_t() ) );
    }
    virtual bool recursive_match_all_c( match_param<CI> & param, CI icur ) const
    {
        return ( match_char_t::recursive_match_this_c( param, icur ) && recursive_match_next_( param, icur, true_t() ) );
    }
    virtual bool recursive_match_this_( match_param<CI> & param, CI & icur ) const
    {
        return _do_match_this( param, icur, false_t() );
    }
    virtual bool recursive_match_this_c( match_param<CI> & param, CI & icur ) const
    {
        return _do_match_this( param, icur, true_t() );
    }
    virtual bool iterative_match_this_( match_param<CI> & param ) const
    {
        param.next = next();
        return _do_match_this( param, param.icur, false_t() );
    }
    virtual bool iterative_match_this_c( match_param<CI> & param ) const
    {
        param.next = next();
        return _do_match_this( param, param.icur, true_t() );
    }
private:
    template< typename CSTRINGS >
    bool _do_match_this( match_param<CI> & param, CI & icur, CSTRINGS ) const
    {
        if( eos_t<CSTRINGS>::eval( param, icur ) || ! traits_type::eq( *icur, m_ch ) )
            return false;
        ++icur;
        return true;
    }
};

template< typename CI >
class match_char_nocase_t : public match_char<CI>
{
    match_char_nocase_t & operator=( match_char_nocase_t const & );
public:
    typedef typename match_char<CI>::char_type char_type;

    match_char_nocase_t( char_type lower, char_type upper )
        : match_char<CI>( upper ), m_ch_lower( lower ) 
    {
    }

    virtual sub_expr<CI> * quantify( size_t lbound, size_t ubound, bool greedy, regex_arena & arena )
    {
        if( greedy )
            return new( arena ) max_atom_quantifier<CI, match_char_nocase_t<CI> >( this, lbound, ubound );
        else
            return new( arena ) min_atom_quantifier<CI, match_char_nocase_t<CI> >( this, lbound, ubound );
    }
    virtual bool recursive_match_all_( match_param<CI> & param, CI icur ) const
    {
        return ( match_char_nocase_t::recursive_match_this_( param, icur ) && recursive_match_next_( param, icur, false_t() ) );
    }
    virtual bool recursive_match_all_c( match_param<CI> & param, CI icur ) const
    {
        return ( match_char_nocase_t::recursive_match_this_c( param, icur ) && recursive_match_next_( param, icur, true_t() ) );
    }
    virtual bool recursive_match_this_( match_param<CI> & param, CI & icur ) const
    {
        return _do_match_this( param, icur, false_t() );
    }
    virtual bool recursive_match_this_c( match_param<CI> & param, CI & icur ) const
    {
        return _do_match_this( param, icur, true_t() );
    }
    virtual bool iterative_match_this_( match_param<CI> & param ) const
    {
        param.next = next();
        return _do_match_this( param, param.icur, false_t() );
    }
    virtual bool iterative_match_this_c( match_param<CI> & param ) const
    {
        param.next = next();
        return _do_match_this( param, param.icur, true_t() );
    }
private:
    template< typename CSTRINGS >
    bool _do_match_this( match_param<CI> & param, CI & icur, CSTRINGS ) const
    {
        if( eos_t<CSTRINGS>::eval( param, icur ) ||
            ( ! traits_type::eq( *icur, m_ch ) &&
              ! traits_type::eq( *icur, m_ch_lower ) ) )
            return false;
        ++icur;
        return true;
    }

    char_type const  m_ch_lower;
};

template< typename CI >
inline match_char<CI> * create_char(
     typename std::iterator_traits<CI>::value_type ch,
     REGEX_FLAGS flags, regex_arena & arena )
{
    typedef typename std::iterator_traits<CI>::value_type CH;
    typedef std::char_traits<CH> traits_type;

    switch( NOCASE & flags )
    {
    case 0:
        return new( arena ) match_char_t<CI>( ch );

    case NOCASE:
        {
            CH lower = regex_tolower( ch );
            CH upper = regex_toupper( ch );

            if( traits_type::eq( lower, upper ) )
                return new( arena ) match_char_t<CI>( ch );
            else
                return new( arena ) match_char_nocase_t<CI>( lower, upper );
        }
    default:
        __assume( 0 ); // tells the compiler that this is unreachable
    }
}

template< typename CI >
class match_literal : public sub_expr<CI>
{
    match_literal & operator=( match_literal const & );
public:
    typedef typename sub_expr<CI>::char_type      char_type;
    typedef std::basic_string<char_type>          string_type;
    typedef typename string_type::iterator        iterator;
    typedef typename string_type::const_iterator  const_iterator;

    match_literal( const_iterator istart, const_iterator istop )
        : m_istart( istart ), m_istop( istop ), m_dist( std::distance( m_istart, m_istop ) )
    {
    }

    const_iterator const m_istart;
    const_iterator const m_istop;
    ptrdiff_t const m_dist; // must be signed integral type

    virtual width_type width_this( width_param<CI> & )
    {
        width_type width = { ( size_t ) m_dist, ( size_t ) m_dist };
        return width;
    }
    virtual bool iterative_rematch_this_( match_param<CI> & param ) const
    {
        std::advance( param.icur, ( int ) - m_dist );
        return false;
    }
    virtual bool iterative_rematch_this_c( match_param<CI> & param ) const
    {
        std::advance( param.icur, ( int ) - m_dist );
        return false;
    }
};

template< typename CI >
class match_literal_t : public match_literal<CI>
{
    match_literal_t & operator=( match_literal_t const & );
public:
    typedef typename match_literal<CI>::char_type       char_type;
    typedef typename match_literal<CI>::string_type     string_type;
    typedef typename match_literal<CI>::iterator        iterator;
    typedef typename match_literal<CI>::const_iterator  const_iterator;

    match_literal_t( const_iterator istart, const_iterator istop )
        : match_literal<CI>( istart, istop )
    {
    }

    virtual sub_expr<CI> * quantify( size_t lbound, size_t ubound, bool greedy, regex_arena & arena )
    {
        if( greedy )
            return new( arena ) max_atom_quantifier<CI, match_literal_t<CI> >( this, lbound, ubound );
        else
            return new( arena ) min_atom_quantifier<CI, match_literal_t<CI> >( this, lbound, ubound );
    }
    virtual bool recursive_match_all_( match_param<CI> & param, CI icur ) const
    {
        return ( match_literal_t::recursive_match_this_( param, icur ) && recursive_match_next_( param, icur, false_t() ) );
    }
    virtual bool recursive_match_all_c( match_param<CI> & param, CI icur ) const
    {
        return ( match_literal_t::recursive_match_this_c( param, icur ) && recursive_match_next_( param, icur, true_t() ) );
    }
    virtual bool recursive_match_this_( match_param<CI> & param, CI & icur ) const
    {
        return _do_match_this( param, icur, false_t() );
    }
    virtual bool recursive_match_this_c( match_param<CI> & param, CI & icur ) const
    {
        return _do_match_this( param, icur, true_t() );
    }
    virtual bool iterative_match_this_( match_param<CI> & param ) const
    {
        param.next = next();
        return _do_match_this( param, param.icur, false_t() );
    }
    virtual bool iterative_match_this_c( match_param<CI> & param ) const
    {
        param.next = next();
        return _do_match_this( param, param.icur, true_t() );
    }
private:
    template< typename CSTRINGS >
    bool _do_match_this( match_param<CI> & param, CI & icur, CSTRINGS ) const
    {
        CI icur_tmp = icur;
        const_iterator ithis = m_istart;
        for( ; m_istop != ithis; ++icur_tmp, ++ithis )
        {
            if( eos_t<CSTRINGS>::eval( param, icur_tmp ) || ! traits_type::eq( *ithis, *icur_tmp ) )
                return false;
        }
        icur = icur_tmp;
        return true;
    }
};

template< typename CI >
class match_literal_nocase_t : public match_literal<CI>
{
    match_literal_nocase_t & operator=( match_literal_nocase_t const & );

public:
    typedef typename match_literal<CI>::char_type       char_type;
    typedef typename match_literal<CI>::string_type     string_type;
    typedef typename match_literal<CI>::iterator        iterator;
    typedef typename match_literal<CI>::const_iterator  const_iterator;

    match_literal_nocase_t( iterator istart, const_iterator istop, regex_arena & arena )
        : match_literal<CI>( istart, istop ),
          m_szlower( regex_allocator<char_type>( arena ).allocate( ( size_t ) m_dist ) )
    {
        // Copy from istart to m_szlower
        std::copy( m_istart, m_istop, m_szlower );
        // Store the uppercase version of the literal in [ m_istart, m_istop ).
        regex_toupper( istart, istop );
        // Store the lowercase version of the literal in m_strlower.
        regex_tolower( m_szlower, static_cast<char_type const *>( m_szlower + m_dist ) );
    }

    virtual sub_expr<CI> * quantify( size_t lbound, size_t ubound, bool greedy, regex_arena & arena )
    {
        if( greedy )
            return new( arena ) max_atom_quantifier<CI, match_literal_nocase_t<CI> >( this, lbound, ubound );
        else
            return new( arena ) min_atom_quantifier<CI, match_literal_nocase_t<CI> >( this, lbound, ubound );
    }
    virtual bool recursive_match_all_( match_param<CI> & param, CI icur ) const
    {
        return ( match_literal_nocase_t::recursive_match_this_( param, icur ) && recursive_match_next_( param, icur, false_t() ) );
    }
    virtual bool recursive_match_all_c( match_param<CI> & param, CI icur ) const
    {
        return ( match_literal_nocase_t::recursive_match_this_c( param, icur ) && recursive_match_next_( param, icur, true_t() ) );
    }
    virtual bool recursive_match_this_( match_param<CI> & param, CI & icur ) const
    {
        return _do_match_this( param, icur, false_t() );
    }
    virtual bool recursive_match_this_c( match_param<CI> & param, CI & icur ) const
    {
        return _do_match_this( param, icur, true_t() );
    }
    virtual bool iterative_match_this_( match_param<CI> & param ) const
    {
        param.next = next();
        return _do_match_this( param, param.icur, false_t() );
    }
    virtual bool iterative_match_this_c( match_param<CI> & param ) const
    {
        param.next = next();
        return _do_match_this( param, param.icur, true_t() );
    }
private:
    // Allocated from a regex arena. The memory will be cleaned up
    // when the arena is deallocated.
    char_type *const m_szlower;

    template< typename CSTRINGS >
    bool _do_match_this( match_param<CI> & param, CI & icur, CSTRINGS ) const
    {
        CI icur_tmp = icur;
        const_iterator ithisu    = m_istart;  // uppercase
        char_type const * ithisl = m_szlower; // lowercase
        for( ; m_istop != ithisu; ++icur_tmp, ++ithisu, ++ithisl )
        {
            if( eos_t<CSTRINGS>::eval( param, icur_tmp ) ||
                ( ! traits_type::eq( *ithisu, *icur_tmp ) &&
                  ! traits_type::eq( *ithisl, *icur_tmp ) ) )
                return false;
        }
        icur = icur_tmp;
        return true;
    }
};

template< typename CI, typename II1, typename II2 >
inline sub_expr<CI> * create_literal(
     II1 istart, II2 istop, REGEX_FLAGS flags, regex_arena & arena )
{
    // a match_char is faster than a match_literal, so prefer it when
    // the literal is only 1 char wide
    if( 1 == std::distance<II2>( istart, istop ) )
    {
        return create_char<CI>( *istart, flags, arena );
    }

    switch( NOCASE & flags )
    {
    case 0:
        return new( arena ) match_literal_t<CI>( istart, istop );
    case NOCASE:
        return new( arena ) match_literal_nocase_t<CI>( istart, istop, arena );
    default:
        __assume( 0 ); // tells the compiler that this is unreachable
    }
}

template< typename CI >
class match_any : public sub_expr<CI>
{
public:
    virtual width_type width_this( width_param<CI> & )
    {
        width_type width = { 1, 1 };
        return width;
    }
    virtual bool iterative_rematch_this_( match_param<CI> & param ) const
    {
        --param.icur;
        return false;
    }
    virtual bool iterative_rematch_this_c( match_param<CI> & param ) const
    {
        --param.icur;
        return false;
    }
};

template< typename CI, typename EOS >
class match_any_t : public match_any<CI>
{
    template< typename CSTRINGS >
    static bool _do_match_this( match_param<CI> & param, CI & icur, CSTRINGS )
    {
        typedef typename EOS::template rebind<CSTRINGS>::other op_t;
        if( op_t::eval( param, icur ) )
            return false;
        ++icur;
        return true;
    }
public:
    virtual sub_expr<CI> * quantify( size_t lbound, size_t ubound, bool greedy, regex_arena & arena )
    {
        if( greedy )
            return new( arena ) max_atom_quantifier<CI, match_any_t<CI, EOS> >( this, lbound, ubound );
        else
            return new( arena ) min_atom_quantifier<CI, match_any_t<CI, EOS> >( this, lbound, ubound );
    }

    virtual bool recursive_match_all_( match_param<CI> & param, CI icur ) const
    {
        return ( match_any_t::recursive_match_this_( param, icur ) && recursive_match_next_( param, icur, false_t() ) );
    }
    virtual bool recursive_match_all_c( match_param<CI> & param, CI icur ) const
    {
        return ( match_any_t::recursive_match_this_c( param, icur ) && recursive_match_next_( param, icur, true_t() ) );
    }
    virtual bool recursive_match_this_( match_param<CI> & param, CI & icur ) const
    {
        return _do_match_this( param, icur, false_t() );
    }
    virtual bool recursive_match_this_c( match_param<CI> & param, CI & icur ) const
    {
        return _do_match_this( param, icur, true_t() );
    }
    virtual bool iterative_match_this_( match_param<CI> & param ) const
    {
        param.next = next();
        return _do_match_this( param, param.icur, false_t() );
    }
    virtual bool iterative_match_this_c( match_param<CI> & param ) const
    {
        param.next = next();
        return _do_match_this( param, param.icur, true_t() );
    }
};

template< typename CI >
inline match_any<CI> * create_any( REGEX_FLAGS flags, regex_arena & arena )
{
    switch( SINGLELINE & flags )
    {
    case 0:
        return new( arena ) match_any_t<CI, eol_t<true_t> >();
    case SINGLELINE:
        return new( arena ) match_any_t<CI, eos_t<true_t> >();
    default:
        __assume( 0 ); // tells the compiler that this is unreachable
    }
}

template< typename CI >
class match_charset : public sub_expr<CI>
{
public:
    virtual width_type width_this( width_param<CI> & )
    {
        width_type width = { 1, 1 };
        return width;
    }
    virtual bool iterative_rematch_this_( match_param<CI> & param ) const
    {
        --param.icur;
        return false;
    }
    virtual bool iterative_rematch_this_c( match_param<CI> & param ) const
    {
        --param.icur;
        return false;
    }
};

template< typename CI, bool CASE >
class match_charset_t : public match_charset<CI>
{
    charset const & m_cs; // ref to an intrinsic charset ( possibly user-defined )

    match_charset_t & operator=( match_charset_t const & );

    template< typename CSTRINGS >
    bool _do_match_this( match_param<CI> & param, CI & icur, CSTRINGS ) const
    {
        if( eos_t<CSTRINGS>::eval( param, icur ) || ! m_cs.in( *icur, bool2type<CASE>() ) )
            return false;
        ++icur;
        return true;
    }
public:
    match_charset_t( charset const & cs )
         : m_cs( cs ) 
    {
    }

    virtual sub_expr<CI> * quantify( size_t lbound, size_t ubound, bool greedy, regex_arena & arena )
    {
        if( greedy )
            return new( arena ) max_atom_quantifier<CI, match_charset_t<CI, CASE> >( this, lbound, ubound );
        else
            return new( arena ) min_atom_quantifier<CI, match_charset_t<CI, CASE> >( this, lbound, ubound );
    }
    virtual bool recursive_match_all_( match_param<CI> & param, CI icur ) const
    {
        return ( match_charset_t::recursive_match_this_( param, icur ) && recursive_match_next_( param, icur, false_t() ) );
    }
    virtual bool recursive_match_all_c( match_param<CI> & param, CI icur ) const
    {
        return ( match_charset_t::recursive_match_this_c( param, icur ) && recursive_match_next_( param, icur, true_t() ) );
    }
    virtual bool recursive_match_this_( match_param<CI> & param, CI & icur ) const
    {
        return _do_match_this( param, icur, false_t() );
    }
    virtual bool recursive_match_this_c( match_param<CI> & param, CI & icur ) const
    {
        return _do_match_this( param, icur, true_t() );
    }
    virtual bool iterative_match_this_( match_param<CI> & param ) const
    {
        param.next = next();
        return _do_match_this( param, param.icur, false_t() );
    }
    virtual bool iterative_match_this_c( match_param<CI> & param ) const
    {
        param.next = next();
        return _do_match_this( param, param.icur, true_t() );
    }
};

template< typename CI, bool CASE >
class match_custom_charset_t : public match_charset<CI>
{
    std::auto_ptr<custom_charset const> m_pcs; // ptr to a custom charset alloc'ed in regex arena

    template< typename CSTRINGS >
    bool _do_match_this( match_param<CI> & param, CI & icur, CSTRINGS ) const
    {
        if( eos_t<CSTRINGS>::eval( param, icur ) || ! m_pcs->in( *icur, bool2type<CASE>() ) )
            return false;
        ++icur;
        return true;
    }
public:
    match_custom_charset_t( custom_charset const * pcs )
         : m_pcs( pcs ) 
    {
    }

    virtual sub_expr<CI> * quantify( size_t lbound, size_t ubound, bool greedy, regex_arena & arena )
    {
        if( greedy )
            return new( arena ) max_atom_quantifier<CI, match_custom_charset_t<CI, CASE> >( this, lbound, ubound );
        else
            return new( arena ) min_atom_quantifier<CI, match_custom_charset_t<CI, CASE> >( this, lbound, ubound );
    }
    virtual bool recursive_match_all_( match_param<CI> & param, CI icur ) const
    {
        return ( match_custom_charset_t::recursive_match_this_( param, icur ) && recursive_match_next_( param, icur, false_t() ) );
    }
    virtual bool recursive_match_all_c( match_param<CI> & param, CI icur ) const
    {
        return ( match_custom_charset_t::recursive_match_this_c( param, icur ) && recursive_match_next_( param, icur, true_t() ) );
    }
    virtual bool recursive_match_this_( match_param<CI> & param, CI & icur ) const
    {
        return _do_match_this( param, icur, false_t() );
    }
    virtual bool recursive_match_this_c( match_param<CI> & param, CI & icur ) const
    {
        return _do_match_this( param, icur, true_t() );
    }
    virtual bool iterative_match_this_( match_param<CI> & param ) const
    {
        param.next = next();
        return _do_match_this( param, param.icur, false_t() );
    }
    virtual bool iterative_match_this_c( match_param<CI> & param ) const
    {
        param.next = next();
        return _do_match_this( param, param.icur, true_t() );
    }
};

template< typename CI >
inline match_charset<CI> * create_charset(
     charset const & charset,
     REGEX_FLAGS flags, regex_arena & arena )
{
    switch( NOCASE & flags )
    {
    case 0:
        return new( arena ) match_charset_t<CI, true>( charset );
    case NOCASE:
        return new( arena ) match_charset_t<CI, false>( charset );
    default:
        __assume( 0 ); // tells the compiler that this is unreachable
    }
}

template< typename CI >
inline match_charset<CI> * create_custom_charset(
     custom_charset const * pcharset,
     REGEX_FLAGS flags, regex_arena & arena )
{
    switch( NOCASE & flags )
    {
    case 0:
        return new( arena ) match_custom_charset_t<CI, true>( pcharset );
    case NOCASE:
        return new( arena ) match_custom_charset_t<CI, false>( pcharset );
    default:
        __assume( 0 ); // tells the compiler that this is unreachable
    }
}

template< typename CI >
class word_assertion_t : public assertion<CI>
{
    word_assertion_t & operator=( word_assertion_t const & );
public:
    typedef typename assertion<CI>::char_type char_type;

    word_assertion_t()
        : m_isword( intrinsic_charsets<char_type>::get_word_charset() ) 
    {
    }
protected:
    charset const & m_isword;
};

template< typename CI >
class word_boundary_t : public word_assertion_t<CI>
{
    word_boundary_t & operator=( word_boundary_t const & );

    template< typename CSTRINGS >
    bool _do_match_this( match_param<CI> & param, CI icur, CSTRINGS ) const
    {
        bool const fthisword = ! eos_t<CSTRINGS>::eval( param, icur ) && m_isword.in( *icur, true_t()  );
        bool const fprevword = ! bos_t<CSTRINGS>::eval( param, icur ) && m_isword.in( *--icur, true_t() );

        return ( m_fisboundary == ( fprevword != fthisword ) );
    }
public:
    word_boundary_t( bool const fisboundary )
        : m_fisboundary( fisboundary ) 
    {
    }

    virtual bool recursive_match_all_( match_param<CI> & param, CI icur ) const
    {
        return ( word_boundary_t::recursive_match_this_( param, icur ) && recursive_match_next_( param, icur, false_t() ) );
    }
    virtual bool recursive_match_all_c( match_param<CI> & param, CI icur ) const
    {
        return ( word_boundary_t::recursive_match_this_c( param, icur ) && recursive_match_next_( param, icur, true_t() ) );
    }
    virtual bool recursive_match_this_( match_param<CI> & param, CI & icur ) const
    {
        return _do_match_this( param, icur, false_t() );
    }
    virtual bool recursive_match_this_c( match_param<CI> & param, CI & icur ) const
    {
        return _do_match_this( param, icur, true_t() );
    }
    virtual bool iterative_match_this_( match_param<CI> & param ) const
    {
        param.next = next();
        return _do_match_this( param, param.icur, false_t() );
    }
    virtual bool iterative_match_this_c( match_param<CI> & param ) const
    {
        param.next = next();
        return _do_match_this( param, param.icur, true_t() );
    }
protected:
    bool const m_fisboundary;
};

template< typename CI >
class word_start_t : public word_assertion_t<CI>
{
    word_start_t & operator=( word_start_t const & );

    template< typename CSTRINGS >
    bool _do_match_this( match_param<CI> & param, CI icur, CSTRINGS ) const
    {
        bool const fthisword = ! eos_t<CSTRINGS>::eval( param, icur ) && m_isword.in( *icur, true_t()  );
        bool const fprevword = ! bos_t<CSTRINGS>::eval( param, icur ) && m_isword.in( *--icur, true_t() );

        return ! fprevword && fthisword;
    }
public:
    virtual bool recursive_match_all_( match_param<CI> & param, CI icur ) const
    {
        return ( word_start_t::recursive_match_this_( param, icur ) && recursive_match_next_( param, icur, false_t() ) );
    }
    virtual bool recursive_match_all_c( match_param<CI> & param, CI icur ) const
    {
        return ( word_start_t::recursive_match_this_c( param, icur ) && recursive_match_next_( param, icur, true_t() ) );
    }
    virtual bool recursive_match_this_( match_param<CI> & param, CI & icur ) const
    {
        return _do_match_this( param, icur, false_t() );
    }
    virtual bool recursive_match_this_c( match_param<CI> & param, CI & icur ) const
    {
        return _do_match_this( param, icur, true_t() );
    }
    virtual bool iterative_match_this_( match_param<CI> & param ) const
    {
        param.next = next();
        return _do_match_this( param, param.icur, false_t() );
    }
    virtual bool iterative_match_this_c( match_param<CI> & param ) const
    {
        param.next = next();
        return _do_match_this( param, param.icur, true_t() );
    }
};

template< typename CI >
class word_stop_t : public word_assertion_t<CI>
{
    word_stop_t & operator=( word_stop_t const & );

    template< typename CSTRINGS >
    bool _do_match_this( match_param<CI> & param, CI icur, CSTRINGS ) const
    {
        bool const fthisword = ! eos_t<CSTRINGS>::eval( param, icur ) && m_isword.in( *icur, true_t()  );
        bool const fprevword = ! bos_t<CSTRINGS>::eval( param, icur ) && m_isword.in( *--icur, true_t() );

        return fprevword && ! fthisword;
    }
public:
    virtual bool recursive_match_all_( match_param<CI> & param, CI icur ) const
    {
        return ( word_stop_t::recursive_match_this_( param, icur ) && recursive_match_next_( param, icur, false_t() ) );
    }
    virtual bool recursive_match_all_c( match_param<CI> & param, CI icur ) const
    {
        return ( word_stop_t::recursive_match_this_c( param, icur ) && recursive_match_next_( param, icur, true_t() ) );
    }
    virtual bool recursive_match_this_( match_param<CI> & param, CI & icur ) const
    {
        return _do_match_this( param, icur, false_t() );
    }
    virtual bool recursive_match_this_c( match_param<CI> & param, CI & icur ) const
    {
        return _do_match_this( param, icur, true_t() );
    }
    virtual bool iterative_match_this_( match_param<CI> & param ) const
    {
        param.next = next();
        return _do_match_this( param, param.icur, false_t() );
    }
    virtual bool iterative_match_this_c( match_param<CI> & param ) const
    {
        param.next = next();
        return _do_match_this( param, param.icur, true_t() );
    }
};

template< typename CI >
inline assertion<CI> * create_word_boundary(
    bool const fisboundary,
    REGEX_FLAGS, regex_arena & arena )
{
    return new( arena ) word_boundary_t<CI>( fisboundary );
}

template< typename CI >
inline assertion<CI> * create_word_start( REGEX_FLAGS, regex_arena & arena )
{
    return new( arena ) word_start_t<CI>();
}

template< typename CI >
inline assertion<CI> * create_word_stop( REGEX_FLAGS, regex_arena & arena )
{
    return new( arena ) word_stop_t<CI>();
}

typedef std::pair<size_t, size_t> extent;

template< typename CI > class max_group_quantifier;
template< typename CI > class min_group_quantifier;

template< typename CI >
class match_group_base : public sub_expr<CI>
{
protected:
    typedef std::list<sub_expr<CI>*, REGEX_ALLOCATOR<sub_expr<CI>*> > alt_list_type;
private:
    match_group_base & operator=( match_group_base const & );

    void _push_frame( match_param<CI> & param ) const
    {
        unsafe_stack * ps = param.pstack;

        if( size_t( -1 ) != m_cgroup )
        {
            CI & reserved1 = ( *param.prgbackrefs )[ m_cgroup ].reserved1;
            ps->push( reserved1 );
            reserved1 = param.icur;
        }

        ps->push( m_rgalternates.begin() );
    }

    void _pop_frame( match_param<CI> & param ) const
    {
        typedef typename alt_list_type::const_iterator iter_type;
        unsafe_stack * ps = param.pstack;

        iter_type it;
        ps->pop( it );

        if( size_t( -1 ) != m_cgroup )
            ps->pop( ( *param.prgbackrefs )[ m_cgroup ].reserved1 );
    }

    template< typename CSTRINGS >
    bool _recursive_match_all( match_param<CI> & param, CI icur, CSTRINGS ) const
    {
        typedef typename alt_list_type::const_iterator LCI;

        if( size_t( -1 ) != m_cgroup ) // could be -1 if this is a lookahead_assertion
        {
            CI old_istart = ( *param.prgbackrefs )[ m_cgroup ].reserved1;
            ( *param.prgbackrefs )[ m_cgroup ].reserved1 = icur;

            for( LCI ialt = m_rgalternates.begin(); ialt != m_rgalternates.end(); ++ialt )
            {
                if( (*ialt)->recursive_match_all_( param, icur, CSTRINGS() ) )
                    return true;
            }

            ( *param.prgbackrefs )[ m_cgroup ].reserved1 = old_istart;
        }
        else
        {
            for( LCI ialt = m_rgalternates.begin(); ialt != m_rgalternates.end(); ++ialt )
            {
                if( (*ialt)->recursive_match_all_( param, icur, CSTRINGS() ) )
                    return true;
            }
        }

        return false;
    }
    bool _iterative_match_this( match_param<CI> & param ) const
    {
        _push_frame( param );
        param.next = m_rgalternates.front();
        return true;
    }
    bool _iterative_rematch_this( match_param<CI> & param ) const
    {
        typedef typename alt_list_type::const_iterator LCI;
        LCI next_iter = ++param.pstack->top( LCI() );
        if( next_iter != m_rgalternates.end() )
        {
            param.next = *next_iter;
            return true;
        }
        _pop_frame( param );
        return false;
    }
public:
    match_group_base( size_t cgroup, regex_arena & arena )
        : m_rgalternates( MAKE_ALLOCATOR(sub_expr<CI>const*,arena) ),
          m_pptail( NULL ), m_cgroup( cgroup ), m_nwidth( uninit_width() )
    {
    }

    // Derived classes that own the end_group object must have a
    // destructor, and that destructor must call _cleanup().
    virtual ~match_group_base() = 0;

    virtual bool recursive_match_all_( match_param<CI> & param, CI icur ) const
    {
        return _recursive_match_all( param, icur, false_t() );
    }
    virtual bool recursive_match_all_c( match_param<CI> & param, CI icur ) const
    {
        return _recursive_match_all( param, icur, true_t() );
    }
    virtual bool iterative_match_this_( match_param<CI> & param ) const
    {
        return _iterative_match_this( param );
    }
    virtual bool iterative_match_this_c( match_param<CI> & param ) const
    {
        return _iterative_match_this( param );
    }
    virtual bool iterative_rematch_this_( match_param<CI> & param ) const
    {
        return _iterative_rematch_this( param );
    }
    virtual bool iterative_rematch_this_c( match_param<CI> & param ) const
    {
        return _iterative_rematch_this( param );
    }

    size_t group_number() const
    {
        return m_cgroup;
    }

    void add_item( sub_expr<CI> * pitem )
    {
        *m_pptail = pitem;
        m_pptail = pitem->pnext();
    }

    void add_alternate()
    {
        m_rgalternates.push_back( NULL );
        m_pptail = & m_rgalternates.back();
    }

    void end_alternate()
    {
        *m_pptail = _get_end_group();
    }

    size_t calternates() const
    {
        return m_rgalternates.size();
    }

    virtual void set_extent( extent const & )
    {
    }

    width_type group_width( std::vector<match_group_base<CI>*> & rggroups,
                            std::list<size_t> const & invisible_groups )
    {
        assert( 0 == m_cgroup );
        // This should only be called on the top node
        if( uninit_width() == m_nwidth )
        {
            width_param<CI> param1( 0, uninit_width(), rggroups, invisible_groups );
            match_group_base<CI>::width_this( param1 );
            // If someone incremented the cookie, then we need a second pass
            if( 0 != param1.cookie && worst_width != m_nwidth )
            {
                width_param<CI> param2( 0, m_nwidth, rggroups, invisible_groups );
                match_group_base<CI>::width_this( param2 );
                assert( 0 == param2.cookie );
            }
        }
        return m_nwidth;
    }

    virtual width_type width_this( width_param<CI> & param )
    {
        width_type width = { size_t( -1 ), 0 };
        typedef typename alt_list_type::iterator LCI;
        for( LCI ialt = m_rgalternates.begin(); worst_width != width && ialt != m_rgalternates.end(); ++ialt )
        {
            // prevent possible infinite recursion
            if( m_cgroup < param.rggroups.size() )
                param.rggroups[ m_cgroup ] = NULL;

            width_type temp_width = ( *ialt )->get_width( param );

            if( m_cgroup < param.rggroups.size() )
                param.rggroups[ m_cgroup ] = this;

            width.m_min = (std::min)( width.m_min, temp_width.m_min );
            width.m_max = (std::max)( width.m_max, temp_width.m_max );
        }
        return m_nwidth = width;
    }

protected:
    void _cleanup()
    {
        typedef typename alt_list_type::const_iterator LCI;
        for( LCI ialt = m_rgalternates.begin(); ialt != m_rgalternates.end(); ++ialt )
            delete *ialt;
        m_rgalternates.clear();
    }

    virtual sub_expr<CI> * _get_end_group() = 0;

    alt_list_type    m_rgalternates;
    sub_expr<CI>  ** m_pptail; // only used when adding elements
    size_t const     m_cgroup;
    width_type       m_nwidth;
};

template< typename CI >
inline match_group_base<CI>::~match_group_base()
{
}

// A indestructable_sub_expr is an object that brings itself back
// to life after explicitly being deleted.  It is used
// to ease clean-up of the sub_expr graph, where most
// nodes are dynamically allocated, but some nodes are
// members of other nodes and are not dynamically allocated.
// The recursive delete of the sub_expr graph causes
// delete to be ( incorrectly ) called on these members.
// By inheriting these members from indestructable_sub_expr,
// explicit attempts to delete the object will have no
// effect. ( Actually, the object will be destructed and
// then immediately reconstructed. ) This is accomplished
// by calling placement new in operator delete.
template< typename CI, typename T >
class indestructable_sub_expr : public sub_expr<CI>
{
    static void * operator new( size_t, regex_arena & );
    static void operator delete( void *, regex_arena & );
protected:
    static void * operator new( size_t, void * pv ) { return pv; }
    static void operator delete( void *, void * ) {}
public:
    virtual ~indestructable_sub_expr() {}
    static void operator delete( void * pv ) { new( pv ) T; }
};

template< typename CI >
class match_group : public match_group_base<CI>
{
    match_group & operator=( match_group const & );
public:
    match_group( size_t cgroup, regex_arena & arena )
        : match_group_base<CI>( cgroup, arena ),
          m_end_group( this ) 
    {
    }

    virtual ~match_group()
    {
        _cleanup();
    }

    virtual sub_expr<CI> * quantify( size_t lbound, size_t ubound, bool greedy, regex_arena & arena )
    {
        if( greedy )
            return new( arena ) max_group_quantifier<CI>( this, lbound, ubound );
        else
            return new( arena ) min_group_quantifier<CI>( this, lbound, ubound );
    }

protected:
    typedef typename match_group_base<CI>::alt_list_type alt_list_type;

    struct old_backref
    {
        CI   istart;
        CI   iend;
        bool matched;

        old_backref() {}
        old_backref( backref_tag<CI> const & br )
            : istart( br.first ), iend( br.second ), matched( br.matched ) {}
    };

    static void restore_backref( backref_tag<CI> & br, old_backref const & old_br )
    {
        br.first   = old_br.istart;
        br.second  = old_br.iend;
        br.matched = old_br.matched;
    }

    template< typename CSTRINGS >
    bool _call_back( match_param<CI> & param, CI icur, CSTRINGS ) const
    {
        if( size_t( -1 ) != m_cgroup )
        {
            // Save the relevant portions of the backref in an old_backref struct
            old_backref old_br( ( *param.prgbackrefs )[ m_cgroup ] );

            ( *param.prgbackrefs )[ m_cgroup ].first   = ( *param.prgbackrefs )[ m_cgroup ].reserved1;
            ( *param.prgbackrefs )[ m_cgroup ].second  = icur;
            ( *param.prgbackrefs )[ m_cgroup ].matched = true;

            if( recursive_match_next_( param, icur, CSTRINGS() ) )
                return true;

            // Restore the backref to its saved state
            restore_backref( ( *param.prgbackrefs )[ m_cgroup ], old_br );
        }
        else
        {
            if( recursive_match_next_( param, icur, CSTRINGS() ) )
                return true;
        }

        return false;
    }

    class end_group : public indestructable_sub_expr<CI, end_group>
    {
        match_group<CI> const *const m_pgroup;

        end_group & operator=( end_group const & );

        void _push_frame( match_param<CI> & param ) const
        {
            size_t cgroup = m_pgroup->group_number();

            if( size_t( -1 ) != cgroup )
            {
                backref_tag<CI> & br = ( *param.prgbackrefs )[ cgroup ];
                old_backref old_br( br );
                param.pstack->push( old_br );

                br.first   = br.reserved1;
                br.second  = param.icur;
                br.matched = true;
            }
        }
        void _pop_frame( match_param<CI> & param ) const
        {
            size_t cgroup = m_pgroup->group_number();

            if( size_t( -1 ) != cgroup )
            {
                old_backref old_br;
                param.pstack->pop( old_br );
                match_group<CI>::restore_backref( ( *param.prgbackrefs )[ cgroup ], old_br );
            }
        }
        bool _iterative_match_this( match_param<CI> & param ) const
        {
            _push_frame( param );
            param.next = m_pgroup->next();
            return true;
        }
        bool _iterative_rematch_this( match_param<CI> & param ) const
        {
            _pop_frame( param );
            return false;
        }
    public:
        end_group( match_group<CI> const * pgroup = NULL )
            : m_pgroup( pgroup ) 
        {
        }
        virtual bool recursive_match_all_( match_param<CI> & param, CI icur ) const
        {
            return m_pgroup->_call_back( param, icur, false_t() );
        }
        virtual bool recursive_match_all_c( match_param<CI> & param, CI icur ) const
        {
            return m_pgroup->_call_back( param, icur, true_t() );
        }
        virtual bool iterative_match_this_( match_param<CI> & param ) const
        {
            return _iterative_match_this( param );
        }
        virtual bool iterative_match_this_c( match_param<CI> & param ) const
        {
            return _iterative_match_this( param );
        }
        virtual bool iterative_rematch_this_( match_param<CI> & param ) const
        {
            return _iterative_rematch_this( param );
        }
        virtual bool iterative_rematch_this_c( match_param<CI> & param ) const
        {
            return _iterative_rematch_this( param );
        }
        virtual width_type width_this( width_param<CI> & )
        { 
            return zero_width; 
        }
    } m_end_group;

    friend class end_group;

    virtual sub_expr<CI> * _get_end_group()
    {
        return & m_end_group;
    }
};

template< typename CI >
inline void save_backrefs( std::vector<backref_tag<CI> > const & rgbackrefs, CI * prgci )
{
    typedef typename std::vector<backref_tag<CI> >::const_iterator VI;
    for( VI iter = rgbackrefs.begin(); iter != rgbackrefs.end(); ++iter, ++prgci )
    {
        new( static_cast<void*>( prgci ) ) CI( iter->reserved1 );
    }
}

template< typename CI >
inline void restore_backrefs( std::vector<backref_tag<CI> > & rgbackrefs, CI * prgci )
{
    typedef typename std::vector<backref_tag<CI> >::iterator VI;
    for( VI iter = rgbackrefs.begin(); iter != rgbackrefs.end(); ++iter, ++prgci )
    {
        iter->reserved1 = *prgci;
        prgci->~CI();
    }
}

template< typename CI >
class group_wrapper : public sub_expr<CI>
{
    match_group_base<CI> const *const m_pgroup;

    group_wrapper & operator=( group_wrapper const & );
public:
    group_wrapper( match_group_base<CI> const * pgroup ) 
        : m_pgroup( pgroup ) 
    {
    }
    virtual bool iterative_match_this_( match_param<CI> & param ) const
    {
        return m_pgroup->match_group_base<CI>::iterative_match_this_( param );
    }
    virtual bool iterative_match_this_c( match_param<CI> & param ) const
    {
        return m_pgroup->match_group_base<CI>::iterative_match_this_c( param );
    }
    virtual bool iterative_rematch_this_( match_param<CI> & param ) const
    {
        return m_pgroup->match_group_base<CI>::iterative_rematch_this_( param );
    }
    virtual bool iterative_rematch_this_c( match_param<CI> & param ) const
    {
        return m_pgroup->match_group_base<CI>::iterative_rematch_this_c( param );
    }
    virtual width_type width_this( width_param<CI> & ) 
    {
        return zero_width;
    }
};

// Behaves like a lookahead assertion if m_cgroup is -1, or like
// an independent group otherwise.
template< typename CI >
class independent_group_base : public match_group_base<CI>
{
    independent_group_base & operator=( independent_group_base const & );

    template< typename CSTRINGS >
    bool _recursive_match_all( match_param<CI> & param, CI icur, CSTRINGS ) const
    {
        backref_tag<CI> * prgbr = NULL;

        // Copy onto the stack the part of the backref vector that could
        // be modified by the lookahead.
        if( m_extent.second )
        {
            prgbr = static_cast<backref_tag<CI>*>( alloca( m_extent.second * sizeof( backref_tag<CI> ) ) );
            std::copy( param.prgbackrefs->begin() + m_extent.first,
                       param.prgbackrefs->begin() + m_extent.first + m_extent.second,
                       std::raw_storage_iterator<backref_tag<CI>*, backref_tag<CI> >( prgbr ) );
        }

        // Match until the end of this group and then return
        // BUGBUG can the compiler optimize this?
        bool const fdomatch = CSTRINGS::value ?
            match_group_base<CI>::recursive_match_all_c( param, icur ) :
            match_group_base<CI>::recursive_match_all_( param, icur );

        if( m_fexpected == fdomatch )
        {
            // If m_cgroup != 1, then this is not a zero-width assertion.
            if( fdomatch && size_t( -1 ) != m_cgroup )
                icur = ( *param.prgbackrefs )[ m_cgroup ].second;

            if( recursive_match_next_( param, icur, CSTRINGS() ) )
                return true;
        }

        // if match_group::recursive_match_all_ returned true, the backrefs must be restored
        if( m_extent.second && fdomatch )
            std::copy( prgbr, prgbr + m_extent.second, param.prgbackrefs->begin() + m_extent.first );

        return false;
    }
    template< typename CSTRINGS >
    bool _iterative_match_this( match_param<CI> & param, CSTRINGS ) const
    {
        group_wrapper<CI> expr( this );

        _push_frame( param );
        CI istart = param.icur;

        bool const fdomatch = matcher_helper<CI>::_Do_match_iterative( &expr, param, param.icur, CSTRINGS() );

        if( m_fexpected == fdomatch )
        {
            // If m_cgroup == -1, then this is a zero-width assertion.
            if( fdomatch && size_t( -1 ) == m_cgroup )
                param.icur = istart;

            param.next = next();
            return true;
        }

        _pop_frame( param );
        return false;
    }
    bool _iterative_rematch_this( match_param<CI> & param ) const
    {
        _pop_frame( param );
        return false;
    }
public:
    independent_group_base( size_t cgroup, regex_arena & arena )
        : match_group_base<CI>( cgroup, arena ),
          m_fexpected( true ), m_extent( 0, 0 ) 
    {
    }
    virtual void set_extent( extent const & ex )
    {
        m_extent = ex;
    }
    virtual bool recursive_match_all_( match_param<CI> & param, CI icur ) const
    {
        return _recursive_match_all( param, icur, false_t() );
    }
    virtual bool recursive_match_all_c( match_param<CI> & param, CI icur ) const
    {
        return _recursive_match_all( param, icur, true_t() );
    }
    virtual bool iterative_match_this_( match_param<CI> & param ) const
    {
        return _iterative_match_this( param, false_t() );
    }
    virtual bool iterative_match_this_c( match_param<CI> & param ) const
    {
        return _iterative_match_this( param, true_t() );
    }
    virtual bool iterative_rematch_this_( match_param<CI> & param ) const
    {
        return _iterative_rematch_this( param );
    }
    virtual bool iterative_rematch_this_c( match_param<CI> & param ) const
    {
        return _iterative_rematch_this( param );
    }
protected:

    void _push_frame( match_param<CI> & param ) const
    {
        unsafe_stack * pstack = param.pstack;
        typedef typename match_param<CI>::backref_vector::const_iterator VCI;
        VCI istart = param.prgbackrefs->begin() + m_extent.first;
        VCI iend   = istart + m_extent.second;

        for( ; iend != istart; ++istart )
        {
            pstack->push( *istart );
        }
        pstack->push( param.icur );
    }

    void _pop_frame( match_param<CI> & param ) const
    {
        unsafe_stack * pstack = param.pstack;
        typedef typename match_param<CI>::backref_vector::iterator VI;

        VI istart = param.prgbackrefs->begin() + m_extent.first;
        VI iend   = istart + m_extent.second;

        pstack->pop( param.icur );
        while( iend != istart )
        {
            pstack->pop( *--iend );
        }
    }

    independent_group_base( bool const fexpected, regex_arena & arena )
        : match_group_base<CI>( size_t( -1 ), arena ), m_fexpected( fexpected ) 
    {
    }

    bool const m_fexpected;
    extent     m_extent;
};

template< typename CI >
class independent_group : public independent_group_base<CI>
{
    independent_group & operator=( independent_group const & );
public:
    independent_group( size_t cgroup, regex_arena & arena )
        : independent_group_base<CI>( cgroup, arena ), m_end_group( this ) 
    {
    }

    virtual ~independent_group()
    {
        _cleanup();
    }

    virtual sub_expr<CI> * quantify( size_t lbound, size_t ubound, bool greedy, regex_arena & arena )
    {
        if( greedy )
            return new( arena ) max_group_quantifier<CI>( this, lbound, ubound );
        else
            return new( arena ) min_group_quantifier<CI>( this, lbound, ubound );
    }

protected:
    independent_group( bool const fexpected, regex_arena & arena )
        : independent_group_base<CI>( fexpected, arena ),
          m_end_group( this ) 
    {
    }

    bool _call_back( match_param<CI> & param, CI icur ) const
    {
        if( size_t( -1 ) != m_cgroup )
        {
            backref_tag<CI> & br = ( *param.prgbackrefs )[ m_cgroup ];
            br.first   = br.reserved1;
            br.second  = icur;
            br.matched = true;
        }
        return true;
    }

    class end_group : public indestructable_sub_expr<CI, end_group>
    {
        independent_group<CI> const *const m_pgroup;

        end_group & operator=( end_group const & );
        bool _iterative_match_this( match_param<CI> & param ) const
        {
            size_t cgroup = m_pgroup->group_number();
            if( size_t( -1 ) != cgroup )
            {
                backref_tag<CI> & br = ( *param.prgbackrefs )[ cgroup ];
                br.first   = br.reserved1;
                br.second  = param.icur;
                br.matched = true;
            }
            param.next = NULL;
            return true;
        }
    public:
        end_group( independent_group<CI> const * pgroup = NULL )
            : m_pgroup( pgroup ) 
        {
        }
        virtual bool recursive_match_all_( match_param<CI> & param, CI icur ) const
        {
            return m_pgroup->_call_back( param, icur );
        }
        virtual bool recursive_match_all_c( match_param<CI> & param, CI icur ) const
        {
            return m_pgroup->_call_back( param, icur );
        }
        virtual bool iterative_match_this_( match_param<CI> & param ) const
        {
            return _iterative_match_this( param );
        }
        virtual bool iterative_match_this_c( match_param<CI> & param ) const
        {
            return _iterative_match_this( param );
        }
        virtual width_type width_this( width_param<CI> & )
        {
            return zero_width;
        }
    } m_end_group;

    friend class end_group;

    virtual sub_expr<CI> * _get_end_group()
    {
        return & m_end_group;
    }
};

template< typename CI >
class lookahead_assertion : public independent_group<CI>
{
    lookahead_assertion & operator=( lookahead_assertion const & );

public:
    lookahead_assertion( bool const fexpected, regex_arena & arena )
        : independent_group<CI>( fexpected, arena ) 
    {
    }
    virtual sub_expr<CI> * quantify( size_t, size_t, bool, regex_arena & )
    {
        throw bad_regexpr( "look-ahead assertion cannot be quantified" );
    }
    virtual bool is_assertion() const
    {
        return true;
    }
    virtual width_type width_this( width_param<CI> & param )
    {
        // calculate the group's width and store it, but return zero_width
        match_group_base<CI>::width_this( param );
        return zero_width;
    }
};

template< typename CI >
class lookbehind_assertion : public independent_group_base<CI>
{
    lookbehind_assertion & operator=( lookbehind_assertion const & );

    template< typename CSTRINGS >
    bool _recursive_match_all( match_param<CI> & param, CI icur, CSTRINGS ) const
    {
        // This is the room in the string from the start to the current position
        size_t room = std::distance( param.ibegin, icur );

        // If we don't have enough room to match the lookbehind, the match fails.
        // If we wanted the match to fail, try to match the rest of the pattern.
        if( m_nwidth.m_min > room )
            return m_fexpected ? false : recursive_match_next_( param, icur, CSTRINGS() );

        backref_tag<CI> * prgbr = NULL;

        // Copy onto the stack the part of the backref vector that could
        // be modified by the lookbehind.
        if( m_extent.second )
        {
            prgbr = static_cast<backref_tag<CI>*>( alloca( m_extent.second * sizeof( backref_tag<CI> ) ) );
            std::copy( param.prgbackrefs->begin() + m_extent.first,
                       param.prgbackrefs->begin() + m_extent.first + m_extent.second,
                       std::raw_storage_iterator<backref_tag<CI>*, backref_tag<CI> >( prgbr ) );
        }

        CI local_istart  = icur;
        std::advance( local_istart, -int( (std::min)( m_nwidth.m_max, room ) ) );

        CI local_istop = icur;
        std::advance( local_istop, -int( m_nwidth.m_min - 1 ) );

        // Create a local param struct that has icur as param.iend
        match_param<CI> local_param( param.ibegin, param.istart, icur, param.prgbackrefs );

        // Find the rightmost match that ends at icur.
        for( CI local_icur = local_istart; local_icur != local_istop; ++local_icur )
        {
            // Match until the end of this group and then return
            // Note that we're calling recursive_match_all_ regardless of the CSTRINGS switch.
            // This is because for the lookbehind assertion, the termination condition is when
            // icur == param.iend, not when *icur == '\0'
            bool const fmatched = match_group_base<CI>::recursive_match_all_( local_param, local_icur );

            // If the match results were what we were expecting, try to match the
            // rest of the pattern. If that succeeds, return true.
            if( m_fexpected == fmatched && recursive_match_next_( param, icur, CSTRINGS() ) )
                return true;

            // if match_group::recursive_match_all_ returned true, the backrefs must be restored
            if( fmatched )
            {
                if( m_extent.second )
                    std::copy( prgbr, prgbr + m_extent.second, param.prgbackrefs->begin() + m_extent.first );

                // Match succeeded. If this is a negative lookbehind, we didn't want it
                // to succeed, so return false.
                if( ! m_fexpected )
                    return false;
            }
        }

        // No variation of the lookbehind was satisfied in a way that permited
        // the rest of the pattern to match successfully, so return false.
        return false;
    }

    template< typename CSTRINGS >
    bool _iterative_match_this( match_param<CI> & param, CSTRINGS ) const
    {
        // Save the backrefs
        _push_frame( param );

        // This is the room in the string from the start to the current position
        size_t room = std::distance( param.ibegin, param.icur );

        // If we don't have enough room to match the lookbehind, the match fails.
        // If we wanted the match to fail, try to match the rest of the pattern.
        if( m_nwidth.m_min > room )
        {
            if( m_fexpected )
            {
                _pop_frame( param );
                return false;
            }
            param.next = next();
            return true;
        }

        CI local_istart  = param.icur;
        std::advance( local_istart, -int( (std::min)( m_nwidth.m_max, room ) ) );

        CI local_istop = param.icur;
        std::advance( local_istop, -int( m_nwidth.m_min - 1 ) );

        // Create a local param struct that has icur as param.iend
        match_param<CI> local_param( param.ibegin, param.istart, param.icur, param.prgbackrefs );
        local_param.pstack = param.pstack;

        group_wrapper<CI> expr( this );

        // Find the rightmost match that ends at icur.
        for( CI local_icur = local_istart; local_icur != local_istop; ++local_icur )
        {
            // Match until the end of this group and then return
            // Note that we're calling _Do_match_iterative_helper regardless of the CSTRINGS switch.
            // This is because for the lookbehind assertion, the termination condition is when
            // icur == param.iend, not when *icur == '\0'
            bool const fmatched = matcher_helper<CI>::_Do_match_iterative_helper( &expr, local_param, local_icur );

            // If the match results were what we were expecting, try to match the
            // rest of the pattern. If that succeeds, return true.
            if( m_fexpected == fmatched )
            {
                param.next = next();
                return true;
            }

            // if match_group::recursive_match_all_ returned true, the backrefs must be restored
            if( fmatched )
            {
                // Restore the backrefs
                _pop_frame( param );

                // Match succeeded. If this is a negative lookbehind, we didn't want it
                // to succeed, so return false.
                if( ! m_fexpected )
                    return false;

                // Save the backrefs again.
                _push_frame( param );
            }
        }

        // No variation of the lookbehind was satisfied in a way that permited
        // the rest of the pattern to match successfully, so return false.
        _pop_frame( param );
        return false;
    }
    bool _iterative_rematch_this( match_param<CI> & param ) const
    {
        _pop_frame( param );
        return false;
    }
public:
    lookbehind_assertion( bool const fexpected, regex_arena & arena )
        : independent_group_base<CI>( fexpected, arena )
    {
    }

    virtual ~lookbehind_assertion()
    {
        _cleanup();
    }

    virtual bool recursive_match_all_( match_param<CI> & param, CI icur ) const
    {
        return _recursive_match_all( param, icur, false_t() );
    }
    virtual bool recursive_match_all_c( match_param<CI> & param, CI icur ) const
    {
        return _recursive_match_all( param, icur, true_t() );
    }
    virtual bool iterative_match_this_( match_param<CI> & param ) const
    {
        return _iterative_match_this( param, false_t() );
    }
    virtual bool iterative_match_this_c( match_param<CI> & param ) const
    {
        return _iterative_match_this( param, true_t() );
    }
    virtual bool iterative_rematch_this_( match_param<CI> & param ) const
    {
        return _iterative_rematch_this( param );
    }
    virtual bool iterative_rematch_this_c( match_param<CI> & param ) const
    {
        return _iterative_rematch_this( param );
    }

    virtual bool is_assertion() const
    {
        return true;
    }
    virtual width_type width_this( width_param<CI> & param )
    {
        // calculate the group's width and store it, but return zero_width
        match_group_base<CI>::width_this( param );
        return zero_width;
    }

protected:
    struct end_group : public indestructable_sub_expr<CI, end_group>
    {
        virtual bool recursive_match_all_( match_param<CI> & param, CI icur ) const
        {
            return param.istop == icur;
        }
        virtual bool recursive_match_all_c( match_param<CI> & param, CI icur ) const
        {
            return param.istop == icur;
        }
        virtual bool iterative_match_this_( match_param<CI> & param ) const
        {
            param.next = NULL;
            return param.istop == param.icur;
        }
        virtual bool iterative_match_this_c( match_param<CI> & param ) const
        {
            param.next = NULL;
            return param.istop == param.icur;
        }
        virtual width_type width_this( width_param<CI> & )
        {
            return zero_width;
        }
    } m_end_group;

    virtual sub_expr<CI> * _get_end_group()
    {
        return & m_end_group;
    }
};

template< typename CI >
class group_quantifier : public match_quantifier<CI>
{
    group_quantifier & operator=( group_quantifier const & );
    
    bool _iterative_match_this( match_param<CI> & param ) const
    {
        _push_frame( param );
        param.next = m_psub->next(); // ptr to end_quant
        return true;
    }
    bool _iterative_rematch_this( match_param<CI> & param ) const
    {
        _pop_frame( param );
        return false;
    }
public:
    group_quantifier( match_group_base<CI> * psub,
                      size_t lbound, size_t ubound,
                      sub_expr<CI> * pend_quant )
        : match_quantifier<CI>( psub, lbound, ubound ),
          m_group( *psub )
    {
        *psub->pnext() = pend_quant;
    }

    // sub-classes of group_quantifer that own the end_quant
    // object must declare a destructor, and it must call _cleanup
    virtual ~group_quantifier() = 0;

    virtual bool iterative_match_this_( match_param<CI> & param ) const
    {
        return _iterative_match_this( param );
    }
    virtual bool iterative_match_this_c( match_param<CI> & param ) const
    {
        return _iterative_match_this( param );
    }
    virtual bool iterative_rematch_this_( match_param<CI> & param ) const
    {
        return _iterative_rematch_this( param );
    }
    virtual bool iterative_rematch_this_c( match_param<CI> & param ) const
    {
        return _iterative_rematch_this( param );
    }

protected:
    struct old_quant
    {
        size_t reserved2;
        bool   reserved3;
        CI     reserved4;
        CI     reserved5;

        old_quant() 
        {
        }
        old_quant( backref_tag<CI> const & br )
          : reserved2( br.reserved2 ), reserved3( br.reserved3 ),
            reserved4( br.reserved4 ), reserved5( br.reserved5 )
        {
        }
    };

    void _push_frame( match_param<CI> & param ) const
    {
        backref_tag<CI> & br = ( *param.prgbackrefs )[ group_number() ];
        old_quant old_qt( br );
        param.pstack->push( old_qt );

        br.reserved2 = 0;    // nbr of times this group has matched
        br.reserved3 = true; // toggle used for backtracking
        br.reserved4 = static_init<CI>::value;
        br.reserved5 = static_init<CI>::value;
    }

    void _pop_frame( match_param<CI> & param ) const
    {
        backref_tag<CI> & br = ( *param.prgbackrefs )[ group_number() ];
        old_quant old_qt;
        param.pstack->pop( old_qt );

        br.reserved2 = old_qt.reserved2;
        br.reserved3 = old_qt.reserved3;
        br.reserved4 = old_qt.reserved4;
        br.reserved5 = old_qt.reserved5;
    }

    size_t group_number() const
    {
        return m_group.group_number();
    }

    size_t & cmatches( match_param<CI> & param ) const
    {
        return ( *param.prgbackrefs )[ group_number() ].reserved2;
    }

    CI & highwater1( match_param<CI> & param ) const
    {
        return ( *param.prgbackrefs )[ group_number() ].reserved4;
    }

    CI & highwater2( match_param<CI> & param ) const
    {
        return ( *param.prgbackrefs )[ group_number() ].reserved5;
    }

    match_group_base<CI> const & m_group;
};

template< typename CI >
inline group_quantifier<CI>::~group_quantifier()
{
}

template< typename CI >
class max_group_quantifier : public group_quantifier<CI>
{
    max_group_quantifier & operator=( max_group_quantifier const & );
    
    template< typename CSTRINGS >
    bool _recursive_match_all( match_param<CI> & param, CI icur, CSTRINGS ) const
    {
        CI     old_highwater1 = highwater1( param );
        CI     old_highwater2 = highwater2( param );
        size_t old_cmatches   = cmatches( param );

        highwater1( param ) = static_init<CI>::value;
        highwater2( param ) = icur;
        cmatches( param )   = 0;

        if( _recurse( param, icur, CSTRINGS() ) )
            return true;

        cmatches( param )   = old_cmatches;
        highwater2( param ) = old_highwater2;
        highwater1( param ) = old_highwater1;

        return false;
    }
public:
    max_group_quantifier( match_group_base<CI> * psub, size_t lbound, size_t ubound )
        : group_quantifier<CI>( psub, lbound, ubound, & m_end_quant ),
          m_end_quant( this ) 
    {
    }

    virtual ~max_group_quantifier()
    {
        // Must call _cleanup() here before the end_quant object
        // gets destroyed.
        _cleanup();
    }

    virtual bool recursive_match_all_( match_param<CI> & param, CI icur ) const
    {
        return _recursive_match_all( param, icur, false_t() );
    }
    virtual bool recursive_match_all_c( match_param<CI> & param, CI icur ) const
    {
        return _recursive_match_all( param, icur, true_t() );
    }

protected:
    template< typename CSTRINGS >
    bool _recurse( match_param<CI> & param, CI icur, CSTRINGS ) const
    {
        if( m_ubound == cmatches( param ) )
            return recursive_match_next_( param, icur, CSTRINGS() );

        ++cmatches( param );
        if( m_psub->recursive_match_all_( param, icur, CSTRINGS() ) )
            return true;

        if( --cmatches( param ) < m_lbound )
            return false;

        return recursive_match_next_( param, icur, CSTRINGS() );
    }

    class end_quantifier : public indestructable_sub_expr<CI, end_quantifier>
    {
        max_group_quantifier<CI> const *const m_pquant;

        end_quantifier & operator=( end_quantifier const & );

        void _push_frame( match_param<CI> & param ) const
        {
            backref_tag<CI> & br = ( *param.prgbackrefs )[ m_pquant->group_number() ];
            param.pstack->push( br.reserved4 );
            br.reserved4 = br.reserved5;
            br.reserved5 = param.icur;
        }

        void _pop_frame( match_param<CI> & param ) const
        {
            backref_tag<CI> & br = ( *param.prgbackrefs )[ m_pquant->group_number() ];
            br.reserved5 = br.reserved4;
            param.pstack->pop( br.reserved4 );
        }

        template< typename CSTRINGS >
        bool _recursive_match_all( match_param<CI> & param, CI icur, CSTRINGS ) const
        {
            CI old_highwater1 = m_pquant->highwater1( param );

            if( icur == old_highwater1 )
                return m_pquant->recursive_match_next_( param, icur, CSTRINGS() );

            m_pquant->highwater1( param ) = m_pquant->highwater2( param );
            m_pquant->highwater2( param ) = icur;

            if( m_pquant->_recurse( param, icur, CSTRINGS() ) )
                return true;

            m_pquant->highwater2( param ) = m_pquant->highwater1( param );
            m_pquant->highwater1( param ) = old_highwater1;

            return false;
        }
        bool _iterative_match_this( match_param<CI> & param ) const
        {
            backref_tag<CI> & br = ( *param.prgbackrefs )[ m_pquant->group_number() ];

            // forcibly break the infinite loop
            if( param.icur == br.reserved4 )
            {
                _push_frame( param );
                param.next = m_pquant->next();
                return true;
            }

            _push_frame( param );

            // If we've matched the max nbr of times, move on to the next
            // sub-expr. 
            if( m_pquant->m_ubound == br.reserved2 )
            {
                param.next = m_pquant->next();
                br.reserved3 = false;
                return true;
            }

            // Rematch the group.
            br.reserved3 = true;
            param.next = m_pquant->m_psub;
            ++br.reserved2;
            return true;
        }
        bool _iterative_rematch_this( match_param<CI> & param ) const
        {
            backref_tag<CI> & br = ( *param.prgbackrefs )[ m_pquant->group_number() ];

            // infinite loop forcibly broken
            if( param.icur == param.pstack->top( static_init<CI>::value ) )
            {
                _pop_frame( param );
                return false;
            }

            if( br.reserved3 )
            {
                --br.reserved2;
                param.next = m_pquant->next();
                if( m_pquant->m_lbound <= br.reserved2 )
                {
                    br.reserved3 = false;
                    return true;
                }
                _pop_frame( param );
                return false;
            }
            br.reserved3 = true;
            _pop_frame( param );
            return false;
        }
    public:
        end_quantifier( max_group_quantifier<CI> const * pquant = NULL )
            : m_pquant( pquant )
        {
        }
        virtual bool recursive_match_all_( match_param<CI> & param, CI icur ) const
        {
            return _recursive_match_all( param, icur, false_t() );
        }
        virtual bool recursive_match_all_c( match_param<CI> & param, CI icur ) const
        {
            return _recursive_match_all( param, icur, true_t() );
        }
        virtual bool iterative_match_this_( match_param<CI> & param ) const
        {
            return _iterative_match_this( param );
        }
        virtual bool iterative_match_this_c( match_param<CI> & param ) const
        {
            return _iterative_match_this( param );
        }
        virtual bool iterative_rematch_this_( match_param<CI> & param ) const
        {
            return _iterative_rematch_this( param );
        }
        virtual bool iterative_rematch_this_c( match_param<CI> & param ) const
        {
            return _iterative_rematch_this( param );
        }
        virtual width_type width_this( width_param<CI> & )
        {
            return zero_width;
        }
    } m_end_quant;

    friend class end_quantifier;
};

template< typename CI >
class min_group_quantifier : public group_quantifier<CI>
{
    min_group_quantifier & operator=( min_group_quantifier const & );

    template< typename CSTRINGS >
    bool _recursive_match_all( match_param<CI> & param, CI icur, CSTRINGS ) const
    {
        CI     old_highwater1 = highwater1( param );
        CI     old_highwater2 = highwater2( param );
        size_t old_cmatches   = cmatches( param );

        highwater1( param ) = static_init<CI>::value;
        highwater2( param ) = icur;
        cmatches( param )   = 0;

        if( _recurse( param, icur, CSTRINGS() ) )
            return true;

        cmatches( param )   = old_cmatches;
        highwater2( param ) = old_highwater2;
        highwater1( param ) = old_highwater1;

        return false;
    }
public:
    min_group_quantifier( match_group_base<CI> * psub, size_t lbound, size_t ubound )
        : group_quantifier<CI>( psub, lbound, ubound, & m_end_quant ),
          m_end_quant( this )
    {
    }

    virtual ~min_group_quantifier()
    {
        // Must call _cleanup() here before the end_quant object
        // gets destroyed.
        _cleanup();
    }

    virtual bool recursive_match_all_( match_param<CI> & param, CI icur ) const
    {
        return _recursive_match_all( param, icur, false_t() );
    }
    virtual bool recursive_match_all_c( match_param<CI> & param, CI icur ) const
    {
        return _recursive_match_all( param, icur, true_t() );
    }

protected:

    template< typename CSTRINGS >
    bool _recurse( match_param<CI> & param, CI icur, CSTRINGS ) const
    {
        if( m_lbound <= cmatches( param ) )
        {
            if( recursive_match_next_( param, icur, CSTRINGS() ) )
                return true;
        }

        if( m_ubound > cmatches( param ) )
        {
            ++cmatches( param );
            if( m_psub->recursive_match_all_( param, icur, CSTRINGS() ) )
                return true;
            --cmatches( param );
        }

        return false;
    }

    class end_quantifier : public indestructable_sub_expr<CI, end_quantifier>
    {
        min_group_quantifier<CI> const *const m_pquant;

        end_quantifier & operator=( end_quantifier const & );

        void _push_frame( match_param<CI> & param ) const
        {
            backref_tag<CI> & br = ( *param.prgbackrefs )[ m_pquant->group_number() ];
            param.pstack->push( br.reserved4 );
            br.reserved4 = br.reserved5;
            br.reserved5 = param.icur;
        }

        void _pop_frame( match_param<CI> & param ) const
        {
            backref_tag<CI> & br = ( *param.prgbackrefs )[ m_pquant->group_number() ];
            br.reserved5 = br.reserved4;
            param.pstack->pop( br.reserved4 );
        }

        template< typename CSTRINGS >
        bool _recursive_match_all( match_param<CI> & param, CI icur, CSTRINGS ) const
        {
            CI old_highwater1 = m_pquant->highwater1( param );

            if( icur == old_highwater1 )
                return m_pquant->recursive_match_next_( param, icur, CSTRINGS() );

            m_pquant->highwater1( param ) = m_pquant->highwater2( param );
            m_pquant->highwater2( param ) = icur;

            if( m_pquant->_recurse( param, icur, CSTRINGS() ) )
                return true;

            m_pquant->highwater2( param ) = m_pquant->highwater1( param );
            m_pquant->highwater1( param ) = old_highwater1;

            return false;
        }

        bool _iterative_match_this( match_param<CI> & param ) const
        {
            backref_tag<CI> & br = ( *param.prgbackrefs )[ m_pquant->group_number() ];

            // forcibly break the infinite loop
            if( param.icur == br.reserved4 )
            {
                _push_frame( param );
                param.next = m_pquant->next();
                return true;
            }

            _push_frame( param );

            if( m_pquant->m_lbound <= br.reserved2 )
            {
                br.reserved3 = false;
                param.next = m_pquant->next();
                return true;
            }

            ++br.reserved2;
            param.next = m_pquant->m_psub;

            return true;
        }
        
        bool _iterative_rematch_this( match_param<CI> & param ) const
        {
            backref_tag<CI> & br = ( *param.prgbackrefs )[ m_pquant->group_number() ];

            // infinite loop forcibly broken
            if( param.icur == param.pstack->top( static_init<CI>::value ) )
            {
                _pop_frame( param );
                return false;
            }

            if( br.reserved3 )
            {
                --br.reserved2;

                _pop_frame( param );
                return false;
            }

            br.reserved3 = true;

            if( m_pquant->m_ubound > br.reserved2 )
            {
                ++br.reserved2;
                param.next = m_pquant->m_psub;
                return true;
            }

            _pop_frame( param );
            return false;
        }
    public:
        end_quantifier( min_group_quantifier<CI> const * pquant = NULL )
            : m_pquant( pquant )
        {
        }
        
        virtual bool recursive_match_all_( match_param<CI> & param, CI icur ) const
        {
            return _recursive_match_all( param, icur, false_t() );
        }
        virtual bool recursive_match_all_c( match_param<CI> & param, CI icur ) const
        {
            return _recursive_match_all( param, icur, true_t() );
        }
        virtual bool iterative_match_this_( match_param<CI> & param ) const
        {
            return _iterative_match_this( param );
        }
        virtual bool iterative_match_this_c( match_param<CI> & param ) const
        {
            return _iterative_match_this( param );
        }
        virtual bool iterative_rematch_this_( match_param<CI> & param ) const
        {
            return _iterative_rematch_this( param );
        }
        virtual bool iterative_rematch_this_c( match_param<CI> & param ) const
        {
            return _iterative_rematch_this( param );
        }
        virtual width_type width_this( width_param<CI> & )
        {
            return zero_width;
        }
    } m_end_quant;

    friend class end_quantifier;
};

inline void fixup_backref( size_t & cbackref, std::list<size_t> const & invisible_groups )
{
    std::list<size_t>::const_iterator iter = invisible_groups.begin();

    for( ; iter != invisible_groups.end() && cbackref >= *iter; ++iter )
    {
        ++cbackref;
    }
}

template< typename CI >
class match_backref : public sub_expr<CI>
{

    bool _iterative_rematch_this( match_param<CI> & param ) const
    {
        backref_tag<CI> const & br = ( *param.prgbackrefs )[ m_cbackref ];
        ptrdiff_t dist = std::distance( br.first, br.second );
        std::advance( param.icur, ( int ) - dist );
        return false;
    }
public:
    match_backref( size_t cbackref )
        : m_cbackref( cbackref )
    {
    }

    // Return the width specifications of the group to which this backref refers
    virtual width_type width_this( width_param<CI> & param )
    {
        // fix up the backref to take into account the number of invisible groups
        if( param.first_pass() )
            fixup_backref( m_cbackref, param.invisible_groups );

        if( m_cbackref >= param.rggroups.size() )
            throw bad_regexpr( "reference to nonexistent group" );

        // If the entry in the backref vector has been nulled out, then we are
        // calculating the width for this group.
        if( NULL == param.rggroups[ m_cbackref ] )
            return worst_width; // can't tell how wide this group will be.  :-(

        return param.rggroups[ m_cbackref ]->width_this( param );
    }
    virtual bool iterative_rematch_this_( match_param<CI> & param ) const
    {
        return _iterative_rematch_this( param );
    }
    virtual bool iterative_rematch_this_c( match_param<CI> & param ) const
    {
        return _iterative_rematch_this( param );
    }
protected:
    size_t m_cbackref;
};

template< typename CMP, typename CI >
class match_backref_t : public match_backref<CI>
{
public:
    match_backref_t( size_t cbackref )
        : match_backref<CI>( cbackref ) 
    {
    }
    virtual sub_expr<CI> * quantify( size_t lbound, size_t ubound, bool greedy, regex_arena & arena )
    {
        if( greedy )
            return new( arena ) max_atom_quantifier<CI, match_backref_t<CMP, CI> >( this, lbound, ubound );
        else
            return new( arena ) min_atom_quantifier<CI, match_backref_t<CMP, CI> >( this, lbound, ubound );
    }
    virtual bool recursive_match_all_( match_param<CI> & param, CI icur ) const
    {
        return ( match_backref_t::recursive_match_this_( param, icur ) && recursive_match_next_( param, icur, false_t() ) );
    }
    virtual bool recursive_match_all_c( match_param<CI> & param, CI icur ) const
    {
        return ( match_backref_t::recursive_match_this_c( param, icur ) && recursive_match_next_( param, icur, true_t() ) );
    }
    virtual bool recursive_match_this_( match_param<CI> & param, CI & icur ) const
    {
        return _do_match_this( param, icur, false_t() );
    }
    virtual bool recursive_match_this_c( match_param<CI> & param, CI & icur ) const
    {
        return _do_match_this( param, icur, true_t() );
    }
    virtual bool iterative_match_this_( match_param<CI> & param ) const
    {
        param.next = next();
        return _do_match_this( param, param.icur, false_t() );
    }
    virtual bool iterative_match_this_c( match_param<CI> & param ) const
    {
        param.next = next();
        return _do_match_this( param, param.icur, true_t() );
    }
protected:
    template< typename CSTRINGS >
    bool _do_match_this( match_param<CI> & param, CI & icur, CSTRINGS ) const
    {
        // Pattern compilation should have failed if the following is false:
        assert( m_cbackref < param.prgbackrefs->size() );

        // Don't match a backref that hasn't match anything
        if( ! ( *param.prgbackrefs )[ m_cbackref ].matched )
            return false;

        CI ithis       = ( *param.prgbackrefs )[ m_cbackref ].first;
        CI const istop = ( *param.prgbackrefs )[ m_cbackref ].second;
        CI icur_tmp    = icur;

        for( ; istop != ithis; ++icur_tmp, ++ithis )
        {
            if( eos_t<CSTRINGS>::eval( param, icur_tmp ) || CMP::eval( *icur_tmp, *ithis ) )
                return false;
        }
        icur = icur_tmp;
        return true;
    }
};

template< typename CI >
inline match_backref<CI> * create_backref(
    size_t cbackref,
    REGEX_FLAGS flags, regex_arena & arena )
{
    typedef typename std::iterator_traits<CI>::value_type char_type;

    switch( NOCASE & flags )
    {
    case 0:
        return new( arena ) match_backref_t<ch_neq_t<char_type>, CI>( cbackref );
    case NOCASE:
        return new( arena ) match_backref_t<ch_neq_nocase_t<char_type>, CI>( cbackref );
    default:
        __assume( 0 ); // tells the compiler that this is unreachable
    }
}

template< typename CI >
class match_recurse : public sub_expr<CI>
{
    match_recurse & operator=( match_recurse const & );

    void _push_frame( match_param<CI> & param ) const
    {
        typedef typename match_param<CI>::backref_vector::const_iterator VCI;
        unsafe_stack * pstack = param.pstack;
        VCI istart = param.prgbackrefs->begin();
        VCI iend   = param.prgbackrefs->end();
        for( ; iend != istart; ++istart )
        {
            pstack->push( istart->reserved1 );
        }
    }

    void _pop_frame( match_param<CI> & param ) const
    {
        typedef typename match_param<CI>::backref_vector::iterator VI;
        unsafe_stack * pstack = param.pstack;
        VI istart = param.prgbackrefs->begin();
        VI iend   = param.prgbackrefs->end();
        while( iend != istart )
        {
            --iend;
            pstack->pop( iend->reserved1 );
        }
    }

    template< typename CSTRINGS >
    bool _recursive_match_all( match_param<CI> & param, CI icur, CSTRINGS ) const
    {
        // Prevent infinite recursion. If icur == ( *param.prgbackrefs )[ 0 ].reserved1,
        // then the pattern has eaten 0 chars to date, and we would recurse forever.
        if( icur == ( *param.prgbackrefs )[ 0 ].reserved1 )
            return recursive_match_next_( param, icur, CSTRINGS() );

        // copy the backref vector onto the stack
        CI * prgci = static_cast<CI*>( alloca( param.prgbackrefs->size() * sizeof( CI ) ) );
        save_backrefs<CI>( *param.prgbackrefs, prgci );

        // Recurse.
        if( param.first->recursive_match_all_( param, icur, CSTRINGS() ) )
        {
            // Restore the backref vector
            restore_backrefs<CI>( *param.prgbackrefs, prgci );

            // Recursive match succeeded. Try to match the rest of the pattern
            // using the end of the recursive match as the start of the next
            return recursive_match_next_( param, ( *param.prgbackrefs )[ 0 ].second, CSTRINGS() );
        }

        // Recursion failed
        return false;
    }
    template< typename CSTRINGS >
    bool _iterative_match_this( match_param<CI> & param, CSTRINGS ) const
    {
        param.pstack->push( param.icur );

        // Prevent infine recursion
        if( param.icur == ( *param.prgbackrefs )[ 0 ].reserved1 )
        {
            param.next = next();
            return true;
        }

        _push_frame( param );

        if( matcher_helper<CI>::_Do_match_iterative( param.first, param, param.icur, CSTRINGS() ) )
        {
            _pop_frame( param );
            param.next = next();
            return true;
        }

        _pop_frame( param );
        param.pstack->pop( param.icur );
        return false;
    }
    bool _iterative_rematch_this( match_param<CI> & param ) const
    {
        param.pstack->pop( param.icur );
        return false;
    }
public:
    match_recurse()
    {
    }

    virtual sub_expr<CI> * quantify( size_t, size_t, bool, regex_arena & )
    {
        throw bad_regexpr( "recursion sub-expression cannot be quantified" );
    }

    virtual bool recursive_match_all_( match_param<CI> & param, CI icur ) const
    {
        return _recursive_match_all( param, icur, false_t() );
    }
    virtual bool recursive_match_all_c( match_param<CI> & param, CI icur ) const
    {
        return _recursive_match_all( param, icur, true_t() );
    }
    virtual bool iterative_match_this_( match_param<CI> & param ) const
    {
        return _iterative_match_this( param, false_t() );
    }
    virtual bool iterative_match_this_c( match_param<CI> & param ) const
    {
        return _iterative_match_this( param, true_t() );
    }
    virtual bool iterative_rematch_this_( match_param<CI> & param ) const
    {
        return _iterative_rematch_this( param );
    }
    virtual bool iterative_rematch_this_c( match_param<CI> & param ) const
    {
        return _iterative_rematch_this( param );
    }
    virtual width_type width_this( width_param<CI> & param )
    {
        // We need to know how big the whole pattern is before we can say
        // how big a recursive match would be.
        if( param.first_pass() )
        {
            ++param.cookie;
            return zero_width;
        }
        width_type this_width = param.total_width;
        this_width.m_max = width_mult( this_width.m_max, size_t( -1 ) ); // could recurse forever
        return this_width;
    }
};

template< typename CI >
inline match_recurse<CI> * create_recurse( regex_arena & arena )
{
    return new( arena ) match_recurse<CI>();
}

template< typename CI >
struct backref_condition
{
    size_t m_cbackref;

    backref_condition( size_t cbackref )
        : m_cbackref( cbackref )
    {
    }

    template< typename CSTRINGS >
    bool recursive_match_this_( match_param<CI> & param, CI, CSTRINGS ) const
    {
        return m_cbackref < param.prgbackrefs->size() && ( *param.prgbackrefs )[ m_cbackref ].matched;
    }
    template< typename CSTRINGS >
    bool iterative_match_this_( match_param<CI> & param, CSTRINGS ) const
    {
        return m_cbackref < param.prgbackrefs->size() && ( *param.prgbackrefs )[ m_cbackref ].matched;
    }
    template< typename CSTRINGS >
    bool iterative_rematch_this_( match_param<CI> &, CSTRINGS ) const
    {
        return false;
    }
    void width_this( width_param<CI> & param )
    {
        // fix up the backref to take into account the number of invisible groups
        if( param.first_pass() )
            fixup_backref( m_cbackref, param.invisible_groups );
    }
};

template< typename CI >
struct assertion_condition
{
    std::auto_ptr<match_group_base<CI> > m_passert;

    assertion_condition( match_group_base<CI> * passert )
        : m_passert( passert )
    {
    }

    bool recursive_match_this_( match_param<CI> & param, CI icur, false_t ) const
    {
        return m_passert->recursive_match_all_( param, icur );
    }
    bool recursive_match_this_( match_param<CI> & param, CI icur, true_t ) const
    {
        return m_passert->recursive_match_all_c( param, icur );
    }
    bool iterative_match_this_( match_param<CI> & param, false_t ) const
    {
        return m_passert->iterative_match_this_( param );
    }
    bool iterative_match_this_( match_param<CI> & param, true_t ) const
    {
        return m_passert->iterative_match_this_c( param );
    }
    bool iterative_rematch_this_( match_param<CI> & param, false_t ) const
    {
        return m_passert->iterative_rematch_this_( param );
    }
    bool iterative_rematch_this_( match_param<CI> & param, true_t ) const
    {
        return m_passert->iterative_rematch_this_c( param );
    }
    void width_this( width_param<CI> & param )
    {
        ( void ) m_passert->width_this( param );
    }
};

template< typename CI, typename COND >
class match_conditional : public match_group<CI>
{
protected:
    typedef typename match_group<CI>::alt_list_type alt_list_type;

private:
    match_conditional & operator=( match_conditional const & );

    template< typename CSTRINGS >
    bool _recursive_match_all( match_param<CI> & param, CI icur, CSTRINGS ) const
    {
        typedef typename alt_list_type::const_iterator LCI;
        LCI ialt = m_rgalternates.begin();

        if( m_condition.recursive_match_this_( param, icur, CSTRINGS() ) || ++ialt != m_rgalternates.end() )
        {
            return (*ialt)->recursive_match_all_( param, icur, CSTRINGS() );
        }
        return recursive_match_next_( param, icur, CSTRINGS() );
    }
    template< typename CSTRINGS >
    bool _iterative_match_this( match_param<CI> & param, CSTRINGS ) const
    {
        typedef typename alt_list_type::const_iterator LCI;
        LCI ialt = m_rgalternates.begin();

        if( m_condition.iterative_match_this_( param, CSTRINGS() ) )
        {
            param.pstack->push( true );
            param.next = *ialt;
            return true;
        }
        param.pstack->push( false );
        param.next = ( ++ialt != m_rgalternates.end() ) ? *ialt : next();
        return true;
    }
    template< typename CSTRINGS >
    bool _iterative_rematch_this( match_param<CI> & param, CSTRINGS ) const
    {
        bool condition;
        param.pstack->pop( condition );
        if( condition )
            m_condition.iterative_rematch_this_( param, CSTRINGS() );
        return false;
    }
public:
    typedef COND condition_type;

    match_conditional( size_t cgroup, condition_type condition, regex_arena & arena )
        : match_group<CI>( cgroup, arena ),
          m_condition( condition )
    {
    }

    virtual bool recursive_match_all_( match_param<CI> & param, CI icur ) const
    {
        return _recursive_match_all( param, icur, false_t() );
    }
    virtual bool recursive_match_all_c( match_param<CI> & param, CI icur ) const
    {
        return _recursive_match_all( param, icur, true_t() );
    }
    virtual bool iterative_match_this_( match_param<CI> & param ) const
    {
        return _iterative_match_this( param, false_t() );
    }
    virtual bool iterative_match_this_c( match_param<CI> & param ) const
    {
        return _iterative_match_this( param, true_t() );
    }
    virtual bool iterative_rematch_this_( match_param<CI> & param ) const
    {
        return _iterative_rematch_this( param, false_t() );
    }
    virtual bool iterative_rematch_this_c( match_param<CI> & param ) const
    {
        return _iterative_rematch_this( param, true_t() );
    }
    virtual width_type width_this( width_param<CI> & param )
    {
        typedef typename alt_list_type::const_iterator LCI;
        LCI ialt = m_rgalternates.begin();

        width_type width = ( *ialt )->get_width( param );

        if( ++ialt != m_rgalternates.end() )
        {
            width_type temp_width = ( *ialt )->get_width( param );
            width.m_min = (std::min)( width.m_min, temp_width.m_min );
            width.m_max = (std::max)( width.m_max, temp_width.m_max );
        }
        else
        {
            width.m_min = 0;
        }

        // Have the condition calculate its width, too. This is important
        // if the condition is a lookbehind assertion.
        m_condition.width_this( param );

        return m_nwidth = width;
    }

protected:
    condition_type m_condition;
};

template< typename CI >
inline match_conditional<CI, backref_condition<CI> > * create_backref_conditional(
    size_t cgroup,
    size_t cbackref,
    regex_arena & arena )
{
    backref_condition<CI> cond( cbackref );
    return new( arena ) match_conditional<CI, backref_condition<CI> >(
        cgroup, cond, arena );
}

template< typename CI >
inline match_conditional<CI, assertion_condition<CI> > * create_assertion_conditional(
    size_t cgroup,
    match_group_base<CI> * passert,
    regex_arena & arena )
{
    assertion_condition<CI> cond( passert );
    return new( arena ) match_conditional<CI, assertion_condition<CI> >(
        cgroup, cond, arena );
}

// REGEX_ALLOCATOR is a #define which determines which allocator
// gets used as the STL-compliant allocator. When sub_expr objects
// contain STL containers as members, REGEX_ALLOCATOR is the allocator
// type used. (See the match_group and charset classes.) If REGEX_ALLOCATOR
// expands to regex_allocator, then *all* memory used when compiling
// a pattern ends up in a regex_arena.  In that case, destructors do
// not need to be called; we can just throw the arena away. However, if
// REGEX_ALLOCATOR expands to anything else (std::allocator, for instance)
// then some of the memory does not live in a regex_arena, and destructors
// do need to be called.  The following code enforces this logic.
template< typename AL >
struct skip_dtor_calls
{
    enum { value = false };
};
template<>
struct skip_dtor_calls< regex_allocator<char> >
{
    enum { value = true };
};

//
// From basic_rpattern_base_impl
//

template< typename CI >
REGEXPR_H_INLINE basic_rpattern_base_impl<CI>::~basic_rpattern_base_impl()
{
    if( skip_dtor_calls< REGEX_ALLOCATOR<char> >::value )
    {
        m_pfirst.release();
    }
    assign_auto_ptr( m_pfirst, static_cast<sub_expr_base<CI>const*>(0) );
    m_arena.deallocate();
}

template< typename CI >
REGEXPR_H_INLINE bool basic_rpattern_base_impl<CI>::_do_match( match_param<CI> & param, bool use_null ) const
{
    if( GLOBAL & flags() ) // do a global find
    {
        // The NOBACKREFS flag is ignored in the match method.
        bool const fAll   = ( ALLBACKREFS   == ( ALLBACKREFS   & flags() ) );
        bool const fFirst = ( FIRSTBACKREFS == ( FIRSTBACKREFS & flags() ) );

        backref_vector rgtempbackrefs;

        while( matcher_helper<CI>::_Do_match( *this, param, use_null ) )
        {
            backref_type const & br = ( *param.prgbackrefs )[0];

            // Handle specially the backref flags
            if( fFirst )
                rgtempbackrefs.push_back( br );
            else if( fAll )
                rgtempbackrefs.insert(
                    rgtempbackrefs.end(),
                    param.prgbackrefs->begin(),
                    param.prgbackrefs->end() );
            else
                rgtempbackrefs.swap( *param.prgbackrefs );

            param.istart = br.second;
            param.no0len = ( br.first == br.second );
        }

        // restore the backref vectors
        param.prgbackrefs->swap( rgtempbackrefs );
        return ! param.prgbackrefs->empty();
    }
    else
        return matcher_helper<CI>::_Do_match( *this, param, use_null );
}

template< typename CI >
REGEXPR_H_INLINE bool basic_rpattern_base_impl<CI>::_do_match( match_param<CI> & param, char_type const * szbegin ) const
{
    if( RIGHTMOST & flags() )
    {
        // We need to know the end of the string if we're doing a
        // RIGHTMOST match
        param.istop = param.istart;
        std::advance( param.istop, traits_type::length( szbegin ) );
        return basic_rpattern_base_impl<CI>::_do_match( param, false );
    }
    return basic_rpattern_base_impl<CI>::_do_match( param, true );
}

template< typename CI >
REGEXPR_H_INLINE size_t basic_rpattern_base_impl<CI>::_do_count( match_param<CI> & param, bool use_null ) const
{
    size_t cmatches = 0;

    while( matcher_helper<CI>::_Do_match( *this, param, use_null ) )
    {
        backref_type const & br = ( *param.prgbackrefs )[0];
        ++cmatches;
        param.istart = br.second;

        param.no0len = ( br.first == br.second );
    }
    return cmatches;
}

template< typename CI >
REGEXPR_H_INLINE size_t basic_rpattern_base_impl<CI>::_do_count( match_param<CI> & param, char_type const * szbegin ) const
{
    if( RIGHTMOST & flags() )
    {
        // We need to know the end of the string if we're doing a
        // RIGHTMOST count
        param.istop = param.istart;
        std::advance( param.istop, traits_type::length( szbegin ) );
        return basic_rpattern_base_impl<CI>::_do_count( param, false );
    }
    return basic_rpattern_base_impl<CI>::_do_count( param, true );
}

// A helper class for automatically deallocating the arena when
// parsing the pattern results in an exception
struct deallocation_helper
{
    deallocation_helper( regex_arena & arena )
        : m_arena( arena ),
          m_fparse_successful( false )
    {
    }

    ~deallocation_helper()
    {
        if( ! m_fparse_successful )
            m_arena.deallocate();
    }
    
    void dismiss()
    {
        m_fparse_successful = true;
    }
private:
    deallocation_helper & operator=( deallocation_helper const & );

    regex_arena & m_arena;
    bool m_fparse_successful;
};

} // namespace detail

//
// Implementation of basic_rpattern_base:
//

template< typename CI, typename SY >
REGEXPR_H_INLINE void basic_rpattern_base<CI, SY>::init( string_type const & pat, REGEX_FLAGS flags, REGEX_MODE mode )
{
    basic_rpattern_base<CI, SY> temp( pat, flags, mode );
    swap( temp );
}

template< typename CI, typename SY >
REGEXPR_H_INLINE void basic_rpattern_base<CI, SY>::init( string_type const & pat, string_type const & subst, REGEX_FLAGS flags, REGEX_MODE mode )
{
    basic_rpattern_base<CI, SY> temp( pat, subst, flags, mode );
    swap( temp );
}

template< typename CI, typename SY >
REGEXPR_H_INLINE void basic_rpattern_base<CI, SY>::_common_init( REGEX_FLAGS flags )
{
    m_cgroups = 0;
    std::vector<detail::match_group_base<CI>*> rggroups;
    typename string_type::iterator ipat = m_pat->begin();
    iter_wrap iw( ipat );
    syntax_type sy( flags );
    detail::match_group_base<CI> * pgroup;

    // Set up a sentry that will free the arena memory
    // automatically on parse failure.
    {
        detail::deallocation_helper parse_sentry( m_arena );

        // This will throw on failure
        pgroup = _find_next_group( iw, NULL, sy, rggroups );

        // Note that the parse was successful
        parse_sentry.dismiss();
    }

    assert( NULL == m_pfirst.get() );
    detail::assign_auto_ptr( m_pfirst, pgroup );

    // Calculate the width of the pattern and all groups
    m_nwidth = pgroup->group_width( rggroups, m_invisible_groups );

    //
    // determine if we can get away with only calling m_pfirst->recursive_match_all_ only once
    //

    m_floop = true;

    // Optimization: if first character of pattern string is '^'
    // and we are not doing a multiline match, then we only
    // need to try recursive_match_all_ once
    typename string_type::iterator icur = m_pat->begin();
    if( MULTILINE != ( MULTILINE & m_flags ) &&
        1 == pgroup->calternates() &&
        icur != m_pat->end() &&
        BEGIN_LINE == sy.reg_token( icur, m_pat->end() ) )
    {
        m_flags = ( REGEX_FLAGS ) ( m_flags & ~RIGHTMOST );
        m_floop = false;
    }

    // Optimization: if first 2 characters of pattern string are ".*" or ".+",
    // then we only need to try recursive_match_all_ once
    icur = m_pat->begin();
    if( RIGHTMOST != ( RIGHTMOST & m_flags ) &&
        SINGLELINE == ( SINGLELINE & m_flags ) &&
        1 == pgroup->calternates() &&
        icur != m_pat->end() &&
        MATCH_ANY == sy.reg_token( icur, m_pat->end() ) &&
        icur != m_pat->end() )
    {
        switch( sy.quant_token( icur, m_pat->end() ) )
        {
        case ONE_OR_MORE:
        case ZERO_OR_MORE:
        case ONE_OR_MORE_MIN:
        case ZERO_OR_MORE_MIN:
            m_floop = false;
        }
    }
}

template< typename CI, typename SY >
REGEXPR_H_INLINE void basic_rpattern_base<CI, SY>::set_substitution( string_type const & subst )
{
    std::auto_ptr<string_type> temp_subst( new string_type( subst ) );
    detail::subst_list_type temp_subst_list;
    bool uses_backrefs = false;

    _normalize_string( *temp_subst );
    basic_rpattern_base<CI, SY>::_parse_subst( *temp_subst, uses_backrefs, temp_subst_list );

    detail::swap_auto_ptr( temp_subst, m_subst );
    std::swap( uses_backrefs, m_fuses_backrefs );
    temp_subst_list.swap( m_subst_list );
}

template< typename CI, typename SY >
inline detail::match_group_base<CI> * basic_rpattern_base<CI, SY>::_find_next_group(
    iter_wrap & iw,
    detail::match_group_base<CI> * pgroup_enclosing, syntax_type & sy,
    std::vector<detail::match_group_base<CI>*> & rggroups )
{
    std::auto_ptr<detail::match_group_base<CI> > pgroup;
    typename string_type::iterator itemp = iw.ipat;
    REGEX_FLAGS old_flags = sy.get_flags();
    TOKEN tok;
    size_t extent_start = m_cgroups;
    bool fconditional = false;

    // Look for group extensions.
    if( m_pat->end() != iw.ipat && NO_TOKEN != ( tok = sy.ext_token( iw.ipat, m_pat->end() ) ) )
    {
        if( m_pat->begin() == itemp || m_pat->end() == iw.ipat )
            throw bad_regexpr( "ill-formed regular expression" );

        // Is this a recursion element?
        if( EXT_RECURSE == tok )
        {
            pgroup_enclosing->add_item( detail::create_recurse<CI>( m_arena ) );

            // This pattern could recurse deeply. Note that fact here so that
            // we can opt to use a stack-conservative algorithm at match time.
            m_fok_to_recurse = false;
        }

        // Don't process empty groups like (?:) or (?i) or (?R)
        if( END_GROUP != sy.reg_token( itemp = iw.ipat, m_pat->end() ) )
        {
            switch( tok )
            {
            case EXT_NOBACKREF:
                // note that this group is not visible, so we can fix
                // up offsets into the backref vector later
                m_invisible_groups.push_back( m_cgroups );
                detail::assign_auto_ptr( pgroup, new( m_arena ) detail::match_group<CI>( _get_next_group_nbr(), m_arena ) );
                break;

            case EXT_INDEPENDENT:
                m_invisible_groups.push_back( m_cgroups );
                detail::assign_auto_ptr( pgroup, new( m_arena ) detail::independent_group<CI>( _get_next_group_nbr(), m_arena ) );
                break;

            case EXT_POS_LOOKAHEAD:
                detail::assign_auto_ptr( pgroup, new( m_arena ) detail::lookahead_assertion<CI>( true, m_arena ) );
                break;

            case EXT_NEG_LOOKAHEAD:
                detail::assign_auto_ptr( pgroup, new( m_arena ) detail::lookahead_assertion<CI>( false, m_arena ) );
                break;

            case EXT_POS_LOOKBEHIND:
                detail::assign_auto_ptr( pgroup, new( m_arena ) detail::lookbehind_assertion<CI>( true, m_arena ) );
                break;

            case EXT_NEG_LOOKBEHIND:
                detail::assign_auto_ptr( pgroup, new( m_arena ) detail::lookbehind_assertion<CI>( false, m_arena ) );
                break;

            case EXT_CONDITION:
                fconditional = true;
                m_invisible_groups.push_back( m_cgroups );

                if( size_t cbackref = detail::parse_int( iw.ipat, m_pat->end() ) &&
                    END_GROUP == sy.reg_token( iw.ipat, m_pat->end() ) )
                {
                    detail::assign_auto_ptr( 
                        pgroup, detail::create_backref_conditional<CI>( 
                            _get_next_group_nbr(), cbackref, m_arena ) );
                }
                else
                {
                    switch( sy.ext_token( itemp = iw.ipat, m_pat->end() ) )
                    {
                    case EXT_POS_LOOKAHEAD:
                    case EXT_NEG_LOOKAHEAD:
                    case EXT_POS_LOOKBEHIND:
                    case EXT_NEG_LOOKBEHIND:
                        {
                            std::auto_ptr<detail::match_group_base<CI> > pgroup_tmp( 
                                _find_next_group( iw, NULL, sy, rggroups ) );
                            detail::assign_auto_ptr( 
                                pgroup, detail::create_assertion_conditional<CI>( 
                                    _get_next_group_nbr(), pgroup_tmp.get(), m_arena ) );
                            pgroup_tmp.release();
                        }
                        break;
                    default:
                        throw bad_regexpr( "bad extension sequence" );
                    }
                }
                break;

            case EXT_COMMENT:
                while( END_GROUP != ( tok = sy.reg_token( iw.ipat, m_pat->end() ) ) )
                {
                    if( NO_TOKEN == tok && m_pat->end() != iw.ipat )
                        ++iw.ipat;
                    if( m_pat->end() == iw.ipat )
                        throw bad_regexpr( "Expecting end of comment" );
                }
                break;

            default:
                throw bad_regexpr( "bad extension sequence" );
            }
        }
        else
        {
            // Skip over the END_GROUP token
            iw.ipat = itemp;
        }
    }
    else
    {
        detail::assign_auto_ptr( pgroup, new( m_arena ) detail::match_group<CI>( _get_next_group_nbr(), m_arena ) );
        ++m_cgroups_visible;
    }

    if( NULL != pgroup.get() )
    {
        pgroup->add_alternate();
        while( _find_next( iw, pgroup.get(), sy, rggroups ) );
        pgroup->end_alternate();

        // if this is a conditional group, then there must be at
        // most 2 alternates.
        if( fconditional && 2 < pgroup->calternates() )
            throw bad_regexpr( "Too many alternates in conditional subexpression" );

        // Add this group to the rggroups array
        if( size_t( -1 ) != pgroup->group_number() )
        {
            if( pgroup->group_number() >= rggroups.size() )
                rggroups.resize( pgroup->group_number() + 1, NULL );
            rggroups[ pgroup->group_number() ] = pgroup.get();
        }

        // tell this group how many groups are contained within it
        pgroup->set_extent( detail::extent( extent_start, m_cgroups - extent_start ) );

        // If this is not a pattern modifier, restore the
        // flags to their previous settings.  This causes
        // pattern modifiers to have the scope of their
        // enclosing group.
        sy.set_flags( old_flags );
    }

    return pgroup.release();
}

namespace detail
{

// If we reached the end of the string before finding the end of the
// character set, then this is an ill-formed regex
template< typename CI >
inline void check_iter( CI icur, CI istop )
{
    if( istop == icur )
        throw bad_regexpr( "expecting end of character set" );
}

template< typename II, typename CI >
inline typename std::iterator_traits<CI>::value_type get_escaped_char( II & icur, CI iend, bool normalize )
{
    typedef typename std::iterator_traits<CI>::value_type CH;
    CH ch = 0, i;
    check_iter<CI>( icur, iend );

    switch( *icur )
    {
    // octal escape sequence
    case REGEX_CHAR(CH,'0'): case REGEX_CHAR(CH,'1'): case REGEX_CHAR(CH,'2'): case REGEX_CHAR(CH,'3'):
    case REGEX_CHAR(CH,'4'): case REGEX_CHAR(CH,'5'): case REGEX_CHAR(CH,'6'): case REGEX_CHAR(CH,'7'):
        ch = CH( *icur++ - REGEX_CHAR(CH,'0') );
        for( i=0; i<2 && REGEX_CHAR(CH,'0') <= *icur && REGEX_CHAR(CH,'7') >= *icur; check_iter<CI>( ++icur, iend ) )
            ch = CH( ch * 8 + ( *icur - REGEX_CHAR(CH,'0') ) );
        break;
    // bell character
    case REGEX_CHAR(CH,'a'):
        if( ! normalize )
            goto default_;
        ch = REGEX_CHAR(CH,'\a');
        ++icur;
        break;
    // control character
    case REGEX_CHAR(CH,'c'):
        check_iter<CI>( ++icur, iend );
        ch = *icur++;
        if( REGEX_CHAR(CH,'a') <= ch && REGEX_CHAR(CH,'z') >= ch )
            ch = detail::regex_toupper( ch );
        ch ^= 0x40;
        break;
    // escape character
    case REGEX_CHAR(CH,'e'):
        ch = 27;
        ++icur;
        break;
    // formfeed character
    case REGEX_CHAR(CH,'f'):
        if( ! normalize )
            goto default_;
        ch = REGEX_CHAR(CH,'\f');
        ++icur;
        break;
    // newline
    case REGEX_CHAR(CH,'n'):
        if( ! normalize )
            goto default_;
        ch = REGEX_CHAR(CH,'\n');
        ++icur;
        break;
    // return
    case REGEX_CHAR(CH,'r'):
        if( ! normalize )
            goto default_;
        ch = REGEX_CHAR(CH,'\r');
        ++icur;
        break;
    // horizontal tab
    case REGEX_CHAR(CH,'t'):
        if( ! normalize )
            goto default_;
        ch = REGEX_CHAR(CH,'\t');
        ++icur;
        break;
    // vertical tab
    case REGEX_CHAR(CH,'v'):
        if( ! normalize )
            goto default_;
        ch = REGEX_CHAR(CH,'\v');
        ++icur;
        break;
    // hex escape sequence
    case REGEX_CHAR(CH,'x'):
        for( ++icur, ch=i=0; i<2 && detail::regex_isxdigit( *icur ); check_iter<CI>( ++icur, iend ) )
            ch = CH( ch * 16 + detail::regex_xdigit2int( *icur ) );
        break;
    // backslash
    case REGEX_CHAR(CH,'\\'):
        if( ! normalize )
            goto default_;
        ch = REGEX_CHAR(CH,'\\');
        ++icur;
        break;
    // all other escaped characters represent themselves
    default: default_:
        ch = *icur;
        ++icur;
        break;
    }

    return ch;
}

template< typename CH, typename CS, typename SY >
inline void parse_charset(
    std::auto_ptr<CS> & pnew,
    typename std::basic_string<CH>::iterator & icur,
    typename std::basic_string<CH>::const_iterator iend,
    SY & sy )
{
    typedef CH char_type;
    typedef std::basic_string<CH> string_type;
    typedef typename string_type::const_iterator CI;
    typename string_type::iterator itemp = icur;
    bool const normalize = ( NORMALIZE == ( NORMALIZE & sy.get_flags() ) );

    if( iend != itemp && CHARSET_NEGATE == sy.charset_token( itemp, iend ) )
    {
        pnew->m_fcompliment = true;
        icur = itemp;
    }

    TOKEN tok;
    char_type ch_prev = 0;
    bool fhave_prev = false;
    charset const * pcharset = NULL;
    typename string_type::iterator iprev = icur;
    bool const fnocase = ( NOCASE == ( NOCASE & sy.get_flags() ) );

    check_iter<CI>( icur, iend );

    // remember the current position and grab the next token
    tok = sy.charset_token( icur, iend );
    do
    {
        check_iter<CI>( icur, iend );

        if( CHARSET_RANGE == tok && fhave_prev )
        {
            // remember the current position
            typename string_type::iterator iprev2 = icur;
            fhave_prev = false;

            // ch_prev is lower bound of a range
            switch( sy.charset_token( icur, iend ) )
            {
            case CHARSET_RANGE:
            case CHARSET_NEGATE:
                icur = iprev2; // un-get these tokens and fall through
            case NO_TOKEN:
                pnew->set_bit_range( ch_prev, *icur++, fnocase );
                continue;
            case CHARSET_ESCAPE: // BUGBUG user-defined charset?
                pnew->set_bit_range( ch_prev, get_escaped_char( icur, iend, normalize ), fnocase );
                continue;
            case CHARSET_BACKSPACE:
                pnew->set_bit_range( ch_prev, char_type( 8 ), fnocase ); // backspace
                continue;
            case CHARSET_END: // fall through
            default:          // not a range.
                icur = iprev; // backup to range token
                pnew->set_bit( ch_prev, fnocase );
                pnew->set_bit( *icur++, fnocase );
                continue;
            }
        }

        if( fhave_prev )
            pnew->set_bit( ch_prev, fnocase );
        fhave_prev = false;

        switch( tok )
        {
            // None of the intrinsic charsets are case-sensitive,
            // so no special handling must be done when the NOCASE
            // flag is set.
        case CHARSET_RANGE:
        case CHARSET_NEGATE:
        case CHARSET_END:
            icur = iprev; // un-get these tokens
            ch_prev = *icur++;
            fhave_prev = true;
            continue;
        case CHARSET_BACKSPACE:
            ch_prev = char_type( 8 ); // backspace
            fhave_prev = true;
            continue;
        case ESC_DIGIT:
            *pnew |= intrinsic_charsets<char_type>::get_digit_charset();
            continue;
        case ESC_NOT_DIGIT:
            *pnew |= intrinsic_charsets<char_type>::get_not_digit_charset();
            continue;
        case ESC_SPACE:
            *pnew |= intrinsic_charsets<char_type>::get_space_charset();
            continue;
        case ESC_NOT_SPACE:
            *pnew |= intrinsic_charsets<char_type>::get_not_space_charset();
            continue;
        case ESC_WORD:
            *pnew |= intrinsic_charsets<char_type>::get_word_charset();
            continue;
        case ESC_NOT_WORD:
            *pnew |= intrinsic_charsets<char_type>::get_not_word_charset();
            continue;
        case CHARSET_ALNUM:
            pnew->m_posixcharson |= ( _ALNUM );
            continue;
        case CHARSET_NOT_ALNUM:
            pnew->m_posixcharsoff.push_back( _ALNUM );
            continue;
        case CHARSET_ALPHA:
            pnew->m_posixcharson |= ( _ALPHA );
            continue;
        case CHARSET_NOT_ALPHA:
            pnew->m_posixcharsoff.push_back( _ALPHA );
            continue;
        case CHARSET_BLANK:
            pnew->m_posixcharson |= ( _BLANK );
            continue;
        case CHARSET_NOT_BLANK:
            pnew->m_posixcharsoff.push_back( _BLANK );
            continue;
        case CHARSET_CNTRL:
            pnew->m_posixcharson |= ( _CONTROL );
            continue;
        case CHARSET_NOT_CNTRL:
            pnew->m_posixcharsoff.push_back( _CONTROL );
            continue;
        case CHARSET_DIGIT:
            pnew->m_posixcharson |= ( _DIGIT );
            continue;
        case CHARSET_NOT_DIGIT:
            pnew->m_posixcharsoff.push_back( _DIGIT );
            continue;
        case CHARSET_GRAPH:
            pnew->m_posixcharson |= ( _GRAPH );
            continue;
        case CHARSET_NOT_GRAPH:
            pnew->m_posixcharsoff.push_back( _GRAPH );
            continue;
        case CHARSET_LOWER:
            if( NOCASE == ( NOCASE & sy.get_flags() ) )
                pnew->m_posixcharson |= ( _LOWER|_UPPER );
            else
                pnew->m_posixcharson |= ( _LOWER );
            continue;
        case CHARSET_NOT_LOWER:
            if( NOCASE == ( NOCASE & sy.get_flags() ) )
                pnew->m_posixcharsoff.push_back( _LOWER|_UPPER );
            else
                pnew->m_posixcharsoff.push_back( _LOWER );
            continue;
        case CHARSET_PRINT:
            pnew->m_posixcharson |= ( _PRINT );
            continue;
        case CHARSET_NOT_PRINT:
            pnew->m_posixcharsoff.push_back( _PRINT );
            continue;
        case CHARSET_PUNCT:
            pnew->m_posixcharson |= ( _PUNCT );
            continue;
        case CHARSET_NOT_PUNCT:
            pnew->m_posixcharsoff.push_back( _PUNCT );
            continue;
        case CHARSET_SPACE:
            pnew->m_posixcharson |= ( _SPACE );
            continue;
        case CHARSET_NOT_SPACE:
            pnew->m_posixcharsoff.push_back( _SPACE );
            continue;
        case CHARSET_UPPER:
            if( NOCASE == ( NOCASE & sy.get_flags() ) )
                pnew->m_posixcharson |= ( _UPPER|_LOWER );
            else
                pnew->m_posixcharson |= ( _UPPER );
            continue;
        case CHARSET_NOT_UPPER:
            if( NOCASE == ( NOCASE & sy.get_flags() ) )
                pnew->m_posixcharsoff.push_back( _UPPER|_LOWER );
            else
                pnew->m_posixcharsoff.push_back( _UPPER );
            continue;
        case CHARSET_XDIGIT:
            pnew->m_posixcharson |= ( _HEX );
            continue;
        case CHARSET_NOT_XDIGIT:
            pnew->m_posixcharsoff.push_back( _HEX );
            continue;
        case CHARSET_ESCAPE:
            // Maybe this is a user-defined intrinsic charset
            pcharset = get_altern_charset( *icur, sy );
            if( NULL != pcharset )
            {
                 *pnew |= *pcharset;
                 ++icur;
                 continue;
            }
            else
            {
                ch_prev = get_escaped_char( icur, iend, normalize );
                fhave_prev = true;
            }
            continue;
        default:
            ch_prev = *icur++;
            fhave_prev = true;
            continue;
        }
    }
    while( check_iter<CI>( iprev = icur, iend ),
           CHARSET_END != ( tok = sy.charset_token( icur, iend ) ) );

    if( fhave_prev )
        pnew->set_bit( ch_prev, fnocase );

    pnew->optimize( type2type<char_type>() );
}

template< typename CH, typename SY >
inline charset const * get_altern_charset( CH ch, SY & sy )
{
    typedef std::basic_string<CH> string_type;
    charset const * pcharset = NULL;
    regex::detail::charset_map<CH> & charset_map = sy.s_charset_map;
    typename regex::detail::charset_map<CH>::iterator iter = charset_map.find( ch );
    if( charset_map.end() != iter )
    {
        bool const fnocase = ( NOCASE == ( sy.get_flags() & NOCASE ) );
        pcharset = iter->second.m_rgcharsets[ fnocase ];
        if( NULL == pcharset )
        {
            // tmp takes ownership of any ptrs.
            charset_map_node<CH> tmp = iter->second;
            charset_map.erase( iter ); // prevent possible infinite recursion
            typename string_type::iterator istart = tmp.m_str.begin();
            std::auto_ptr<charset> pnew( new charset );
            std::auto_ptr<charset const> pold( tmp.m_rgcharsets[ !fnocase ] );
            parse_charset<CH, charset>( pnew, istart, tmp.m_str.end(), sy );
            tmp.m_rgcharsets[ fnocase ] = pcharset = pnew.get();
            charset_map[ ch ] = tmp; // could throw
            // charset_map has taken ownership of these pointers now.
            pnew.release();
            pold.release();
        }
    }
    return pcharset;
}

} // namespace detail

//
// Read ahead through the pattern and treat sequential atoms
// as a single atom, making sure to handle quantification
// correctly. Warning: dense code ahead.
//
template< typename CI, typename SY >
inline void basic_rpattern_base<CI, SY>::_find_atom(
    iter_wrap & iw,
    detail::match_group_base<CI> * pgroup,
    syntax_type & sy )
{
    typename string_type::iterator itemp = iw.ipat, istart;
    size_t const nstart = std::distance( m_pat->begin(), iw.ipat );

    do
    {
        if( itemp != iw.ipat ) // Is there whitespace to skip?
        {
            size_t dist = std::distance( m_pat->begin(), iw.ipat );
            m_pat->erase( iw.ipat, itemp ); // erase the whitespace from the patttern
            std::advance( iw.ipat = m_pat->begin(), dist );
            if( m_pat->end() == ( itemp = iw.ipat ) ) // are we at the end of the pattern?
                break;
        }
        switch( sy.quant_token( itemp, m_pat->end() ) )
        {
            // if {, } can't be interpreted as quantifiers, treat them as regular chars
        case BEGIN_RANGE:
            std::advance( istart = m_pat->begin(), nstart );
            if( istart != iw.ipat ) // treat as a quantifier
                goto quantify;
        case NO_TOKEN:
        case END_RANGE:
        case END_RANGE_MIN:
        case RANGE_SEPARATOR:
            break;

        default:
            std::advance( istart = m_pat->begin(), nstart );
            if( istart == iw.ipat ) // must be able to quantify something.
                throw bad_regexpr( "quantifier not expected" );

quantify:   if( istart != --iw.ipat )
                pgroup->add_item( detail::create_literal<CI>( istart, iw.ipat, sy.get_flags(), m_arena ) );
            std::auto_ptr<detail::sub_expr<CI> > pnew( detail::create_char<CI>( *iw.ipat++, sy.get_flags(), m_arena ) );
            _quantify( pnew, iw, false, sy );
            pgroup->add_item( pnew.release() );
            return;
        }
    } while( m_pat->end() != ++iw.ipat && ! sy.reg_token( itemp = iw.ipat, m_pat->end() ) );

    std::advance( istart = m_pat->begin(), nstart );
    assert( iw.ipat != istart );
    pgroup->add_item( detail::create_literal<CI>( istart, iw.ipat, sy.get_flags(), m_arena ) );
}

template< typename CI, typename SY >
inline bool basic_rpattern_base<CI, SY>::_find_next(
    iter_wrap & iw,
    detail::match_group_base<CI> * pgroup,
    syntax_type & sy,
    std::vector<detail::match_group_base<CI>*> & rggroups )
{
    typedef char_type CH;
    std::auto_ptr<detail::sub_expr<CI> > pnew;
    std::auto_ptr<detail::custom_charset> pcs;
    typename string_type::iterator istart, itemp;
    bool fdone, is_group = false;
    bool const normalize = ( NORMALIZE == ( NORMALIZE & sy.get_flags() ) );

    if( m_pat->end() == iw.ipat )
    {
        if( 0 != pgroup->group_number() )
            throw bad_regexpr( "mismatched parenthesis" );
        return false;
    }

    switch( sy.reg_token( iw.ipat, m_pat->end() ) )
    {
    case NO_TOKEN: // not a token. Must be an atom
        if( m_pat->end() == iw.ipat )
        {
            if( 0 != pgroup->group_number() )
                throw bad_regexpr( "mismatched parenthesis" );
            return false;
        }
        _find_atom( iw, pgroup, sy );
        return true;

    case END_GROUP:
        if( 0 == pgroup->group_number() )
            throw bad_regexpr( "mismatched parenthesis" );
        return false;

    case ALTERNATION:
        pgroup->end_alternate();
        pgroup->add_alternate();
        return true;

    case BEGIN_GROUP:
        // Find next group. could return NULL if the group is really
        // a pattern modifier, like: ( ?s-i )
        detail::assign_auto_ptr( pnew, _find_next_group( iw, pgroup, sy, rggroups ) );
        is_group = true;
        break;

    case BEGIN_LINE:
        detail::assign_auto_ptr( pnew, detail::create_bol<CI>( sy.get_flags(), m_arena ) );
        break;

    case END_LINE:
        detail::assign_auto_ptr( pnew, detail::create_eol<CI>( sy.get_flags(), m_arena ) );
        break;

    case BEGIN_CHARSET:
        detail::assign_auto_ptr( pcs, new( m_arena ) detail::custom_charset( m_arena ) );
        detail::parse_charset<char_type, detail::custom_charset>(
            pcs, iw.ipat, m_pat->end(), sy );
        detail::assign_auto_ptr( pnew,
            detail::create_custom_charset<CI>( pcs.get(), sy.get_flags(), m_arena ) );
        pcs.release();
        break;

    case MATCH_ANY:
        detail::assign_auto_ptr( pnew, detail::create_any<CI>( sy.get_flags(), m_arena ) );
        break;

    case ESC_WORD_BOUNDARY:
        detail::assign_auto_ptr( pnew, detail::create_word_boundary<CI>( true, sy.get_flags(), m_arena ) );
        break;

    case ESC_NOT_WORD_BOUNDARY:
        detail::assign_auto_ptr( pnew, detail::create_word_boundary<CI>( false, sy.get_flags(), m_arena ) );
        break;

    case ESC_WORD_START:
        detail::assign_auto_ptr( pnew, detail::create_word_start<CI>( sy.get_flags(), m_arena ) );
        break;

    case ESC_WORD_STOP:
        detail::assign_auto_ptr( pnew, detail::create_word_stop<CI>( sy.get_flags(), m_arena ) );
        break;

    case ESC_DIGIT:
        detail::assign_auto_ptr( pnew, detail::create_charset<CI>( detail::intrinsic_charsets<char_type>::get_digit_charset(), sy.get_flags(), m_arena ) );
        break;

    case ESC_NOT_DIGIT:
        detail::assign_auto_ptr( pnew, detail::create_charset<CI>( detail::intrinsic_charsets<char_type>::get_not_digit_charset(), sy.get_flags(), m_arena ) );
        break;

    case ESC_WORD:
        detail::assign_auto_ptr( pnew, detail::create_charset<CI>( detail::intrinsic_charsets<char_type>::get_word_charset(), sy.get_flags(), m_arena ) );
        break;

    case ESC_NOT_WORD:
        detail::assign_auto_ptr( pnew, detail::create_charset<CI>( detail::intrinsic_charsets<char_type>::get_not_word_charset(), sy.get_flags(), m_arena ) );
        break;

    case ESC_SPACE:
        detail::assign_auto_ptr( pnew, detail::create_charset<CI>( detail::intrinsic_charsets<char_type>::get_space_charset(), sy.get_flags(), m_arena ) );
        break;

    case ESC_NOT_SPACE:
        detail::assign_auto_ptr( pnew, detail::create_charset<CI>( detail::intrinsic_charsets<char_type>::get_not_space_charset(), sy.get_flags(), m_arena ) );
        break;

    case ESC_BEGIN_STRING:
        detail::assign_auto_ptr( pnew, detail::create_bos<CI>( sy.get_flags(), m_arena ) );
        break;

    case ESC_END_STRING:
        detail::assign_auto_ptr( pnew, detail::create_eos<CI>( sy.get_flags(), m_arena ) );
        break;

    case ESC_END_STRING_z:
        detail::assign_auto_ptr( pnew, detail::create_eoz<CI>( sy.get_flags(), m_arena ) );
        break;

    case ESCAPE:
        if( m_pat->end() == iw.ipat )
        {
            // BUGBUG what if the escape sequence is more that 1 character?
            detail::assign_auto_ptr( pnew, detail::create_char<CI>( *--iw.ipat, sy.get_flags(), m_arena ) );
            ++iw.ipat;
        }
        else if( REGEX_CHAR(CH,'0') <= *iw.ipat && REGEX_CHAR(CH,'9') >= *iw.ipat )
        {
            // Parse at most 3 decimal digits.
            size_t nbackref = detail::parse_int( itemp = iw.ipat, m_pat->end(), 999 );
            // If the resulting number could conceivably be a backref, then it is.
            if( REGEX_CHAR(CH,'0') != *iw.ipat && ( 10 > nbackref || nbackref < _cgroups_total() ) )
            {
                detail::assign_auto_ptr( pnew, detail::create_backref<CI>( nbackref, sy.get_flags(), m_arena ) );
                iw.ipat = itemp;
            }
            else
            {
                // It's an octal character escape sequence. If *ipat is 8 or 9, insert
                // a NULL character, and leave the 8 or 9 as a character literal.
                char_type ch = 0, i = 0;
                for( ; i < 3 && m_pat->end() != iw.ipat && REGEX_CHAR(CH,'0') <= *iw.ipat && REGEX_CHAR(CH,'7') >= *iw.ipat; ++i, ++iw.ipat )
                    ch = CH( ch * 8 + ( *iw.ipat - REGEX_CHAR(CH,'0') ) );
                detail::assign_auto_ptr( pnew, detail::create_char<CI>( ch, sy.get_flags(), m_arena ) );
            }
        }
        else if( REGEX_CHAR(CH,'e') == *iw.ipat )
        {
            ++iw.ipat;
            detail::assign_auto_ptr( pnew, detail::create_char<CI>( CH( 27 ), sy.get_flags(), m_arena ) );
        }
        else if( REGEX_CHAR(CH,'x') == *iw.ipat )
        {
            char_type ch = 0, i = 0;
            for( ++iw.ipat; i < 2 && m_pat->end() != iw.ipat && detail::regex_isxdigit( *iw.ipat ); ++i, ++iw.ipat )
                ch = CH( ch * 16 + detail::regex_xdigit2int( *iw.ipat ) );
            detail::assign_auto_ptr( pnew, detail::create_char<CI>( ch, sy.get_flags(), m_arena ) );
        }
        else if( REGEX_CHAR(CH,'c') == *iw.ipat )
        {
            if( m_pat->end() == ++iw.ipat )
                throw bad_regexpr( "incomplete escape sequence \\c" );
            char_type ch = *iw.ipat++;
            if( REGEX_CHAR(CH,'a') <= ch && REGEX_CHAR(CH,'z') >= ch )
                ch = detail::regex_toupper( ch );
            detail::assign_auto_ptr( pnew, detail::create_char<CI>( CH( ch ^ 0x40 ), sy.get_flags(), m_arena ) );
        }
        else if( REGEX_CHAR(CH,'a') == *iw.ipat && normalize )
        {
            ++iw.ipat;
            detail::assign_auto_ptr( pnew, detail::create_char<CI>( REGEX_CHAR(CH,'\a'), sy.get_flags(), m_arena ) );
        }
        else if( REGEX_CHAR(CH,'f') == *iw.ipat && normalize )
        {
            ++iw.ipat;
            detail::assign_auto_ptr( pnew, detail::create_char<CI>( REGEX_CHAR(CH,'\f'), sy.get_flags(), m_arena ) );
        }
        else if( REGEX_CHAR(CH,'n') == *iw.ipat && normalize )
        {
            ++iw.ipat;
            detail::assign_auto_ptr( pnew, detail::create_char<CI>( REGEX_CHAR(CH,'\n'), sy.get_flags(), m_arena ) );
        }
        else if( REGEX_CHAR(CH,'r') == *iw.ipat && normalize )
        {
            ++iw.ipat;
            detail::assign_auto_ptr( pnew, detail::create_char<CI>( REGEX_CHAR(CH,'\r'), sy.get_flags(), m_arena ) );
        }
        else if( REGEX_CHAR(CH,'t') == *iw.ipat && normalize )
        {
            ++iw.ipat;
            detail::assign_auto_ptr( pnew, detail::create_char<CI>( REGEX_CHAR(CH,'\t'), sy.get_flags(), m_arena ) );
        }
        else if( REGEX_CHAR(CH,'\\') == *iw.ipat && normalize )
        {
            ++iw.ipat;
            detail::assign_auto_ptr( pnew, detail::create_char<CI>( REGEX_CHAR(CH,'\\'), sy.get_flags(), m_arena ) );
        }
        else
        {
            // Is this a user-defined intrinsic character set?
            detail::charset const * pcharset = detail::get_altern_charset( *iw.ipat, sy );
            if( NULL != pcharset )
                detail::assign_auto_ptr( pnew, detail::create_charset<CI>( *pcharset, sy.get_flags(), m_arena ) );
            else
                detail::assign_auto_ptr( pnew, detail::create_char<CI>( *iw.ipat, sy.get_flags(), m_arena ) );
            ++iw.ipat;
        }
        break;

        // If quotemeta, loop until we find quotemeta off or end of string
    case ESC_QUOTE_META_ON:
        for( istart = itemp = iw.ipat, fdone = false; !fdone && m_pat->end() != iw.ipat; )
        {
            switch( sy.reg_token( iw.ipat, m_pat->end() ) )
            {
            case ESC_QUOTE_META_OFF:
                fdone = true;
                break;
            case NO_TOKEN:
                if( m_pat->end() != iw.ipat )
                    ++iw.ipat; // fallthrough
            default:
                itemp = iw.ipat;
                break;
            }
        }
        if( itemp != istart )
            pgroup->add_item( detail::create_literal<CI>( istart, itemp, sy.get_flags(), m_arena ) );

        // skip the quantification code below
        return true;

    // Should never get here for valid patterns
    case ESC_QUOTE_META_OFF:
        throw bad_regexpr( "quotemeta turned off, but was never turned on" );

    default:
        assert( ! "Unhandled token type" );
        break;
    }

    // If pnew is null, then the current subexpression is a no-op.
    if( pnew.get() )
    {
        // Look for quantifiers
        _quantify( pnew, iw, is_group, sy );

        // Add the item to the group
        pgroup->add_item( pnew.release() );
    }
    return true;
}

template< typename CI, typename SY >
inline void basic_rpattern_base<CI, SY>::_quantify(
    std::auto_ptr<detail::sub_expr<CI> > & pnew,
    iter_wrap & iw,
    bool is_group,
    syntax_type & sy )
{
    if( m_pat->end() != iw.ipat && ! pnew->is_assertion() )
    {
        typename string_type::iterator itemp = iw.ipat, itemp2;
        bool fmin = false;

        // Since size_t is unsigned, -1 is really the largest size_t
        size_t lbound = ( size_t )-1;
        size_t ubound = ( size_t )-1;
        size_t ubound_tmp;

        switch( sy.quant_token( itemp, m_pat->end() ) )
        {
        case ZERO_OR_MORE_MIN:
            fmin = true;
        case ZERO_OR_MORE:
            lbound = 0;
            break;

        case ONE_OR_MORE_MIN:
            fmin = true;
        case ONE_OR_MORE:
            lbound = 1;
            break;

        case ZERO_OR_ONE_MIN:
            fmin = true;
        case ZERO_OR_ONE:
            lbound = 0;
            ubound = 1;
            break;

        case BEGIN_RANGE:
            lbound = detail::parse_int( itemp, m_pat->end() );
            if( m_pat->end() == itemp )
                return; // not a valid quantifier - treat as atom

            switch( sy.quant_token( itemp, m_pat->end() ) )
            {
            case END_RANGE_MIN:
                fmin = true;
            case END_RANGE:
                ubound = lbound;
                break;

            case RANGE_SEPARATOR:
                itemp2 = itemp;
                ubound_tmp = detail::parse_int( itemp, m_pat->end() );
                if( itemp != itemp2 )
                    ubound = ubound_tmp;
                if( itemp == m_pat->end() )
                    return; // not a valid quantifier - treat as atom
                switch( sy.quant_token( itemp, m_pat->end() ) )
                {
                case END_RANGE_MIN:
                    fmin = true;
                case END_RANGE:
                    break;
                default:
                    return; // not a valid quantifier - treat as atom
                }
                break;

            default:
                return; // not a valid quantifier - treat as atom
            }

            if( ubound < lbound  )
                throw bad_regexpr( "Can't do {n, m} with n > m" );

            break;
        }

        if( ( size_t )-1 != lbound )
        {
            // If we are quantifying a group, then this pattern could recurse
            // deeply. Note that fact here so that we can opt to use a stack-
            // conservative algorithm at match time.
            if( is_group && ubound > 16 )
                m_fok_to_recurse = false;

            std::auto_ptr<detail::sub_expr<CI> > pquant( pnew->quantify( lbound, ubound, ! fmin, m_arena ) );
            pnew.release();
            detail::assign_auto_ptr( pnew, pquant.release() );
            iw.ipat = itemp;
        }
    }
}

template< typename CI, typename SY >
inline void basic_rpattern_base<CI, SY>::_add_subst_backref( 
    detail::subst_node & snode, 
    size_t nbackref, 
    size_t rstart,
    bool & uses_backrefs,
    detail::subst_list_type & subst_list ) const
{
    uses_backrefs = true;
    assert( detail::subst_node::SUBST_STRING == snode.stype );
    if( snode.subst_string.rlength )
        subst_list.push_back( snode );

    snode.stype = detail::subst_node::SUBST_BACKREF;
    snode.subst_backref = nbackref;
    subst_list.push_back( snode );

    // re-initialize the detail::subst_node
    snode.stype = detail::subst_node::SUBST_STRING;
    snode.subst_string.rstart = rstart;
    snode.subst_string.rlength = 0;
}

template< typename CI, typename SY >
inline void basic_rpattern_base<CI, SY>::_parse_subst(
    string_type & subst,
    bool & uses_backrefs,
    detail::subst_list_type & subst_list ) const
{
    TOKEN tok;
    detail::subst_node snode;
    typename string_type::iterator icur = subst.begin();
    size_t nbackref;
    typename string_type::iterator itemp;
    bool fdone;
    syntax_type sy( m_flags );

    uses_backrefs = false;

    // Initialize the subst_node
    snode.stype = detail::subst_node::SUBST_STRING;
    snode.subst_string.rstart = 0;
    snode.subst_string.rlength = 0;

    while( subst.end() != icur )
    {
        switch( tok = sy.subst_token( icur, subst.end() ) )
        {
        case SUBST_MATCH:
            _add_subst_backref( snode, 0, std::distance( subst.begin(), icur ), uses_backrefs, subst_list );
            break;

        case SUBST_PREMATCH:
            _add_subst_backref( snode, ( size_t )detail::subst_node::PREMATCH, std::distance( subst.begin(), icur ), uses_backrefs, subst_list );
            break;

        case SUBST_POSTMATCH:
            _add_subst_backref( snode, ( size_t )detail::subst_node::POSTMATCH, std::distance( subst.begin(), icur ), uses_backrefs, subst_list );
            break;

        case SUBST_BACKREF:
            nbackref = detail::parse_int( icur, subst.end(), cgroups() - 1 ); // always at least 1 group
            if( 0 == nbackref )
                throw bad_regexpr( "invalid backreference in substitution" );

            _add_subst_backref( snode, nbackref, std::distance( subst.begin(), icur ), uses_backrefs, subst_list );
            break;

        case SUBST_QUOTE_META_ON:
            assert( detail::subst_node::SUBST_STRING == snode.stype );
            if( snode.subst_string.rlength )
                subst_list.push_back( snode );

            snode.subst_string.rstart = std::distance( subst.begin(), icur );
            for( itemp = icur, fdone = false; !fdone && subst.end() != icur; )
            {
                switch( tok = sy.subst_token( icur, subst.end() ) )
                {
                case SUBST_ALL_OFF:
                    fdone = true;
                    break;
                case NO_TOKEN:
                    ++icur; // fall-through
                default:
                    itemp = icur;
                    break;
                }
            }
            snode.subst_string.rlength = std::distance( subst.begin(), itemp ) - snode.subst_string.rstart;
            if( snode.subst_string.rlength )
                subst_list.push_back( snode );

            if( tok == SUBST_ALL_OFF )
            {
                snode.stype = detail::subst_node::SUBST_OP;
                snode.op    = detail::subst_node::ALL_OFF;
                subst_list.push_back( snode );
            }

            // re-initialize the subst_node
            snode.stype = detail::subst_node::SUBST_STRING;
            snode.subst_string.rstart = std::distance( subst.begin(), icur );
            snode.subst_string.rlength = 0;
            break;

        case SUBST_UPPER_ON:
        case SUBST_UPPER_NEXT:
        case SUBST_LOWER_ON:
        case SUBST_LOWER_NEXT:
        case SUBST_ALL_OFF:
            assert( detail::subst_node::SUBST_STRING == snode.stype );
            if( snode.subst_string.rlength )
                subst_list.push_back( snode );

            snode.stype = detail::subst_node::SUBST_OP;
            snode.op    = ( detail::subst_node::op_type ) tok;
            subst_list.push_back( snode );

            // re-initialize the subst_node
            snode.stype = detail::subst_node::SUBST_STRING;
            snode.subst_string.rstart = std::distance( subst.begin(), icur );
            snode.subst_string.rlength = 0;
            break;

        case SUBST_ESCAPE:
            if( subst.end() == icur )
                throw bad_regexpr( "expecting escape sequence in substitution string" );
            assert( detail::subst_node::SUBST_STRING == snode.stype );
            if( snode.subst_string.rlength )
                subst_list.push_back( snode );
            snode.subst_string.rstart = std::distance( subst.begin(), icur++ );
            snode.subst_string.rlength = 1;
            break;

        case NO_TOKEN:
        default:
            ++snode.subst_string.rlength;
            ++icur;
            break;
        }
    }
    assert( detail::subst_node::SUBST_STRING == snode.stype );
    if( snode.subst_string.rlength )
        subst_list.push_back( snode );
}


template< typename CH >
REGEXPR_H_INLINE void reset_intrinsic_charsets( CH )
{
    detail::intrinsic_charsets<CH>::reset();
}


typedef ::regex::detail::select
<
    REGEX_FOLD_INSTANTIATIONS &&
        detail::is_convertible<char const *,std::string::const_iterator>::value,
    std::string::const_iterator,
    char const *
>::type lpcstr_t;

typedef ::regex::detail::select
<
    REGEX_FOLD_INSTANTIATIONS &&
        detail::is_convertible<wchar_t const *,std::wstring::const_iterator>::value,
    std::wstring::const_iterator,
    wchar_t const *
>::type lpcwstr_t;

namespace
{
// Used to fake the compiler into implicitly instantiating the templates we need
bool g_regex_false;

template< typename CI, typename SY >
struct rpattern_instantiator
{
    typedef ::regex::basic_rpattern<CI,SY>      rpattern_type;
    typedef ::regex::basic_match_results<CI>    results_type;
    typedef typename rpattern_type::char_type   char_type;
    typedef typename rpattern_type::string_type string_type;

    rpattern_instantiator()
    {
        if( g_regex_false )
        {
            string_type const str;
            CI ci = CI();
            results_type res;
            rpattern_type pat;
            rpattern_type pat1( str );
            rpattern_type pat2( str, str );
            rpattern_type pat3( pat );
            pat3 = pat;

            pat.init( str );
            pat.init( str, str );
            pat.set_substitution( str );
            //pat.match( &*ci, res ); // could cause a static assert
            pat.match( ci, ci, res );
            //pat.count( &*ci ); // could cause a static assert
            pat.count( ci, ci );
            reset_intrinsic_charsets( char_type() );

            // These force VC6 to create COMDATs for set_substitution and the two init methods
            void (*preset)( char_type ) = & reset_intrinsic_charsets;
            (*preset)( char_type() );

            void (rpattern_type::*psetsub)( string_type const & ) = & rpattern_type::set_substitution;
            (pat.*psetsub)( str );

            void (rpattern_type::*pinit1)( string_type const &, REGEX_FLAGS, REGEX_MODE ) = & rpattern_type::init;
            (pat.*pinit1)( str, NOFLAGS, MODE_DEFAULT );

            void (rpattern_type::*pinit2)( string_type const &, string_type const &, REGEX_FLAGS, REGEX_MODE ) = & rpattern_type::init;
            (pat.*pinit2)( str, str, NOFLAGS, MODE_DEFAULT );
        }
    }
};

// Here is a rudimentary typelist facility to allow the REGEX_TO_INSTANTIATE
// list to recursively generate the instantiations we are interested in.
struct null_type;

template< typename H, typename T >
struct cons
{
    typedef H head;
    typedef T tail;
};

template< typename T1=null_type, typename T2=null_type, typename T3=null_type, typename T4=null_type,
          typename T5=null_type, typename T6=null_type, typename T7=null_type, typename T8=null_type >
struct typelist
{
    typedef cons< T1, typename typelist<T2,T3,T4,T5,T6,T7,T8,null_type>::type > type;
};

template<>
struct typelist<null_type,null_type,null_type,null_type,null_type,null_type,null_type,null_type>
{
    typedef null_type type;
};

// The recursive_instantiator uses typelists and the rpattern_instantiator
// to generate instantiations for all the types in the typelist.
template< typename TYPELIST >
struct recursive_instantiator
{
    // The inner struct is needed as a work-around for the lack
    // of partial template specialization.
    template< typename SY >
    struct inner
    {
        inner()
        {
            if( g_regex_false )
            {
                typedef typename TYPELIST::head CI;
                typedef typename ::std::iterator_traits<CI>::value_type char_type;
                typedef typename SY::template rebind<char_type>::other syntax_type;
                rpattern_instantiator<CI,syntax_type> dummy1;
                ( void ) dummy1;

                typedef typename TYPELIST::tail TYPELIST2;
                typedef recursive_instantiator< TYPELIST2 > other;
                typedef typename other::template inner<SY> other_inner;
                other_inner dummy2;
                ( void ) dummy2;
            }
        }
    };
};

template<>
struct recursive_instantiator< null_type >
{
    template< typename SY >
    struct inner
    {
    };
};

// Here is a list of types to instantiate.
#ifndef REGEX_TO_INSTANTIATE
# ifdef REGEX_WIDE_AND_NARROW
#  define REGEX_TO_INSTANTIATE std::string::const_iterator,  \
                               std::wstring::const_iterator, \
                               lpcstr_t,                     \
                               lpcwstr_t
# else
#  define REGEX_TO_INSTANTIATE restring::const_iterator,     \
                               lpctstr_t
# endif
#endif

// Create the perl instantiations
#ifndef REGEX_NO_PERL
recursive_instantiator<typelist<REGEX_TO_INSTANTIATE>::type>::inner<perl_syntax<char> > _dummy1;
#endif

// Create the posix instantiations
#ifdef REGEX_POSIX
recursive_instantiator<typelist<REGEX_TO_INSTANTIATE>::type>::inner<posix_syntax<char> > _dummy2;
#endif

} // unnamed namespace

} // namespace regex

