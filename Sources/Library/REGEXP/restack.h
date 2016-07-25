//+---------------------------------------------------------------------------
//
//  Copyright ( C ) Microsoft Corporation, 1994 - 2002.
//
//  File:       restack.h
//
//  Functions:  a quick-'n'-dirty, type-unsafe stack used by the iterative
//              regular expression algorithm
//
//  Notes:      Care must be taken when using this stack. You must pop off
//              the correct type of object, otherwise you get garbage. Also,
//              in the current implementation, you shouldn't push anything
//              that has a non-trivial destructor, or if you do, then don't
//              use the set_jump/long_jump methods.
//
//  Author:     Eric Niebler ( ericne@microsoft.com )
//
//  History:    11/15/2001   ericne   Created
//
//----------------------------------------------------------------------------

#ifndef HETERO_STACK_H
#define HETERO_STACK_H

#include <string>
#include <utility>
#include <typeinfo>
#include <stdexcept>
#include <functional>

namespace regex
{

namespace detail
{

// For compile-time assertions that generate
// no run-time overhead.
template< bool f > struct static_assert;
template<>         struct static_assert<true> { static_assert() {} };

#ifdef _MSC_VER
// warning C4127: conditional expression is constant
// warning C4189: local variable is initialized but not referenced
// warning C4244: conversion from 'T' to 'int', possible loss of data
// warning C4510: default constructor could not be generated
// warning C4610: struct can never be instantiated - user defined constructor required
#pragma warning( push )
#pragma warning( disable : 4127 4189 4244 4510 4610 )

// Make sure nobody has tampered with the packing before defining the
// alignof structure
#pragma pack( push )
#pragma pack() // use the default packing
#endif

template< typename T >
class alignof
{
    struct helper
    {
        helper();
        char c_;
        T t_;
    };
public:
    enum { value = sizeof(helper)-sizeof(T) < sizeof(T) ? sizeof(helper)-sizeof(T) : sizeof(T) };
};

#ifdef _MSC_VER
#pragma pack( pop )
#endif

template< typename T >
struct unique_type_info
{
    static std::type_info const *const value;

    static bool equals(
        std::type_info const *const * ppti )
    {
        return ppti == & value || **ppti == typeid(T);
    }
};

template< typename T >
std::type_info const *const unique_type_info<T>::value = &typeid(T);

//
// Type traits
//

typedef char (&yes_type)[1];
typedef char (&no_type)[2];

#if defined(_MSC_VER) & _MSC_VER <= 1300
    // For use in conditional typedefs
    template< bool F, typename T, typename U >
    class select
    {
        template< bool > struct select_helper { typedef U type; };
        template<> struct select_helper<true> { typedef T type; };
    public:
        typedef typename select_helper<F>::type type;
    };

    // Check to see whether T is convertible to a U
    template< typename T, typename U >
    class is_convertible
    {
        static yes_type __cdecl  _convertible_helper( U );
        static no_type  __cdecl  _convertible_helper(...);
        static T t;
    public:
        enum { value = ( sizeof(_convertible_helper(t)) == sizeof(yes_type) ) };
    };
#else
    // For use in conditional typedefs
    template< bool F, typename T, typename U > struct select { typedef U type; };
    template< typename T, typename U > struct select<true,T,U> { typedef T type; };

    // Check to see whether T is convertible to a U
    template< typename U > yes_type   _convertible_helper( U );
    template< typename U > no_type    _convertible_helper(...);
    template< typename T, typename U >
    class is_convertible
    {
        static T t;
    public:
        enum { value = ( sizeof(_convertible_helper<U>(t)) == sizeof(yes_type) ) };
    };
#endif

} // namespace detail

// --------------------------------------------------------------------------
//
// Class:       hetero_stack
//
// Description: Fast, heterogeneous stack.
//
// Methods:     _alloc          - reserve space on stack
//              _unwind         - unwind the stack
//              hetero_stack    - c'tor
//              ~hetero_stack   - d'tor, release all dynamic memory
//              push            - push an object on the stack
//              pop             - pop an object from the stack
//
// Members:     m_first_node    -
//              m_current_node  -
//
// Typedefs:    byte            -
//
// History:     10/19/2001 - ericne - Created
//
// --------------------------------------------------------------------------
template
<
    size_t  Alignment          = sizeof(void*),
    bool    RuntimeTypeCheck   = true,  // should we perform run-time type checking?
    bool    AssumePOD          = false, // assume non-throwing copy/assign/destroy for better perf
    size_t  DynamicBlockSize   = 4096,  // blocks allocated from heap are this size
    size_t  StaticBlockSize    = 1024   // initial block on stack is this size
>
class hetero_stack
{
public:

    typedef hetero_stack<Alignment,RuntimeTypeCheck,AssumePOD,DynamicBlockSize,StaticBlockSize> stack_type;

    template< typename T >
    struct size_of
    {
        enum
        {
            // round up sizeof(T) to the nearest multiple of Alignment
            aligned = ( sizeof( T ) + Alignment - 1 ) & ~( Alignment - 1 ),
            with_rtti = RuntimeTypeCheck ?
                aligned + size_of<std::type_info const*const*>::aligned :
                aligned
        };
    };

private:

    static void _check_align()
    {
        // The alignment must be a power of 2
        detail::static_assert<
            Alignment == 1    || Alignment == 2    || Alignment == 4   ||
            Alignment == 8    || Alignment == 16   || Alignment == 32  ||
            Alignment == 128  || Alignment == 256  || Alignment == 512 ||
            Alignment == 1024 || Alignment == 2048 || Alignment == 4096 > const align_check;
        ( void ) align_check;
    }

    typedef unsigned char byte;

    struct stack_node
    {
        struct header
        {
            stack_node * m_back;
            stack_node * m_next;
            byte       * m_current; // ptr into m_mem. alloc from here
            byte       * m_end;     // ptr to last+1 byte in m_mem
        };

        union
        {
            header  m_head;
            byte    m_align[ size_of<header>::aligned ];
        };

        // This is the buffer into which values will be pushed and popped.
        // It is guaranteed to meet the Alignment requirements because of
        // the union above.
        byte         m_mem[1];

        size_t size() const // throw()
        {
            return ( size_t )( m_head.m_end - m_mem );
        }
    };

    enum
    {
        DYNAMIC_BLOCK_SIZE =
            DynamicBlockSize > sizeof( stack_node ) ?
            DynamicBlockSize : sizeof( stack_node )
    };

    union
    {
        byte m_buf[ offsetof( stack_node, m_mem ) + StaticBlockSize ];
        stack_node m_node;
    } m_first_node;

    stack_node * m_current_node;

    // Cache these for faster access
    byte * m_begin;
    byte * m_current;
    byte * m_end;

    void * _grow( size_t size ) // throw(std::bad_alloc)
    {
        // write the cached value of current into the node.
        // OK to do this even if later statements throw.
        m_current_node->m_head.m_current = m_current;

        // Do we have a node with available memory already?
        if( m_current_node->m_head.m_next )
        {
            // Does this node have enough room?
            if( size <= m_current_node->m_head.m_next->size() )
            {
                m_current_node = m_current_node->m_head.m_next;
                m_current = m_current_node->m_head.m_current = m_current_node->m_mem + size;
                m_end     = m_current_node->m_head.m_end;
                return m_begin = m_current_node->m_mem;
            }

            // Create a new node and insert it into the list
            stack_node * new_node = static_cast<stack_node*>( ::operator new( size + offsetof( stack_node, m_mem ) ) );
            
            new_node->m_head.m_back = m_current_node;
            new_node->m_head.m_next = m_current_node->m_head.m_next;
            
            m_current = m_end = new_node->m_head.m_current = new_node->m_head.m_end = new_node->m_mem + size;

            m_current_node->m_head.m_next->m_head.m_back = new_node;
            m_current_node->m_head.m_next = new_node;
            m_current_node = new_node;

            return m_begin = m_current_node->m_mem;
        }

        // We need to create a new node from scratch
        size_t new_size = (std::max)( size, (size_t)DYNAMIC_BLOCK_SIZE - offsetof( stack_node, m_mem ) );
        stack_node * new_node = static_cast<stack_node*>( ::operator new( new_size + offsetof( stack_node, m_mem ) ) );
        
        new_node->m_head.m_back = m_current_node;
        new_node->m_head.m_next = NULL;
        
        m_current = new_node->m_head.m_current = new_node->m_mem + size;
        m_end     = new_node->m_head.m_end     = new_node->m_mem + new_size;

        m_current_node->m_head.m_next = new_node;
        m_current_node = new_node;

        return m_begin = m_current_node->m_mem;
    }

    void * _alloc( size_t size ) // throw(std::bad_alloc)
    {
        // This is the ptr to return
        void * mem = m_current;

        // Advance the high-water mark
        m_current += size;

        // Check to see if we have overflown this buffer
        if( std::less<void*>()( m_end, m_current ) )
        {
            // oops, back this out.
            m_current -= size;

            // allocate a new block and return a ptr to the new memory
            return _grow( size );
        }

        return mem;
    }

    void * _unwind( byte * pb ) // throw()
    {
        // roll back the stack
        m_current = pb;

        // If we've unwound this whole block, then make the
        // previous node the current node
        if( m_current == m_begin )
        {
            // write the cached value of m_current into m_current_node
            m_current_node->m_head.m_current = m_current;
            m_current_node = m_current_node->m_head.m_back;

            // update the cache
            m_begin   = m_current_node->m_mem;
            m_current = m_current_node->m_head.m_current;
            m_end     = m_current_node->m_head.m_end;
        }

        return pb;
    }

    void * _unwind( size_t size ) // throw()
    {
        return _unwind( m_current - size );
    }

    struct real_unwinder;
    friend struct real_unwinder;
    struct real_unwinder
    {
        stack_type * pstack_;
        size_t       size_;
        bool         dismissed_;
        real_unwinder( stack_type * pstack, size_t size ) // throw()
            : pstack_(pstack), size_(size), dismissed_(false) {}
        ~real_unwinder() // throw()
        {
            if( ! dismissed_ )
                pstack_->_unwind( size_ );
        }
        void dismiss() // throw()
        {
            dismissed_ = true;
        }
    };

    struct dummy_unwinder
    {
        dummy_unwinder( stack_type *, size_t ) {} // throw()
        void dismiss() {} // throw()
    };

    // Disallow these for now. Might implement them later.
    hetero_stack( hetero_stack const & );
    hetero_stack & operator=( hetero_stack const & );

public:

    class type_error : public std::logic_error
    {
        std::type_info const * prequested_type_;
        std::type_info const * pactual_type_;
    public:
        type_error( 
            std::type_info const & requested_type,
            std::type_info const & actual_type,
            std::string const & s = "type error in hetero_stack" ) // throw()
          : std::logic_error( s + " (requested type: " + requested_type.name()
                + ", actual type: " + actual_type.name() + ")" ),
            prequested_type_( &requested_type ),
            pactual_type_( &actual_type )
        {
        }
        std::type_info const & requested_type() const // throw()
        {
            return *prequested_type_;
        }
        std::type_info const & actual_type() const // throw()
        {
            return *pactual_type_;
        }
    };

    hetero_stack() // throw()
      : m_current_node( &m_first_node.m_node )
    {
        _check_align();
        m_first_node.m_node.m_head.m_back    = & m_first_node.m_node;
        m_first_node.m_node.m_head.m_next    = NULL;
        m_begin = m_current = m_first_node.m_node.m_head.m_current = m_first_node.m_node.m_mem;
        m_end = m_first_node.m_node.m_head.m_end = m_first_node.m_buf + sizeof( m_first_node );
    }

    ~hetero_stack() // throw()
    {
        m_current_node = m_first_node.m_node.m_head.m_next;
        for( stack_node * next_node; m_current_node; m_current_node = next_node )
        {
            next_node = m_current_node->m_head.m_next;
            ::operator delete( static_cast<void*>( m_current_node ) );
        }
    }

    template< typename T >
    inline void push( T const & t ) // throw(std::bad_alloc,...)
    {
        // Make sure that the alignment for type T is not worse
        // than our declared alignment.
        detail::static_assert<( Alignment >= detail::alignof<T>::value )> const align_test;
        ( void ) align_test;

        // If T won't throw in copy c'tor, and if RuntimeTypeCheck is false,
        // then we don't need to use an unwinder object.
        typedef typename ::regex::detail::select< AssumePOD, // || detail::has_trivial_copy<T>::value,
            dummy_unwinder, real_unwinder >::type unwinder;

        // If this throws, it doesn't change state,
        // so there is nothing to roll back.
        void * pv = _alloc( size_of<T>::with_rtti );

        // Rolls back the _alloc if later steps throw
        // BUGBUG we can do the alloc, but not update m_current until after
        // the copy c'tor to avoid the need for an unwinder object
        unwinder guard1( this, size_of<T>::with_rtti );

        new ( pv ) T( t ); // Could throw if ! has_trivial_copy<T>::value

        // If we are debugging the stack, then push a pointer to the type_info
        // for this type T. It will be checked in pop().
        if( RuntimeTypeCheck )
        {
            new ( static_cast<byte*>( pv ) + size_of<T>::aligned ) 
                ( std::type_info const*const* )( & detail::unique_type_info<T>::value );
        }

        // ok, everything succeeded -- dismiss the guard
        guard1.dismiss();
    }

    template< typename T >
    inline void pop( T & t ) // throw(...)
    {
        detail::static_assert<( Alignment >= detail::alignof<T>::value )> const align_test;
        ( void ) align_test;

        // If we are debugging the stack, then in push() we pushed a pointer
        // to the type_info struct for this type T.  Check it now.
        if( RuntimeTypeCheck )
        {
            void * pti = m_current - size_of<std::type_info const*const*>::aligned;
            if( ! detail::unique_type_info<T>::equals( *static_cast<std::type_info const*const**>( pti ) ) )
                throw type_error( typeid( T ), ***static_cast<std::type_info const*const**>( pti ) );
        }

        // Don't change state yet because assignment op could throw!
        void * pT = m_current - size_of<T>::with_rtti;
        t = *static_cast<T const*>( pT ); // could throw
        static_cast<T const*>( pT )->~T();
        _unwind( static_cast<byte*>( pT ) );
    }

    // Call this version of pop when you don't need the popped value
    template< typename T >
    inline void pop() // throw(type_error,...)
    {
        detail::static_assert<( Alignment >= detail::alignof<T>::value )> const align_test;
        ( void ) align_test;

        // If we are debugging the stack, then in push() we pushed a pointer

        // to the type_info struct for this type T.  Check it now.
        if( RuntimeTypeCheck )
        {
            void * pti = m_current - size_of<std::type_info const*const*>::aligned;
            if( ! detail::unique_type_info<T>::equals( *static_cast<std::type_info const*const**>( pti ) ) )
                throw type_error( typeid( T ), ***static_cast<std::type_info const*const**>( pti ) );
        }

        T const * pT = static_cast<T const *>( _unwind( size_of<T>::with_rtti ) );
        pT->~T();
    }

    // Call this version of pop when you don't need the popped value and
    // throwing an exception isn't an option
    template< typename T >
    inline bool pop( std::nothrow_t const & ) // throw()
    {
        detail::static_assert<( Alignment >= detail::alignof<T>::value )> const align_test;
        ( void ) align_test;

        // If we are debugging the stack, then in push() we pushed a pointer
        // to the type_info struct for this type T.  Check it now.
        if( RuntimeTypeCheck )
        {
            void * pti = m_current - size_of<std::type_info const*const*>::aligned;
            if( ! detail::unique_type_info<T>::equals( *static_cast<std::type_info const*const**>( pti ) ) )
                return false; // type error, can't throw so bail.
        }

        T const * pT = static_cast<T const *>( _unwind( size_of<T>::with_rtti ) );
        pT->~T();
        return true;
    }

    template< typename T >
    inline T & top( T ) const // throw(type_error,...)
    {
        detail::static_assert<( Alignment >= detail::alignof<T>::value )> const align_test;
        ( void ) align_test;

        if( RuntimeTypeCheck )
        {
            // If we are debugging the stack, then the top of the stack is a
            // pointer to a type_info struct. Assert that we have the correct type.
            void * pti = m_current - size_of<std::type_info const*const*>::aligned;
            if( ! detail::unique_type_info<T>::equals( *static_cast<std::type_info const*const**>( pti ) ) )
                throw type_error( typeid( T ), ***static_cast<std::type_info const*const**>( pti ) );
        }

        void * pT = m_current - size_of<T>::with_rtti;
        return *static_cast<T*>( pT );
    }

    // Fetch the type_info for the element at the top of the stack
    std::type_info const & top_type() const // throw()
    {
        detail::static_assert< RuntimeTypeCheck > const type_check; ( void ) type_check;
        void * pti = m_current - size_of<std::type_info const*const*>::aligned;
        return ***static_cast<std::type_info const*const**>( pti );
    }

    // Get a pointer to the top of the stack
    void * set_jump() const // throw()
    {
        return m_current;
    }

    // Quick and dirty stack unwind. Does not call destructors.
    void long_jump( void * jump_ptr ) // throw()
    {
        for( ;; )
        {
            if( std::less<void*>()( jump_ptr, m_current_node->m_mem ) ||
                std::less<void*>()( m_current_node->m_head.m_end, jump_ptr ) )
            {
                m_current_node->m_head.m_current = m_current_node->m_mem;
                m_current_node = m_current_node->m_head.m_back;
            }
            else
            {
                m_begin   = m_current_node->m_mem;
                m_current = m_current_node->m_head.m_current = static_cast<byte*>( jump_ptr );
                m_end     = m_current_node->m_head.m_end;
                return;
            }
        }
    }

    bool empty() const // throw()
    {
        return m_current == m_first_node.m_node.m_mem;
    }

    // Use scoped_push for automatically pushing/popping
    // things to and from the stack. This is especially useful
    // if you want to push a bunch of things "atomically".  For
    // instance:
    //
    // typedef hetero_stack<>::scoped_pop scoped_pop;
    // scoped_pop p1 = stack.scoped_push( int(1) ); // could throw
    // scoped_pop p2 = stack.scoped_push( std::string("foo") ); // could throw
    // stack.push( float(3.14159) ); // could throw
    // p2.dismiss(); // ok, nothing threw, so ...
    // p1.dismiss(); //  ... dismiss the scoped_pops
    //
    // If p2 and p1 are not dismissed, as in the case when an
    // exception gets thrown, then they automatically pop their
    // arguments from the stack.

    class scoped_pop_base
    {
    protected:
        stack_type * pstack_;
        bool mutable owns_;
        scoped_pop_base & operator=( scoped_pop_base const & ); // disallow assignment
    public:
        scoped_pop_base( stack_type * pstack ) // throw(std::bad_alloc,...)
          : pstack_( pstack )
          , owns_( true )
        {
        }
        scoped_pop_base( scoped_pop_base const & that ) // throw() // destructive copy
          : pstack_( that.pstack_ )
          , owns_( that.owns_ )
        {
            // This popper takes ownership, that popper gives it up.
            that.owns_ = false;
        }
        void dismiss() const // throw()
        {
            owns_ = false;
        }
    };

    template< typename T >
    struct scoped_pop_t : public scoped_pop_base
    {
        scoped_pop_t( stack_type * pstack, T const & t ) // throw(std::bad_alloc,...)
          : scoped_pop_base( pstack )
        {
            // Note that if this throws an exception the destructor
            // will not get called, which is what we want.
            pstack_->push( t );
        }
        ~scoped_pop_t() // throw()
        {
            // If we own this stack space, pop it.
            if( owns_ )
                pstack_->template pop<T>( std::nothrow );
        }
    };

    template< typename T >
    scoped_pop_t<T> scoped_push( T const & t ) // throw(...)
    {
        return scoped_pop_t<T>( this, t );
    }

    typedef scoped_pop_base const & scoped_pop;
};

// BUGBUG push and pop *must* be balanced since d'tors need to be called.
// Iterative execution of regex matching makes this difficult considering
// the fact that push() could cause an exception to be thrown.  The entire
// stack of sub_expr* must be backtracked completely and correctly.

// Alternate solution: push nothing that needs to be destroyed.

#ifdef _MSC_VER
#pragma warning( pop )
#endif

} // namespace regex

#endif
