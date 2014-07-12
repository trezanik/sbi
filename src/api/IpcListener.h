#pragma once

/**
 * @file	IpcListener.h
 * @author	James Warren
 * @brief	Listener class to inherit from for IPC
 */



#include "definitions.h"


BEGIN_NAMESPACE(APP_NAMESPACE)


/**
 * The class to inherit from if you want to be notified on new data written to
 * an Ipc. Once the final listener has been notified, the data could be invalid
 * at any time, so it is a listeners responsibility to copy data that must be
 * retained in their notification handler.
 *
 * @class IpcListener
 */
class SBI_API IpcListener
{
	// this class calls our Notify method
	friend class Interprocess;
private:
	NO_CLASS_ASSIGNMENT(IpcListener);
	NO_CLASS_COPY(IpcListener);

public:
	IpcListener()
	{
	}
	~IpcListener()
	{
	}


	/**
	 * @note Should we just supply the data as a parameter to this function
	 * straight out, or should we do things like a hint or smth.
	 * Otherwise, we're just notifying the listener that they should call
	 * ReadSMO, which just gets the data all the same anyway...
	 */
	virtual void
	Notify()
	{
		/* on Visual Studio, this cannot just be a declaration, as it
		 * requires a call to this function to exist within the library,
		 * despite nothing from api actively using it.
		 * gcc/clang are happy without this, but no problems with it. */
	}
};



END_NAMESPACE
