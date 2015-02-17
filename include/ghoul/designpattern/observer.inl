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

#include <utility>

namespace ghoul {

template <typename Event>
template <typename Observer>
int TemplateObservable<Event>::registerObserver(const Event& event, Observer&& observer) {
	_observers[event].push_back({std::forward<Observer>(observer), _currentIndex});
	return _currentIndex++;
}

template <typename Event>
template <typename Observer>
int TemplateObservable<Event>::registerObserver(Event&& event, Observer&& observer) {
	_observers[std::move(event)].push_back({std::forward<Observer>(observer), _currentIndex});
	return _currentIndex++;
}

template <typename Event>
void TemplateObservable<Event>::unregisterObserver(int observerId) {
	auto it = std::find_if(_observers.begin(), _observers.end(), [observerId](const std::pair<Event, Function>& obs) {
		return obs.second.second == observerId;
	});
		
	if (it != _observers.end())
		_observers.erase(it);
}

template <typename Event>
void TemplateObservable<Event>::notify(const Event& event) const {
	assert(_observers.find(event) != _observers.end());
	for (const auto& obs : _observers.at(event))
		obs.first();
}

} // namespace ghoul
