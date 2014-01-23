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
        if (not newTable) then
            newTable = {}
            t[newKey] = newTable
        end
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
end