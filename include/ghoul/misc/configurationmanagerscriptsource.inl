/*****************************************************************************************
 *                                                                                       *
 * GHOUL                                                                                 *
 * General Helpful Open Utility Library                                                  *
 *                                                                                       *
 * Copyright (c) 2012 Alexander Bock                                                     *
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

#include <string>

namespace {

const std::string _configurationScript = "\
config = {} \n \
function merge(t1, t2) \n \
    for k, v in pairs(t2) do \n \
        if (type(v) == 'table') and (type(t1[k] or false) == 'table') then \n \
            merge(t1[k], t2[k]) \n \
        else \n \
            t1[k] = v \n \
        end \n \
    end \n \
    return t1 \n \
end \n \
\n \
function createTables(key, t) \n \
    pos = key:find('[.]') \n \
    if (not pos) then \n \
        pos = key:find('[[]') \n \
        if (pos) then \n \
            key = key:sub(0,pos-1) \n \
        end \n \
        if (not t[key]) then \n \
            t[key] = {} \n \
        end \n \
    else \n \
        newKey = key:sub(0, pos - 1) \n \
        newTable = t[newKey] \n \
        if (not newTable) then \n \
            newTable = {} \n \
            t[newKey] = newTable \n \
        end \n \
        createTables(key:sub(pos + 1), newTable) \n \
    end \n \
end \n \
\n \
function loadConfiguration(file) \n \
    io.input(file) \n \
    contents = io.read('*all') \n \
    source = 'return ' .. contents \n \
    settings = assert(load(source))() \n \
    merge(config, settings or {}) \n \
end \n \
\n \
function loadConfigurationSource(source) \n \
    source = 'return ' .. source \n \
    settings = assert(load(source))() \n \
    merge(config, settings or {}) \n \
end \n \
\n \
function getKeys(location, t) \n \
    t = t or config \n \
    if (location == '') then \n \
        result = {} \n \
        n = 1 \n \
        for k,_ in pairs(t) do \n \
            result[n] = k \n \
            n = n + 1 \n \
        end \n \
        return result \n \
    else \n \
        pos = location:find('[.]') \n \
        if (not pos) then \n \
            newTable = t[location] \n \
            if (newTable) then \n \
                result = {} \n \
                n = 1 \n \
                for k,_ in pairs(newTable) do \n \
                    result[n] = k \n \
                    n = n + 1 \n \
                end \n \
                return result \n \
            else \n \
                -- 'location' does not point to a valid table \n \
                return nil \n \
            end \n \
        else \n \
            newTable = t[location:sub(0, pos - 1)] \n \
            if (not newTable) then \n \
                -- 'location' does not point to a valid table \n \
                return nil \n \
            else \n \
                return getKeys(location:sub(pos + 1), newTable) \n \
            end \n \
        end \n \
    end \n \
end \n \
\n \
function hasKey(key, t) \n \
    return getValue(key, config) \n \
end \n \
\n \
function getValue(key, t) \n \
    return assert(load('return config.' .. key))() \n \
end \n \
\n \
function setValue(key, v, t) \n \
    createTables(key, config) \n \
    assert(load('local x=... config.' .. key .. ' = x'))(v) \n \
end \n \
";
}
