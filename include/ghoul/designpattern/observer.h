/*****************************************************************************************
 *                                                                                       *
 * OpenSpace                                                                             *
 *                                                                                       *
 * Copyright (c) 2014                                                                    *
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

#ifndef __OBSERVERABLE_H__
#define __OBSERVERABLE_H__

#include <functional>
#include <map>
#include <vector>

namespace ghoul {

template <typename Event>
class TemplateObservable {
public:
	template <typename Observer>
	int registerObserver(const Event& event, Observer&& observer);

	template <typename Observer>
	int registerObserver(Event&& event, Observer&& observer);

	void unregisterObserver(int observerId);

	void notify(const Event& event) const;

private:
	typedef std::pair<std::function<void()>, int> Function;

	int _currentIndex = 0;
	std::map<Event, std::vector<Function>> _observers;
};

typedef TemplateObservable<std::string> Observable;

} // namespace ghoul

#include "observer.inl"

#endif // __OBSERVERABLE_H__
