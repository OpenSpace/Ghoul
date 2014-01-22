-------------------------------------------------------------------------------------------
--                                                                                       --
-- GHOUL                                                                                 --
-- General Helpful Open Utility Library                                                  --
--                                                                                       --
-- Copyright (c) 2012 Alexander Bock                                                     --
--                                                                                       --
-- Permission is hereby granted, free of charge, to any person obtaining a copy of this  --
-- software and associated documentation files (the "Software"), to deal in the Software --
-- without restriction, including without limitation the rights to use, copy, modify,    --
-- merge, publish, distribute, sublicense, and/or sell copies of the Software, and to    --
-- permit persons to whom the Software is furnished to do so, subject to the following   --
-- conditions:                                                                           --
--                                                                                       --
-- The above copyright notice and this permission notice shall be included in all copies --
-- or substantial portions of the Software.                                              --
--                                                                                       --
-- THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,   --
-- INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A         --
-- PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT    --
-- HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF  --
-- CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE  --
-- OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                                         --
-------------------------------------------------------------------------------------------

config = {}

function merge(t1, t2)
    for k, v in pairs(t2) do
        if (type(v) == "table") and (type(t1[k] or false) == "table") then
            merge(t1[k], t2[k])
        else
            t1[k] = v
        end
    end
    return t1
end

function createTables(key, t)
    pos = key:find('[.]')
    if (not pos) then
        -- print("notpos", t, key, pos)
        pos = key:find('[[]')
        if (pos) then
            key = key:sub(0,pos-1)
            -- print("modkey", key)
        end
        if (not t[key]) then
            t[key] = {}
        end
    else
        newKey = key:sub(0, pos - 1)
        newTable = t[newKey]
        -- print("newKey,newTable", newKey, newTable)
        if (not newTable) then
            newTable = {}
            t[newKey] = newTable
        end
        -- print("rec")
        -- print("key:sub", key:sub(pos + 1))
        -- print("newTable", newTable)
        createTables(key:sub(pos + 1), newTable)
    end

end

function printTable(t, i)
    i = i or ""
    for k,v in pairs(t) do
        if (type(v) ~= "table") then
            print(i .. k .. ' , ' .. tostring(v))
        else
            print(i .. k)
            printTable(v, i .. " ")
        end
    end
end


function loadConfiguration(file)
    io.input(file)
    contents = io.read("*all")
    source = "return " .. contents
    settings = assert(load(source))()
    
    merge(config, settings or {})
    
    -- printTable(config)
end

function getKeys(location, t)
    t = t or config -- default value of 'config'
    
    if (location == "") then
        result = {}
        n = 1
        for k,_ in pairs(t) do
            result[n] = k
            n = n + 1
        end
        return result
    else
        pos = location:find('[.]')
        if (not pos) then
            newTable = t[location]
            if (newTable) then
                result = {}
                n = 1
                for k,_ in pairs(newTable) do
                    result[n] = k
                    n = n + 1
                end
                return result            
                -- return getKeys("", newTable)
            else
                -- 'location' does not point to a valid table
                return nil
            end
        else
            newTable = t[location:sub(0, pos - 1)]
            if (not newTable) then
                -- 'location' does not point to a valid table
                return nil
            else
                return getKeys(location:sub(pos + 1), newTable)
            end
        end
    end
end

function hasKey(key, t)
    return getValue(key, config)
end

function getValue(key, t)
    return assert(load("return config." .. key))()
end

function setValue(key, v, t)
    createTables(key, config)
    assert(load("local x=... config." .. key .. " = x"))(v)

    --[[
    t = t or config -- default value of 'config'
    pos = key:find('[.]')
    if (not pos) then
        t[key] = v
    else
        newKey = key:sub(0, pos - 1)
        newTable = t[newKey]
        if (not newTable) then
            newTable = {}
            t[newKey] = newTable
        end
        setValue(key:sub(pos + 1), v, newTable)
    end
    ]]
end