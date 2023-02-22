-- test script

function handle_http_message(request, response)
    print('in handle_http_message')

    print('params are' .. request)
    resp = http_resp.new()
    resp:add_header('abc', 'cde')
    resp:add_header('cde', 'fgi')
    local headers = resp:get_header('abc')
    print('stored headers:' .. headers)
    resp:dump()
    resp:set_status_code(400)
    resp:set_reason('BAAD REQUEST')
    resp:dump()
    resp:delete()
end