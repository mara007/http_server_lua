-- test script

function handle_http_message(request, response)
    print('in handle_http_message')

    request:add_header('PATH', request:get_path())
    request:add_header('METHOD', request:get_method())

    response:add_header('TESTED_HEADER', response:get_header('TEST_HEADER'))
    response:set_status_code(200)
    response:set_reason('ITS OK')
end

function test_shared_storage(request, response)
    print('in test_shared_storage')
    SHARED_STORAGE.put('key1', 'value1')
    val = SHARED_STORAGE.get('key1')
    print('shared storage[key1] = ' .. val)
    print('shared storage[key1] = ' .. shared_storage.get('key1'))

    SHARED_STORAGE.put('key_multi', 'val1', 'val2', 'val3', 44)
    val1, val2 = SHARED_STORAGE.get('key_multi')
    print('shared storage[key_multi] = ' .. val1 .. ', '.. val2)

    val_array = table.pack(SHARED_STORAGE.get('key_multi'))
    for i, v in ipairs(val_array) do
        print('unpacked value: i=' .. i .. ' val=' .. v)
    end

    SHARED_STORAGE.put('unpacked_key3', val_array[3])

    for i, k in ipairs(SHARED_STORAGE.keys()) do
        print('key[' .. i ..']: ' .. k)
    end
    for k in {SHARED_STORAGE.keys()} do
        print('key: ' .. k)
    end
end

function prototype_function(data, data2)
    print('PROTOTYPE')
    x = proto_type.new()
    x:set_data('ahoj')
    data:set_data('FROM SKRIPT')
    data2:set_data2('FROM SKRIPT2' .. x:get_data())
    print('data:' .. x:get_data())
    print('data2:' .. data2:get_data2())

    y = proto_type2.new2()
    y:set_data2('data2')
    -- print('new proto2 :'..y:get_data2())
end