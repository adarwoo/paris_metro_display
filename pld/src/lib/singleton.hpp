#ifndef lib_singleton_h
#define lib_singleton_h
/**
 * @addtogroup service
 * @{
 * @addtogroup singleton
 *****************************************************************************
 * A statically allocated singleton implementation.
 * This template allow managing singleton instances, whose construction is
 *  triggered on the very first use, whilst using statically allocated
 *  memory.
 * @n
 * Unlike global instances, whose order of creation cannot be controlled,
 *  this singleton pattern guarantee that the instance has been constructed.
 * The memory is statically allocated through the macro #ALLOCATE_INSTANCE .
 * @n
 * This singleton pattern also allow the implementation detail to be
 *  hidden behind an interface. To use a derived implementation, a forward
 *  declaration is enough.
 * @n
 * Example:
 * @code
 * #include "lib/singleton.h"
 * using lib::Singleton;
 * struct ISequencer // Public interface
 * { virtual void doA() = 0; }
 * // Create a singleton of the interface
 * class SequencerA : Singleton<ISequencer, SequencerA>
 * { ... };
 * ALLOCATE_INSTANCE(SequencerA);
 *
 * // To use the singleton from anywhere
 * // A forward declaration of SequencerA is enough.
 * Singleton<ISequencer, SequencerA>::instance().doA();
 * @endcode
 *****************************************************************************
 * @{
 * @file
 * [Mode](group__mode.html) base class and API
 * @author gax
 */

#include "lib/builtin.hpp"

namespace lib
{
   /** 
    * @ingroup singleton
    * Allocate an objet of a given type.
    * This is a placeholder only. The macro #ALLOCATE_INSTANCE takes
    *  care of providing the actual typed instance of this function.
    * @tparam A The type of object to allocate.
    */
   template <class A> void *allocate();
   
   /**
    * @ingroup singleton
    * Singleton class template definition.
    * @tparam I The interface through which the template gets used
    * @tparam T The type of object managed by the singleton
    */
   template <class I, class T> class Singleton : I
   {
      /** Force static memory allocation for each new type */
      static I *storage;
      
   public:
      /** @return The pointer to the abstract instance */
      static I &instance()
      {
         if ( storage == nullptr )
         {
            // Create a new object (Singleton<IMode, Boot>)
            storage = (I *)allocate<T>();
         }
      
         return *reinterpret_cast<I *>(storage);
      }
   };
   
   template <class I, class T> I *Singleton<I, T>::storage = nullptr;

   /**
    * @ingroup singleton
    * Shortcut to obtaining a instance pointer
	*/
   template <class I, class T> constexpr I *get_instance_ptr()
      { return &Singleton<I, T>::instance(); }
} // End of namespace 'lib'

/**
 * Make provision for enough static memory to host an instance of T.
 * @warning This macro must be invoked outside of any namespace.
 * Otherwise it will fail.
 * @tparam T Name of the class type to allocate.
 */
#define ALLOCATE_INSTANCE(T) \
   namespace lib { \
      namespace { char instance_storage[sizeof(T)]; } \
      template<> void *allocate<T>() { \
         return new (instance_storage) T; \
      } \
   }

/**@}*/
/**@}*/
#endif /* ndef lib_singleton_h_HAS_ALREADY_BEEN_INCLUDED */