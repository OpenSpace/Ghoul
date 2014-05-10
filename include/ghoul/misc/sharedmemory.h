/*****************************************************************************************
 *                                                                                       *
 * GHOUL                                                                                 *
 * General Helpful Open Utility Library                                                  *
 *                                                                                       *
 * Copyright (c) 2012-2014                                                               *
 *                                                                                       *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this  *
 * software and associated documentation files (the "Software"), to deal in the Software *
 * without restriction, including without limitation the rights to use, copy, modify,    *
 * merge, publish, distribute, sublicense, and/or sell copies of the Software, and to    *
 * permit persons to whom the Software is furnished to do so, subject to the following   *
 * conditions:                                                                           *
 *                                                                                       *
 * The above copyright notice and this permission notice shall be included in all copies *
 * or substantial portions of the Software.                                              *
 *                                                                                       *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,   *
 * INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A         *
 * PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT    *
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF  *
 * CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE  *
 * OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                                         *
 ****************************************************************************************/

#ifndef __SHAREDMEMORY_H__
#define __SHAREDMEMORY_H__

#include <map>
#include <string>

#ifdef WIN32
#include <Windows.h>
#endif

namespace ghoul {

/**
 * This class is a platform-independent implementation of a shared memory architecture.
 * It provides the possibility to create (#create) and remove (#remove) globally visible
 * blocks of memory that can be used across processes on the same machine for use in
 * inter-process communication. The workflow for this is as follows: One process has to
 * #create a SharedMemory block with a specific name and will keep the ownership of that
 * name. It does not have to create a SharedMemory object itself, but it is the creating
 * process` responsibility to #remove the shared memory at the end of its lifetime; then,
 * many other processes can use the constructor with the same name to get access to the
 * shared memory. The overloaded operator void* makes the SharedMemory usable just like a
 * void pointer in the code. The size of the memory is accessible using the #size method.
 * Due to some necessary header information, the amount of memory that is allocated will
 * be slightly larger than the passed amount. The allocated memory automatically provides
 * storage for a thread-safe locking mechanism. In the current implementation, this is a
 * lock-free process, using an atomic bool to provide thread-safeness. It is possible for
 * a process to acquire exclusive access to the shared memory by calling #acquireLock and
 * relinquish that access by #releaseLock. Please note that this is not a strong
 * safeguard, as any process not using those two methods to guard their access into the
 * memory will not be stopped from reading or writing into the memory. The #acquireLock
 * method will not return until the lock has been acquired. The #releaseLock will return
 * immediately.
 */
class SharedMemory {
public:
    /**
     * Creates a globally visible shared memory block, which is accessible by other
     * processes with the same <code>name</code>. A block can only be created once and
     * each attempt to create a block whose name is already used will result in a logged
     * error and failure. The amount of memory allocated will be slightly bigger than
     * <code>size</code> to accommodate necessary header information. This header is
     * completely transparent and not accessible by the user of the memory block. The
     * process calling this method effectively has ownership of the memory, regardless if
     * it maps it into its own address space or not. That means that the process calling
     * the create method has to be the one that will call #remove at the end of its
     * lifetime (if the ownership is not transferred in the meantime). A failure to call
     * that method will result in a <b>systemwide</b> memory leak. On Windows, the memory
     * mapped file that will be created is only accessible by the same terminal session
     * as the process that has created it.
     * \param name The name of the SharedMemory block. This name must be unique and
     * unused in the system. A future constructor call using the same <code>name</code>
     * will point to the memory created in this method call
     * \param size The size (in bytes) of the shared memory block that should be created.
     * The actual size in memory will be slightly larger (+ 8 bytes currently) due to
     * necessary header information, which are not accessible by the user
     * \return <code>true</code> if the creation of the shared memory block succeeded,
     * <code>false</code> otherwise
     */
    static bool create(const std::string& name, size_t size);

    /**
     * Removes a previously created shared memory block. The <code>name</code> must be a
     * valid name for an accessible shared memory block. The underlying memory will only
     * be freed when no process has an SharedMemory object pointing to that name anymore
     * as long as one process has its memory attached into its own address space, no 
     * memory will be freed.
     * \param name The name of the shared memory block that should be marked for removal
     * \return <code>true</code> if the removal was done successfully, <code>false</code>
     * otherwise
     */
    static bool remove(const std::string& name);

    /**
     * Tests if a memory block with the given <code>name</code> has been created
     * previously or if the name is available.
     * \param name The name of the shared memory block that should be tested
     * \return <code>true</code> if a shared memory block exists with the given
     * <code>name</code>, <code>false</code> otherwise
     */
    static bool exists(const std::string& name);
    
    /**
     * Creates a SharedMemory object pointing to a previously created shared memory block
     * (#create). If <code>name</code> is a valid name for a shared memory block, the
     * constructor will attach the memory into the calling process' address space, making
     * it available through the operator <code>void*</code>. If an error occurrs either
     * getting a valid handle on the memory mapped file or during mapping the memory into
     * the address space, an invalid SharedMemory object will be created. An invalid
     * object will always return <code>nullptr</code> in its operator void* method. It is
     * possible for the same process to attach the same shared memory block multiple
     * times. It is undefined if two SharedMemory objects with the same name will be
     * attached to the same memory location.
     * \param name The name of the shared memory block to which this process wants to be
     * attached to
     */
    SharedMemory(const std::string& name);

    /**
     * The destructor will detach the memory from the calling processes address space and
     * free all previously acquired resources.
     */
    ~SharedMemory();
    
    /**
     * Returns the pointer to the first usable address of the allocated memory. The
     * transparently handled header is automatically skipped and is invisible to the
     * user. If this SharedMemory object is not valid (when an error occurred in the
     * constructor), a <code>nullptr</code> will be returned. Otherwise, the return value
     * is a valid pointer into a memory block of the predefined size (#size).
     */
    operator void*();

    /**
     * Returns the usable size of the memory block. The value here is the same which was
     * specified in the creation of the shared memory block (#create). The actual size of
     * the memory will be slightly bigger due to header fields.
     * \return The usable size of the memory block.
     */
    size_t size() const;
    
    /**
     * This method acquires a lock for the calling process to provide exclusive access to
     * the shared memory block. While one process owns the lock, any subsequent call to
     * this method will not return until the lock has been released. Please note that
     * this does not guarantee thread-safe execution if it is misused; if one process
     * accesses the memory without surrounding the accesses with #acquireLock,
     * #releaseLock methods, that process is not prevented from changing the memory while
     * a different thread has an exclusive, active lock on that memory. This method will
     * only return as soon as a lock has been successfully acquired.
     */
    void acquireLock();

    /**
     * Releases the acquired exclusive lock on the SharedMemory. As there is only one
     * lock for the SharedMemory and no authorization is used, any process can release
     * the lock of any other process by calling this method.
     */
    void releaseLock();
    
private:
    SharedMemory(const SharedMemory& rhs) = delete;
    SharedMemory& operator=(const SharedMemory& rhs) = delete;

    /**
     * This pointer points to the location in local address space where the shared memory
     * block has been attached to, or <code>nullptr</code> if the attachment failed
     */
    void* _memory;

    /// The logging category for this SharedMemory object; contains the name of the block
    const std::string _loggerCat;

    /**
     * The status of this SharedMemory object. Used to check which step of the
     * constructor failed.
     */
    unsigned char _status;
    
    // on Windows it is not possible to query the size of a memory mapped file, so it
    // has to be stored in the block of memory itself. On POSIX systems, we can store
    // the size in this object itself.
   
#ifdef WIN32
    /**
     * The handle to the memory mapped file that backs this SharedMemory object. Only a
     * virtual file is used, so there is no disk IO necessary.
     */
    HANDLE _sharedMemoryHandle;

    /**
     * Stores a mapping from name (as specified in the #create method) to the acquired
     * handle. This is necessary as each new CreateFileMapping call will create new
     * <code>HANDLE</code>, making it impossible to acquire the <code>HANDLE</code> for a
     * specific name otherwise.
     */
    static std::map<const std::string, HANDLE> _createdSections;
#else
    /// The full allocated size of the shared memory block
    size_t _size;
    /**
     * The handle to the virtual file backing this SharedMemory object. Only a virtual
     * file is used, so there is no disk IO necessary.
     */
    int _sharedMemoryHandle;
#endif
};
    
} // namespace ghoul

#endif // __SHAREDMEMORY_H__
