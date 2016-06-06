/*****************************************************************************************
*                                                                                       *
* OpenSpace                                                                             *
*                                                                                       *
* Copyright (c) 2014-2015                                                               *
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

#include <ghoul/designpattern/event.h>
#include <algorithm>
#include <string>
namespace ghoul {

    template<class... T>
    Event<T...>::Event(){}

    template<class... T>
    Event<T...>::~Event(){}

    template<class... T>
    void Event<T...>::subscribe(std::string name, std::string topic, Listener listener){

        // If topic does not exist, create a new key and vector pair
        if (_topics.find(topic) == _topics.end()) {
            _topics.insert(std::make_pair(topic , std::vector< std::pair<std::string, Listener > >() ));
        }
        // push the subscribers callback function into the specified topic
        _topics[topic].push_back( std::make_pair(name, listener) );
    }

    template<class... T>
    void Event<T...>::publish(std::string topic, T... message){
        // if the topic exists
        if (_topics.find(topic) != _topics.end()) {
            // go through all subscribers and send them the message
            for (auto &subscriber : _topics[topic]) {
                subscriber.second(message...);
            }
        }
    }

    template<class... T>
    void Event<T...>::unsubscribe(std::string name, std::string topic){
        if (_topics.find(topic) != _topics.end()) {
            // Search through the whole array of subscribers to given topic and remove all
            // callbacks that the corresponds to the subscribers name.
            _topics[topic].erase( std::remove_if( _topics[topic].begin(), _topics[topic].end(), 
                // predicate function, true if subscriber name is equal to given name
                [name](std::pair<std::string, Listener > subscriber){
                    return (subscriber.first == name);
                } ), _topics[topic].end() 
            );
        }
    }

    template<class... T>
    void Event<T...>::unsubscribe(std::string name){
        // Search through all topics to erase all callbacks that belong to the 
        // object with given name
        for (auto& topic : _topics) {
            topic.second.erase( std::remove_if( topic.second.begin(), topic.second.end(), 
                // predicate function, true if subscriber name is equal to given name
                [name](std::pair<std::string, Listener > subscriber){
                    return (subscriber.first == name);
                } ), topic.second.end() 
            );
        }
    }
} // namespace ghoul