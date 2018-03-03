/*****************************************************************************************
 *                                                                                       *
 * GHOUL                                                                                 *
 * General Helpful Open Utility Library                                                  *
 *                                                                                       *
 * Copyright (c) 2012-2018                                                               *
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

#ifndef __GHOUL___PROGRAMOBJECTMANAGER___H__
#define __GHOUL___PROGRAMOBJECTMANAGER___H__

#include <ghoul/opengl/programobject.h>
#include <functional>
#include <map>
#include <memory>
#include <string>

namespace ghoul::opengl {

/**
 * The ProgramObjectManager can be used to cache multiple ProgramObjects based on a unique
 * name. A ProgramObject can be requested using #requestProgramObject. If a ProgramObject
 * with the specified name has already been created, a pointer to it is returned.
 * Otherwise it will be created using the passed creationFunction and stored internally.
 * Clients can release a ProgramObject using #releaseProgramObject. The ProgramObject is
 * only deleted when the last client that requested the object has released it. At the 
 * time of ProgramObjectManager destruction, all ProgramObject%s have to have been
 * released or the application will assert. 
 */
class ProgramObjectManager {
public:
    using CreationCallback = std::function<std::unique_ptr<ProgramObject>()>;
    using DestructionCallback = std::function<void(ProgramObject*)>;

    /**
     * Checks whether all ProgramObjects have been released
     */
    ~ProgramObjectManager();

    /*
     * Requests a new ProgramObject with a unique \p name and, if it has not been created
     * previously, calls the \p creationFunction whose responsibility it is to return a
     * newly created ProgramObject, a pointer to which is returned by this function call
     * and all subsequent function calls with the same \p name. If a ProgramObject existed
     * at the time of the call, the \p creationFunction is not called. This method only
     * returns <code>nullptr</code> if \p creationFunction returned a
     * <code>nullptr</code>, which will cause *all* following calls with the same name to
     * return a <code>nullptr</code> as well. The \p creationFunction will be called
     * exactly once for each \p name regardless of its return value.
     * \param name The name of the ProgramObject that is to be generated
     * \param creationFunction If this is the first call with the provided \p name, this
     *        function is executed to create a new ProgramObject. Regardless of its
     *        return value, this function is only ever going to be called exactly once
     */
    ProgramObject* requestProgramObject(const std::string& name,
        const CreationCallback& creationFunction);

    /**
     * Releases the ProgramName with the provided \p name. If the ProgramObject has been
     * requested \c i number of times, and this is the \c ith call for this \p name, the
     * \p destructionFunction is called with the ProgramObject to take care of any
     * additional destruction. OBS: The regular destructor of the ProgramObject will be
     * automatically called after the \p destructionFunction returns, so it is **not**
     * advised for the client to call \c delete on the ProgramObject as well.
     * The return value can be used to modify a local copy of the ProgramObject to make it
     * clear which program name is associated with which local variable.
     * \param name The unique name of the ProgramObject that should be released
     * \param destructionFunction The function that can handle additional destruction
     *        events required by the client. Please not that the regular destructor will
     *        be automatically called after this method returns program control back to
     *        the ProgramObjectManager
     */
    void releaseProgramObject(const std::string& name,
        const DestructionCallback& destructionFunction = [](ProgramObject*) {});

private:
    struct Info {
        std::unique_ptr<ProgramObject> program = nullptr;
        int refCount = 0;
    };
    std::map<std::string, Info> _programs;
};

}  // namespace ghoul::opengl

#endif // __GHOUL___PROGRAMOBJECTMANAGER___H__
