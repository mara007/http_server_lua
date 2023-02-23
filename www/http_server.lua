dofile('www/htlm_main_page.lua')

-- script

counter=1
function handle_http_message(request, response)
    counter = counter +1

    request:dump()
    print('REQ: method:' .. request:get_method())
    print('REQ: path:' .. request:get_path())
    query_param_msg = request:get_param('msg')
    if query_param_msg ~= nil then
        print('REQ - query param "msg" = ' .. query_param_msg)
    end
    -- 200 OK by default
    response:add_header('abc', 'cde')
    -- response:set_body(on_main_get())
end