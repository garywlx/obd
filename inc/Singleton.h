#ifndef _SINGLETON_H_
#define _SINGLETON_H_

#include "TypeDefine.h"

/// Should be placed in the appropriate .cpp file somewhere
#define initialiseSingleton( type ) \
	template <> type * Singleton < type > :: mSingleton = 0

/// To be used as a replacement for initialiseSingleton( )
///  Creates a file-scoped Singleton object, to be retrieved with getSingleton
#define createFileSingleton( type ) \
	initialiseSingleton( type ); \
	type the##type

template < class type >

class  Singleton {
public:
	/// Constructor
	Singleton( ) {
		/// If you hit this assert, this singleton already exists -- you can't create another one!
		ASSERT( mSingleton == 0 );
		mSingleton = static_cast<type *>(this);
	}
	/// Destructor
	virtual ~Singleton( ) {
		mSingleton = 0;
	}

	/// Retrieve the singleton object, if you hit this assert this singleton object doesn't exist yet
	static type & getSingleton( ) { ASSERT( mSingleton ); return *mSingleton; }

	/// Retrieve a pointer to the singleton object
	static type * getSingletonPtr( ) { return mSingleton; }

protected:

	/// Singleton pointer, must be set to 0 prior to creating the object
	static type * mSingleton;
};

#endif