/*****************************************************************************************
 *                                                                                       *
 * GHOUL                                                                                 *
 * General Helpful Open Utility Library                                                  *
 *                                                                                       *
 * Copyright (c) 2012-2015                                                               *
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

template<typename T>
bool ghoul::opengl::VertexBufferObject::initialize(
	const std::vector<T>& varray, 
	const std::vector<GLint>& iarray) 
{
	static_assert(std::is_pod<T>::value, "T has to be a POD");
    if(isInitialized()) {
        return false;
    }

	generateGLObjects();

	if (_vaoID == 0 || _vBufferID == 0 || _iBufferID == 0)
		return false;

	if (varray.size() == 0 || iarray.size() == 0)
		return false;

	_isize = static_cast<unsigned int>(iarray.size());

	glBindVertexArray(_vaoID);

	glBindBuffer(GL_ARRAY_BUFFER, _vBufferID);
	glBufferData(GL_ARRAY_BUFFER,
                 varray.size() * sizeof(T),
                 varray.data(),
                 GL_STATIC_DRAW);
                 
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _iBufferID);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 iarray.size() * sizeof(GLint),
                 iarray.data(),
                 GL_STATIC_DRAW);

	glBindVertexArray(0);
	return true;
}
