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