-- test script

function handle_http_message(request, response)
    print('in handle_http_message')

    print('params are' .. request .. ', '.. response)
    resp = http_resp.new()
    resp:add_header('abc', 'cde')
    resp:add_header('abc', 'cde')
    print('stored headers:' .. resp:get_header('abc'))
    resp:delete()
end