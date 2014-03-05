/**************************************************************************************************
 * GHOUL                                                                                          *
 * General Helpful Open Utility Library                                                           *
 *                                                                                                *
 * Copyright (c) 2012 Alexander Bock                                                              *
 *                                                                                                *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software  *
 * and associated documentation files (the "Software"), to deal in the Software without           *
 * restriction, including without limitation the rights to use, copy, modify, merge, publish,     *
 * distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the  *
 * Software is furnished to do so, subject to the following conditions:                           *
 *                                                                                                *
 * The above copyright notice and this permission notice shall be included in all copies or       *
 * substantial portions of the Software.                                                          *
 *                                                                                                *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING  *
 * BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND     *
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,   *
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, *
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.        *
 *************************************************************************************************/

#include <ghoul/opengl/framebufferobject.h>
#include <ghoul/logging/logmanager.h>

namespace ghoul {
namespace opengl {

const std::string FramebufferObject::_loggerCat("ghoul.opengl.FramebufferObject");

FramebufferObject::FramebufferObject() : _id(0) {
	generateId();
}

FramebufferObject::~FramebufferObject() {
	glDeleteFramebuffers(1, &_id);
}

void FramebufferObject::activate() {
	glBindFramebuffer(GL_FRAMEBUFFER, _id);
}

void FramebufferObject::deactivate() {
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

bool FramebufferObject::isComplete() const {
	bool isComplete = false;

	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	switch(status) {
		case GL_FRAMEBUFFER_COMPLETE:
			isComplete = true;
			break;
		case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
			LERROR("GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT");
			break;
		case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
			LERROR("GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT");
			break;
		case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT:
			LERROR("GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT");
			break;
		case GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT:
			LERROR("GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT");
			break;
		case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
			LERROR("GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER");
			break;
		case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
			LERROR("GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER");
			break;
		case GL_FRAMEBUFFER_UNSUPPORTED:
			LERROR("GL_FRAMEBUFFER_UNSUPPORTED");
			break;
		default:
			LERROR("Unknown error!");
			break;
	}
	return isComplete;
}

bool FramebufferObject::isActive() {
	return ((getActiveObject() == _id) && (_id != 0));
}

void FramebufferObject::attachTexture(Texture* texture, GLenum attachment, int mipLevel, int zSlice) {
	switch(texture->type()) {
		case GL_TEXTURE_1D:
			glFramebufferTexture1D( GL_FRAMEBUFFER, attachment, GL_TEXTURE_1D, *texture, mipLevel );
			break;
		case GL_TEXTURE_3D:
			glFramebufferTexture3D( GL_FRAMEBUFFER, attachment, GL_TEXTURE_3D, *texture, mipLevel, zSlice );
			break;
		case GL_TEXTURE_2D_ARRAY:
			glFramebufferTextureLayer( GL_FRAMEBUFFER, attachment, *texture, mipLevel, zSlice );
			break;
		default: //GL_TEXTURE_2D, GL_TEXTURE_RECTANGLE
			glFramebufferTexture2D( GL_FRAMEBUFFER, attachment, texture->type(), GLuint(*texture), mipLevel );
			break;
	}
	_attachedTextures[attachment] = texture;
}

void FramebufferObject::detachTexture(GLenum attachment) {
	auto iterator = _attachedTextures.find(attachment);
	if( iterator != _attachedTextures.end() )
		_attachedTextures.erase(iterator);
	else
		LWARNING("Trying to detach unknown texture!");

	glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, GL_TEXTURE_2D, 0, 0);
}

void FramebufferObject::detachAll() {
	while(!_attachedTextures.empty())
			detachTexture(_attachedTextures.begin()->first);
}

Texture* FramebufferObject::getTexture(GLenum attachment) {
	auto iterator = _attachedTextures.find(attachment);
	if( iterator != _attachedTextures.end() )
		return _attachedTextures[attachment];
	else
		return 0;
}

GLuint FramebufferObject::getActiveObject() {
	GLint fbo;
	glGetIntegerv (GL_FRAMEBUFFER_BINDING, &fbo);
	return static_cast<GLuint>(fbo);
}

GLuint FramebufferObject::generateId() {
	_id = 0;
	glGenFramebuffers(1, &_id);
	return _id;
}

} // namespace opengl
} // namespace ghoul
