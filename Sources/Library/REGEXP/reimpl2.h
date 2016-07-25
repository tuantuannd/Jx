//+---------------------------------------------------------------------------
//
//  Copyright ( C ) Microsoft Corporation, 1994 - 2002.
//
//  File:       reimpl2.h
//
//  Functions:  helpers for matching and substituting regular expressions
//
//  Notes:      implementation details that really belong in a cpp file,
//              but can't because of template weirdness
//
//  Author:     Eric Niebler ( ericne@microsoft.com )
//
//  History:    8/15/2001   ericne   Created
//
//----------------------------------------------------------------------------

#ifndef REIMPL_H
#define REIMPL_H

//
// Helper functions for match and substitute
//

#ifndef _MSC_VER
#define __assume( x ) assert( false ); break;
#endif

namespace detail
{

// For use while doing uppercase/lowercase conversions:
// For use while doing uppercase/lowercase conversions:
inline  char   regex_toupper(  char   ch ) { return ( char  )toupper( ch ); }
inline  char   regex_tolower(  char   ch ) { return ( char  )tolower( ch ); }
inline wchar_t regex_toupper( wchar_t ch ) { return ( wchar_t )towupper( ch ); }
inline wchar_t regex_tolower( wchar_t ch ) { return ( wchar_t )towlower( ch ); }
template< typename II, typename CI >
inline void regex_toupper( II ibegin, CI iend )
{
    typedef typename std::iterator_traits<CI>::value_type char_type;
    typedef std::char_traits<char_type> traits_type;

    for( ; iend != ibegin; ++ibegin )
        traits_type::assign( *ibegin, regex_toupper( *ibegin ) );
}
template< typename II, typename CI >
inline void regex_tolower( II ibegin, CI iend )
{
    typedef typename std::iterator_traits<CI>::value_type char_type;
    typedef std::char_traits<char_type> traits_type;

    for( ; iend != ibegin; ++ibegin )
        traits_type::assign( *ibegin, regex_tolower( *ibegin ) );
}

// Work-around for a template-template parameter problem on VC7.0
template<typename T> struct type2type { typedef T type; };

template<bool F> struct bool2type { enum { value = F }; };

typedef bool2type<true>  true_t;
typedef bool2type<false> false_t;

//
// Helper fn for swapping two auto_ptr's
//

template< typename T >
inline void swap_auto_ptr( std::auto_ptr<T> & left, std::auto_ptr<T> & right )
{
    std::auto_ptr<T> temp( left );
    left = right;
    right = temp;
}

// --------------------------------------------------------------------------
//
// Class:       match_param
//
// Description: Struct that contains the state of the matching operation.
//              Passed by reference to all recursive_match_all_ and recursive_match_this routines.
//
// Methods:     match_param - ctor
//              match_param - ctor
//
// Members:     ibegin      - start of the string
//              istart      - start of this iteration
//              istop       - end of the string
//              prgbackrefs - pointer to backref array
//
// History:     8/14/2000 - ericne - Created
//
// --------------------------------------------------------------------------
template< typename CI >
struct match_param
{
    typedef backref_tag< CI >         backref_type;
    typedef std::vector<backref_type> backref_vector;

    // Used by the recursive_match routines
    backref_vector * prgbackrefs;
    CI ibegin;
    CI istart;
    CI istop;

    // Used by the iterative_match routines
    CI icur;
    unsafe_stack * pstack;
    sub_expr_base<CI> const * next;

    bool no0len;
    sub_expr_base<CI> const * first;

    match_param(
        CI _istart,
        CI _istop,
        std::vector< backref_tag< CI > > * _prgbackrefs )
      : prgbackrefs( _prgbackrefs ),
        ibegin( _istart ),
        istart( _istart ),
        istop( _istop ),
        icur( _istart ),
        pstack( NULL ),
        next( NULL ),
        no0len( false )
    {
    }
    match_param(
        CI _ibegin,
        CI _istart,
        CI _istop,
        std::vector< backref_tag< CI > > * _prgbackrefs )
      : prgbackrefs( _prgbackrefs ),
        ibegin( _ibegin ),
        istart( _istart ),
        istop( _istop ),
        icur( _istart ),
        pstack( NULL ),
        next( NULL ),
        no0len( false )
    {
    }
};

// --------------------------------------------------------------------------
//
// Class:       regex_arena
//
// Description: A small, fast allocator for speeding up pattern compilation.
//              Every basic_rpattern object has an arena as a member.
//              sub_expr objects can only be allocated from this arena.
//              Memory is alloc'ed in chunks using ::operator new(). Chunks
//              are freed en-masse when the arena gets destroyed, or when
//              deallocate is explicitly called.
//
// Methods:     _new_block    - create a new block & put it in m_pfirst
//              regex_arena   - c'tor
//              ~regex_arena  - free all memory blocks
//              allocate      - Grab some preallocated memory.
//              deallocate    - free all memory blocks.
//              max_size      - the largest chunk of memory the arena is
//                              capable of allocating.
//
// Members:     m_pfirst      - ptr to first block in list
//
// History:     8/17/2001 - ericne - Created
//
// --------------------------------------------------------------------------
class regex_arena
{
    struct block;
    friend struct block;
    block * m_pfirst;
    size_t m_default_size;
    void _new_block( size_t size ); //throw( std::bad_alloc );
    regex_arena( regex_arena const & );
    regex_arena & operator=( regex_arena const & );
public:
    explicit regex_arena( size_t default_size );
    ~regex_arena();
    void * allocate( size_t size ); //throw( std::bad_alloc );
    void deallocate(); //throw();
    size_t max_size() const;
    void swap( regex_arena & that ); // throw()
};

// --------------------------------------------------------------------------
//
// Class:       sub_expr_base
//
// Description: patterns are "compiled" into a directed graph of sub_expr_base
//              structs.  Matching is accomplished by traversing this graph.
//
// Methods:     ~sub_expr_base - virt dtor so cleanup happens correctly
//              recursive_match_all_      - match this sub-expression and all following
//                               sub-expression
//
// History:     8/14/2000 - ericne - Created
//
// --------------------------------------------------------------------------
template< typename CI >
struct sub_expr_base
{
    virtual ~sub_expr_base() = 0;

    virtual bool recursive_match_all_( match_param<CI> &, CI ) const = 0; //throw() = 0;
    virtual bool recursive_match_all_c( match_param<CI> &, CI ) const = 0; //throw() = 0;
    virtual bool iterative_match_this_( match_param<CI> & ) const = 0; //throw() = 0;
    virtual bool iterative_match_this_c( match_param<CI> & ) const = 0; //throw() = 0;
    virtual bool iterative_rematch_this_( match_param<CI> & ) const = 0; //throw() = 0;
    virtual bool iterative_rematch_this_c( match_param<CI> & ) const = 0; //throw() = 0;

    // Use the regex_arena for memory management
    static void * operator new( size_t size, regex_arena & arena )
    {
        return arena.allocate( size );
    }
    static void operator delete( void *, regex_arena & )
    {
    }

    // Invoke the d'tor, but don't bother freeing memory. That will
    // happen automatically when the arena object gets destroyed.
    static void operator delete( void * )
    {
    }

    // For choosing an appropriate virtual function based on a compile time constant
    bool recursive_match_all_( match_param<CI> & param, CI icur, false_t ) const //throw()
    {
        return recursive_match_all_( param, icur );
    }
    bool recursive_match_all_( match_param<CI> & param, CI icur, true_t ) const //throw()
    {
        return recursive_match_all_c( param, icur );
    }
    bool iterative_match_this_( match_param<CI> & param, false_t ) const //throw()
    {
        return iterative_match_this_( param );
    }
    bool iterative_match_this_( match_param<CI> & param, true_t ) const //throw()
    {
        return iterative_match_this_c( param );
    }
    bool iterative_rematch_this_( match_param<CI> & param, false_t ) const //throw()
    {
        return iterative_rematch_this_( param );
    }
    bool iterative_rematch_this_( match_param<CI> & param, true_t ) const //throw()
    {
        return iterative_rematch_this_c( param );
    }
};

template< typename CI >
inline sub_expr_base<CI>::~sub_expr_base()
{
}

// --------------------------------------------------------------------------
//
// Class:       subst_node
//
// Description: Substitution strings are parsed into an array of these
//              structures in order to speed up subst operations.
//
// Members:     stype         - type of this struct
//              subst_string  - do a string substitution
//              subst_backref - do a bacref substitution
//              op            - execute an operation
//
// History:     8/14/2000 - ericne - Created
//
// --------------------------------------------------------------------------
struct subst_node
{
    enum subst_type
    {
        SUBST_STRING,
        SUBST_BACKREF,
        SUBST_OP
    };

    enum { PREMATCH = -1, POSTMATCH = -2 };

    enum op_type
    {
        UPPER_ON   = SUBST_UPPER_ON,
        UPPER_NEXT = SUBST_UPPER_NEXT,
        LOWER_ON   = SUBST_LOWER_ON,
        LOWER_NEXT = SUBST_LOWER_NEXT,
        ALL_OFF    = SUBST_ALL_OFF
    };

    subst_type stype;

    union
    {
        struct
        {
            size_t rstart;
            size_t rlength;
        } subst_string;
        size_t  subst_backref;
        op_type op;
    };
};

typedef std::list<subst_node> subst_list_type;
size_t DEFAULT_BLOCK_SIZE();

// --------------------------------------------------------------------------
//
// Class:       basic_rpattern_base_impl
//
// Description:
//
// Methods:     basic_rpattern_base_impl - ctor
//              flags                   - get the state of the flags
//              uses_backrefs           - true if the backrefs are referenced
//              get_first_subexpression - return ptr to first sub_expr struct
//              get_width               - get min/max nbr chars this pattern can match
//              loops                   - if false, we only need to try to match at 1st position
//              cgroups                 - number of visible groups
//              _cgroups_total          - total number of groups, including hidden ( ?: ) groups
//              get_pat                 - get string representing the pattern
//              get_subst               - get string representing the substitution string
//              get_subst_list          - get the list of subst nodes
//              _normalize_string       - perform character escaping
//
// Members:     m_fuses_backrefs        - true if subst string refers to backrefs
//              m_floop                 - false if pat only needs to be matched in one place
//              m_cgroups               - total count of groups
//              m_cgroups_visible       - count of visible groups
//              m_flags                 - the flags
//              m_nwidth                - width of this pattern
//              m_pat                   - pattern string
//              m_subst                 - substitution string
//              m_subst_list            - list of substitution nodes
//              m_pfirst                - ptr to first subexpression to match
//
// Typedefs:    char_type               -
//              string_type             -
//              size_type               -
//
// History:     8/14/2000 - ericne - Created
//
// --------------------------------------------------------------------------
template< typename CI >
class basic_rpattern_base_impl
{
    basic_rpattern_base_impl( basic_rpattern_base_impl<CI> const & );
    basic_rpattern_base_impl & operator=( basic_rpattern_base_impl<CI> const & );
protected:
    typedef typename std::iterator_traits<CI>::value_type char_type;
    typedef std::char_traits<char_type>                   traits_type;

    typedef std::basic_string<char_type>                  string_type;
    typedef size_t                                        size_type;

    typedef backref_tag<CI>                               backref_type;
    typedef std::vector<backref_type>                     backref_vector;

    explicit basic_rpattern_base_impl(
        REGEX_FLAGS flags = NOFLAGS,
        REGEX_MODE mode = MODE_DEFAULT,
        string_type const & pat   = string_type(),
        string_type const & subst = string_type() ) //throw()
      : m_arena( DEFAULT_BLOCK_SIZE() ),
        m_fuses_backrefs( false ),
        m_floop( true ),
        m_fok_to_recurse( true ),
        m_cgroups( 0 ),
        m_cgroups_visible( 0 ),
        m_flags( flags ),
        m_mode( mode ),
        m_nwidth( uninit_width() ),
        m_pat( new string_type( pat ) ),
        m_subst( new string_type( subst ) ),
        m_subst_list(),
        m_pfirst( NULL ),
        m_invisible_groups()
    {
    }

    virtual ~basic_rpattern_base_impl();

    regex_arena m_arena;           // The sub_expr arena

    bool        m_fuses_backrefs;  // true if the substitution uses backrefs
    bool        m_floop;           // false if m_pfirst->recursive_match_all_ only needs to be called once
    bool        m_fok_to_recurse;  // false if the pattern would recurse too deeply
    size_t      m_cgroups;         // number of groups ( always at least one )
    size_t      m_cgroups_visible; // number of visible groups
    REGEX_FLAGS m_flags;           // flags used to customize search/replace
    REGEX_MODE  m_mode;            // Used to pick the fast or safe algorithm
    width_type  m_nwidth;          // width of the pattern

    std::auto_ptr<string_type> m_pat;   // contains the unparsed pattern
    std::auto_ptr<string_type> m_subst; // contains the unparsed substitution

    subst_list_type m_subst_list; // used to speed up substitution
    std::auto_ptr<sub_expr_base<CI> const> m_pfirst;     // first subexpression in pattern

    std::list<size_t> m_invisible_groups; // groups w/o backrefs

    size_t _cgroups_total() const //throw()
    {
        return m_cgroups;
    }

    bool _loops() const //throw()
    {
        return m_floop;
    }

    size_t _get_next_group_nbr()
    {
        return m_cgroups++;
    }

    void _normalize_string( string_type & str ) const //throw()
    {
        if( NORMALIZE & flags() )
            process_escapes( str, true );
    }

    bool _save_backrefs() const //throw()
    {
        return m_fuses_backrefs || ! ( flags() & NOBACKREFS );
    }

    sub_expr_base<CI> const * _get_first_subexpression() const //throw()
    {
        return m_pfirst.get();
    }

    bool _ok_to_recurse() const //throw()
    {
        switch( m_mode )
        {
        case MODE_FAST:
            return true;
        case MODE_SAFE:
            return false;
        case MODE_MIXED:
            return m_fok_to_recurse;
        default:
            return false;
        }
    }

    // These are virtual so that when the instantiator object implicitly
    // instantiates this template, these functions still get external linkage.
    virtual bool   _do_match( match_param<CI> & param, bool use_null ) const;
    virtual bool   _do_match( match_param<CI> & param, char_type const * szbegin ) const;
    virtual size_t _do_count( match_param<CI> & param, bool use_null ) const;
    virtual size_t _do_count( match_param<CI> & param, char_type const * szbegin ) const;

    friend struct matcher_helper<CI>;

    REGEX_FLAGS flags() const //throw()
    {
        return m_flags;
    }

    REGEX_MODE mode() const // throw()
    {
        return m_mode;
    }

    width_type get_width() const //throw()
    {
        return m_nwidth;
    }

    size_t cgroups() const //throw()
    {
        return m_cgroups_visible;
    }

    string_type const & get_pat() const //throw()
    {
        return *m_pat;
    }

    string_type const & get_subst() const //throw()
    {
        return *m_subst;
    }

    void swap( basic_rpattern_base_impl<CI> & that ) // throw()
    {
        std::swap( m_fuses_backrefs, that.m_fuses_backrefs );
        std::swap( m_floop, that.m_floop );
        std::swap( m_fok_to_recurse, that.m_fok_to_recurse );
        std::swap( m_cgroups, that.m_cgroups );
        std::swap( m_cgroups_visible, that.m_cgroups_visible );
        std::swap( m_flags, that.m_flags );
        std::swap( m_mode, that.m_mode );
        std::swap( m_nwidth, that.m_nwidth );

        swap_auto_ptr( m_pat, that.m_pat );
        swap_auto_ptr( m_subst, that.m_subst );
        swap_auto_ptr( m_pfirst, that.m_pfirst );

        m_subst_list.swap( that.m_subst_list );
        m_invisible_groups.swap( m_invisible_groups );
        m_arena.swap( that.m_arena );
    }

    static size_t const npos;
};

template< typename CI >
size_t const basic_rpattern_base_impl<CI>::npos = size_t( -1 );

template< typename CI >
struct matcher_helper
{
    typedef basic_rpattern_base_impl<CI>             rpattern_type;
    typedef typename rpattern_type::size_type        size_type;
    typedef typename rpattern_type::char_type        char_type;
    typedef typename rpattern_type::traits_type      traits_type;
    typedef typename rpattern_type::backref_type     backref_type;
    typedef typename rpattern_type::backref_vector   backref_vector;

    static match_param<CI> init_param( CI ibegin, CI iend, basic_match_results<CI> & results )
    {
        results.ibegin = ibegin;
        return match_param<CI>( ibegin, iend, & results.m_rgbackrefs );
    }

    // Here is the main dispatch loop.  It is responsible for calling
    // match on the current sub-expression and repeating for the next
    // sub-expression. It also backtracks the match when it needs to.
    template< typename CSTRINGS >
    static bool _Do_match_iterative( sub_expr_base<CI> const * expr, match_param<CI> & param, CI icur, CSTRINGS )
    {
        unsafe_stack & s = *param.pstack;
        void * jump_ptr = s.set_jump(); // the bottom of the stack
        param.icur = icur;

        if( ! expr->iterative_match_this_( param, CSTRINGS() ) )
        {
            return false;
        }

        for( ;; )
        {
            do
            {
                if( param.next == NULL ) // This means we're done
                    if( param.no0len && param.istart == param.icur )
                        goto keep_looking;
                    else
                        return s.long_jump( jump_ptr ), true;
                s.push( expr );
                expr = param.next;
            }
            while( expr->iterative_match_this_( param, CSTRINGS() ) );

            do
            {
                if( jump_ptr == s.set_jump() ) // No more posibilities to try
                    return false;
                s.pop( expr );
    keep_looking:;
            }
            while( ! expr->iterative_rematch_this_( param, CSTRINGS() ) );
        }
    }

    static bool _Do_match_iterative_helper( sub_expr_base<CI> const * expr, match_param<CI> & param, CI icur )
    {
        return _Do_match_iterative( expr, param, icur, false_t() );
    }

    static bool _Do_match_iterative_helper_c( sub_expr_base<CI> const * expr, match_param<CI> & param, CI icur )
    {
        return _Do_match_iterative( expr, param, icur, true_t() );
    }

    static bool _Do_match_recursive( sub_expr_base<CI> const * expr, match_param<CI> & param, CI icur )
    {
        return expr->recursive_match_all_( param, icur );
    }

    static bool _Do_match_recursive_c( sub_expr_base<CI> const * expr, match_param<CI> & param, CI icur )
    {
        return expr->recursive_match_all_c( param, icur );
    }

    static bool _Do_match_impl( rpattern_type const & pat, match_param<CI> & param, bool const use_null )
    {
        typedef std::list<size_t>::const_iterator LCI;
        typedef bool ( *pfndomatch_t )( sub_expr_base<CI> const * expr, match_param<CI> & param, CI icur );

        bool       floop   = pat._loops();
        unsigned   flags   = pat.flags();
        width_type nwidth  = pat.get_width();

        // If the pstack parameter is not NULL, we should do a safe, iterative match.
        // Otherwise, we should do a fast, recursive match.
        pfndomatch_t pfndomatch;
        if( NULL != param.pstack )
            if( use_null )
                pfndomatch = &_Do_match_iterative_helper_c;
            else
                pfndomatch = &_Do_match_iterative_helper;
        else
            if( use_null )
                pfndomatch = &_Do_match_recursive_c;
            else
                pfndomatch = &_Do_match_recursive;

        sub_expr_base<CI> const * pfirst = pat._get_first_subexpression();
        param.first = pfirst;

        assert( NULL != param.prgbackrefs );

        param.prgbackrefs->resize( pat._cgroups_total() );
        std::fill( param.prgbackrefs->begin(), param.prgbackrefs->end(), backref_type() );

#ifdef _MSC_VER
        __try
        {
#endif
            if( ! use_null )
            {
                // If the minimum width of the pattern exceeds the width of the
                // string, a succesful match is impossible
                if( nwidth.m_min <= ( size_t )std::distance( param.istart, param.istop ) )
                {
                    CI local_istop = param.istop;
                    std::advance( local_istop, -int( nwidth.m_min ) );

                    if( RIGHTMOST & flags )
                    {
                        // begin trying to match after the last character.
                        // Continue to the beginning
                        for( CI icur = local_istop; icur >= param.istart; --icur, param.no0len = false )
                            if( ( *pfndomatch )( pfirst, param, icur ) )
                                break; // m_floop not used for rightmost matches
                    }
                    else
                    {
                        // begin trying to match before the first character.
                        // Continue to the end
                        for( CI icur = param.istart; icur <= local_istop; ++icur, param.no0len = false )
                            if( ( *pfndomatch )( pfirst, param, icur ) || ! floop )
                                break;
                    }
                }
            }
            else
            {
                assert( 0 == ( RIGHTMOST & flags ) );
                // begin trying to match before the first character.
                // Continue to the end
                for( CI icur = param.istart; ; ++icur, param.no0len = false )
                {
                    if( ( *pfndomatch )( pfirst, param, icur ) || ! floop )
                        break;
                    if( traits_type::eq( REGEX_CHAR(char_type,'\0'), *icur ) )
                        break;
                }
            }
#ifdef _MSC_VER
        }
        __except( REGEX_STACK_OVERFLOW == _exception_code() )
        {
            // we have overflowed the stack. reset the guard page.
            _resetstkoflw();
            // This match fails silently.
            std::fill( param.prgbackrefs->begin(), param.prgbackrefs->end(), backref_tag<CI>() );
        }
#endif

        // Remove information about the "invisible" groups
        if( ( *param.prgbackrefs )[0].matched )
        {
            size_t dropped = 0;
            std::list<size_t> const & l = pat.m_invisible_groups;

            for( LCI curr = l.begin(), next = l.begin(); curr != l.end(); curr = next, ++dropped )
            {
                if( ++next == l.end() )
                {
                    std::copy(
                        param.prgbackrefs->begin() + *curr + 1,
                        param.prgbackrefs->end(),
                        param.prgbackrefs->begin() + *curr - dropped );
                }
                else
                {
                    std::copy(
                        param.prgbackrefs->begin() + *curr + 1,
                        param.prgbackrefs->begin() + *next,
                        param.prgbackrefs->begin() + *curr - dropped );
                }
            }
            param.prgbackrefs->resize( param.prgbackrefs->size() - dropped );
        }
        else
        {
            param.prgbackrefs->resize( param.prgbackrefs->size() - pat.m_invisible_groups.size() );
        }

        return ( *param.prgbackrefs )[0].matched;
    }


    static bool _Do_match_with_stack( rpattern_type const & pat, match_param<CI> & param, bool const use_null );

    static bool _Do_match( rpattern_type const & pat, match_param<CI> & param, bool const use_null )
    {
        if( pat._ok_to_recurse() )
            return _Do_match_impl( pat, param, use_null );
        return _Do_match_with_stack( pat, param, use_null );
    }

    template< typename CH, typename TR, typename AL >
    static size_t _Do_subst_internal(
        std::basic_string<CH, TR, AL> & str,
        basic_subst_results<CH, TR, AL> const & results,
        rpattern_type const & pat,
        size_type strpos,
        size_type strlen )
    {
        typedef subst_list_type::const_iterator LCI;
        enum { UPPER = -1, NIL, LOWER } next = NIL, rest = NIL;
        bool first = true;
        size_t old_strpos = strpos;
        typename std::basic_string<CH, TR, AL>::iterator itstrlen = str.begin();
        std::advance( itstrlen, strpos + strlen );
        std::basic_string<char_type> const & subst = pat.get_subst();

        for( LCI isubst = pat.m_subst_list.begin(); isubst != pat.m_subst_list.end(); ++isubst )
        {
            size_t sublen;
            typename std::basic_string<CH, TR, AL>::const_iterator  itsubpos1; // iter into str
            typename std::basic_string<CH, TR, AL>::const_iterator  itsublen1;
            typename std::basic_string<char_type>::const_iterator   itsubpos2; // iter into subst string
            typename std::basic_string<char_type>::const_iterator   itsublen2;
            typename std::basic_string<CH, TR, AL>::iterator itstrpos = str.begin();
            std::advance( itstrpos, strpos );

            switch( isubst->stype )
            {
            case subst_node::SUBST_STRING:
                itsubpos2 = subst.begin();
                std::advance( itsubpos2, isubst->subst_string.rstart );
                itsublen2 = itsubpos2;
                std::advance( itsublen2, isubst->subst_string.rlength );

                if( first )
                    str.replace( itstrpos, itstrlen, itsubpos2, itsublen2 );
                else
                    str.insert( itstrpos, itsubpos2, itsublen2 );
                sublen = std::distance( itsubpos2, itsublen2 );
                break;

            case subst_node::SUBST_BACKREF:
                switch( isubst->subst_backref )
                {
                case subst_node::PREMATCH:
                    itsubpos1 = results.backref_str().begin();
                    itsublen1 = itsubpos1;
                    std::advance( itsublen1, sublen = results.rstart() );
                    break;
                case subst_node::POSTMATCH:
                    itsubpos1 = results.backref_str().begin();
                    std::advance( itsubpos1, results.rstart() + results.rlength() );
                    itsublen1 = results.backref_str().end();
                    break;
                default:
                    itsubpos1 = results.backref_str().begin();
                    std::advance( itsubpos1, results.rstart( isubst->subst_backref ) );
                    itsublen1 = itsubpos1;
                    std::advance( itsublen1, results.rlength( isubst->subst_backref ) );
                    break;
                }

                if( first )
                    str.replace( itstrpos, itstrlen, itsubpos1, itsublen1 );
                else
                    str.insert( itstrpos, itsubpos1, itsublen1 );
                sublen = std::distance( itsubpos1, itsublen1 );
                break;

            case subst_node::SUBST_OP:
                switch( isubst->op )
                {
                case subst_node::UPPER_ON:
                    rest = UPPER;
                    break;
                case subst_node::UPPER_NEXT:
                    next = UPPER;
                    break;
                case subst_node::LOWER_ON:
                    rest = LOWER;
                    break;
                case subst_node::LOWER_NEXT:
                    next = LOWER;
                    break;
                case subst_node::ALL_OFF:
                    rest = NIL;
                    break;
                default:
                    __assume( 0 );
                }
                continue; // jump to the next item in the list

                default:
                    __assume( 0 );
            }

            first = false;

            // Are we upper- or lower-casing this string?
            if( rest )
            {
                typename std::basic_string<CH, TR, AL>::iterator istart = str.begin();
                std::advance( istart, strpos );
                typename std::basic_string<CH, TR, AL>::const_iterator istop = istart;
                std::advance( istop, sublen );
                switch( rest )
                {
                case UPPER:
                    regex_toupper( istart, istop );
                    break;
                case LOWER:
                    regex_tolower( istart, istop );
                    break;
                default:
                    __assume( 0 );
                }
            }

            // Are we upper- or lower-casing the next character?
            if( next )
            {
                switch( next )
                {
                case UPPER:
                    str[strpos] = regex_toupper( str[strpos] );
                    break;
                case LOWER:
                    str[strpos] = regex_tolower( str[strpos] );
                    break;
                default:
                    __assume( 0 );
                }
                next = NIL;
            }

            strpos += sublen;
        }

        // If *first* is still true, then we never called str.replace, and the substitution
        // string is empty. Erase the part of the string that the pattern matched.
        if( first )
            str.erase( strpos, strlen );

        // return length of the substitution
        return strpos - old_strpos;
    }

    template< typename CH, typename TR, typename AL >
    static size_t _Do_subst(
        rpattern_type const & pat,
        std::basic_string<CH, TR, AL> & str,
        basic_subst_results<CH, TR, AL> & results,
        size_type pos,
        size_type len )
    {
        typedef std::basic_string<CH, TR, AL> string_type;

        results.m_pbackref_str = pat._save_backrefs() ? &( results.m_backref_str = str ) : &str;
        results.ibegin = results.m_pbackref_str->begin();

        size_t csubst = 0;
        size_type stop_offset = results.m_pbackref_str->size();
        if( len != rpattern_type::npos )
            stop_offset = (std::min)( size_t( pos + len ), stop_offset );

        match_param<CI> param( results.ibegin, results.ibegin, & results.m_rgbackrefs );
        std::advance( param.istart, pos );
        std::advance( param.istop, stop_offset );
        param.ibegin = param.istart;

        if( GLOBAL & pat.flags() )
        {
            bool const fAll   = ( ALLBACKREFS   == ( ALLBACKREFS   & pat.flags() ) );
            bool const fFirst = ( FIRSTBACKREFS == ( FIRSTBACKREFS & pat.flags() ) );
            backref_vector rgtempbackrefs; // temporary vector used if fsave_backrefs

            size_type pos_offset = 0; // keep track of how much the backref_str and
                                    // the current string are out of sync

            while( _Do_match( pat, param, false ) )
            {
                backref_type const & br = results.m_rgbackrefs[0];
                ++csubst;
                size_type match_length = std::distance( br.first, br.second );
                pos = std::distance( results.ibegin, br.first );
                size_type subst_length = _Do_subst_internal( str, results, pat, pos + pos_offset, match_length );

                if( pat._save_backrefs() )
                {
                    pos += match_length;
                    pos_offset += ( subst_length - match_length );

                    // Handle specially the backref flags
                    if( fFirst )
                        rgtempbackrefs.push_back( br );
                    else if( fAll )
                        rgtempbackrefs.insert( rgtempbackrefs.end(),
                                            param.prgbackrefs->begin(),
                                            param.prgbackrefs->end() );
                    else
                        rgtempbackrefs.swap( *param.prgbackrefs );
                }
                else
                {
                    pos += subst_length;
                    stop_offset += ( subst_length - match_length );
                    results.ibegin = results.m_pbackref_str->begin();

                    // we're not saving backref information, so we don't
                    // need to do any special backref maintenance here
                }

                // prevent a pattern that matches 0 characters from matching
                // again at the same point in the string
                param.no0len = ( 0 == match_length );

                param.istart = results.ibegin;
                std::advance( param.istart, pos ); // ineffecient for bidirectional iterators.

                param.istop = results.ibegin;
                std::advance( param.istop, stop_offset ); // ineffecient for bidirectional iterators.
            }

            // If we did special backref handling, swap the backref vectors
            if( pat._save_backrefs() )
            {
                param.prgbackrefs->swap( rgtempbackrefs );
            }
            else if( ! ( *param.prgbackrefs )[0].matched )
            {
                param.prgbackrefs->clear();

            }
        }
        else if( _Do_match( pat, param, false ) )
        {
            backref_type const & br = results.m_rgbackrefs[0];
            ++csubst;
            _Do_subst_internal(
                str, results, pat,
                std::distance( results.ibegin, br.first ),
                std::distance( br.first, br.second ) );
            results.ibegin = results.m_pbackref_str->begin();
        }

        if( NOBACKREFS == ( pat.flags() & NOBACKREFS ) )
        {
            param.prgbackrefs->clear();
        }

        return csubst;
    }
};

template< typename CI >
REGEX_NOINLINE bool matcher_helper<CI>::_Do_match_with_stack( rpattern_type const & pat, match_param<CI> & param, bool const use_null )
{
    unsafe_stack s;
    param.pstack = &s;
    return _Do_match_impl( pat, param, use_null );
}

//
// Some helper functions needed by process_escapes
//
template< typename CH >
inline bool regex_isxdigit( CH ch )
{
    return ( REGEX_CHAR(CH,'0') <= ch && REGEX_CHAR(CH,'9') >= ch )
        || ( REGEX_CHAR(CH,'a') <= ch && REGEX_CHAR(CH,'f') >= ch )
        || ( REGEX_CHAR(CH,'A') <= ch && REGEX_CHAR(CH,'F') >= ch );
}

template< typename CH >
inline int regex_xdigit2int( CH ch )
{
    if( REGEX_CHAR(CH,'a') <= ch && REGEX_CHAR(CH,'f') >= ch )
        return ch - REGEX_CHAR(CH,'a') + 10;
    if( REGEX_CHAR(CH,'A') <= ch && REGEX_CHAR(CH,'F') >= ch )
        return ch - REGEX_CHAR(CH,'A') + 10;
    return ch - REGEX_CHAR(CH,'0');
}

} // namespace detail

// --------------------------------------------------------------------------
//
// Function:    process_escapes
//
// Description: Turn the escape sequnces \f \n \r \t \v \\ into their
//              ASCII character equivalents. Also, optionally process
//              perl escape sequences.
//
// Returns:     void
//
// Arguments:   str      - the string to process
//              fPattern - true if the string is to be processed as a regex
//
// Notes:       When fPattern is true, the perl escape sequences are not
//              processed.  If there is an octal or hex excape sequence, we
//              don't want to turn it into a regex metacharacter here.  We
//              leave it unescaped so the regex parser correctly interprests
//              it as a character literal.
//
// History:     8/1/2001 - ericne - Created
//
// --------------------------------------------------------------------------
template< typename CH, typename TR, typename AL >
inline void process_escapes( std::basic_string<CH, TR, AL> & str, bool fPattern ) //throw()
{
    typedef typename std::basic_string<CH, TR, AL>::size_type size_type;

    size_type i = 0;
    size_type const npos = std::basic_string<CH, TR, AL>::npos;

    if( str.empty() )
        return;

    while( npos != ( i = str.find( REGEX_CHAR(CH,'\\'), i ) ) )
    {
        if( str.size() - 1 == i )
            return;

        switch( str[i+1] )
        {
        case REGEX_CHAR(CH,'a'):
            str.replace( i, 2, 1, REGEX_CHAR(CH,'\a') );
            break;
        case REGEX_CHAR(CH,'b'):
            if( ! fPattern )
                str.replace( i, 2, 1, REGEX_CHAR(CH,'\b') );
            else
                ++i;
            break;
        case REGEX_CHAR(CH,'e'):
            str.replace( i, 2, 1, CH( 27 ) );
            break;
        case REGEX_CHAR(CH,'f'):
            str.replace( i, 2, 1, REGEX_CHAR(CH,'\f') );
            break;
        case REGEX_CHAR(CH,'n'):
            str.replace( i, 2, 1, REGEX_CHAR(CH,'\n') );
            break;
        case REGEX_CHAR(CH,'r'):
            str.replace( i, 2, 1, REGEX_CHAR(CH,'\r') );
            break;
        case REGEX_CHAR(CH,'t'):
            str.replace( i, 2, 1, REGEX_CHAR(CH,'\t') );
            break;
        case REGEX_CHAR(CH,'v'):
            str.replace( i, 2, 1, REGEX_CHAR(CH,'\v') );
            break;
        case REGEX_CHAR(CH,'\\'):
            if( fPattern )
            {
                if( i+3 < str.size() && REGEX_CHAR(CH,'\\') == str[i+2] && REGEX_CHAR(CH,'\\') == str[i+3] )
                    str.erase( i, 2 );
                ++i;
            }
            else
                str.erase( i, 1 );
            break;
        case REGEX_CHAR(CH,'0'): case REGEX_CHAR(CH,'1'): case REGEX_CHAR(CH,'2'): case REGEX_CHAR(CH,'3'):
        case REGEX_CHAR(CH,'4'): case REGEX_CHAR(CH,'5'): case REGEX_CHAR(CH,'6'): case REGEX_CHAR(CH,'7'):
            if( ! fPattern )
            {
                size_t j=i+2;
                CH ch = CH( str[i+1] - REGEX_CHAR(CH,'0') );
                for( ; j-i < 4 && j < str.size() && REGEX_CHAR(CH,'0') <= str[j] && REGEX_CHAR(CH,'7') >= str[j]; ++j )
                    ch = CH( ch * 8 + ( str[j] - REGEX_CHAR(CH,'0') ) );
                str.replace( i, j-i, 1, ch );
            }
            break;
        case REGEX_CHAR(CH,'x'):
            if( ! fPattern )
            {
                CH ch = 0;
                size_t j=i+2;
                for( ; j-i < 4 && j < str.size() && detail::regex_isxdigit( str[j] ); ++j )
                    ch = CH( ch * 16 + detail::regex_xdigit2int( str[j] ) );
                str.replace( i, j-i, 1, ch );
            }
            break;
        case REGEX_CHAR(CH,'c'):
            if( ! fPattern && i+2 < str.size() )
            {
                CH ch = str[i+2];
                if( REGEX_CHAR(CH,'a') <= ch && REGEX_CHAR(CH,'z') >= ch )
                    ch = detail::regex_toupper( ch );
                str.replace( i, 3, 1, CH( ch ^ 0x40 ) );
            }
            break;
        default:
            if( fPattern )
                ++i;
            else
                str.erase( i, 1 );
            break;
        }
        ++i;
        if( str.size() <= i )
            return;
    }
}

#ifndef _MSC_VER
#undef __assume
#endif

#endif
