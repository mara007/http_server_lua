dofile('www/favicon.lua')
dofile('www/html_style.lua')
dofile('www/html_main_page.lua')
dofile('www/html_style.lua')
dofile('www/html_visitors_page.lua')
dofile('www/html_delete_visitors_page.lua')

function FILL_SOME_DATA()
    -- flaw - executed from every lua state
    SHARED_STORAGE.put('Jan;Novak', os.date())
    SHARED_STORAGE.put('Tobi;Smith', os.date())
    SHARED_STORAGE.put('Vincet;Smith', os.date())
    SHARED_STORAGE.put('Jarmila;Jebalova', os.date())
 end
FILL_SOME_DATA()

-----------------------------------------------
-- script entry function
-----------------------------------------------
function handle_http_message(request, response)
    local method = request:get_method()

    if method == 'get' then
        on_get(request, response)
    elseif method == 'post' then
        on_post(request, response)
    else
        response:set_status_code(405)
        response:set_reason('Method Not Allowed')
    end
end

-----------------------------------------------

function on_get(request, response)
    local path = request:get_path()

    if path == '/' then
        on_main_get(request, response)
    elseif path == '/delete_visitors' then
        on_delete_visitors_get(request, response)
    elseif path == '/show_visitors' then
        on_get_visitors(request, response)
    elseif path == '/favicon.ico' then
        response:add_header('content-type', 'image/x-icon')
        response:set_body(FAV_ICON)
    elseif path == '/get_file' then
        on_get_file(request, response)
    else
        response:set_status_code(404)
        response:set_reason('Not Found at all!')
        response:set_body(string.format('<h1>Resource not found: <b>%s</b></h1>', path))
     end

end

function on_post(request, response)
    local path = request:get_path()

    if path == '/new_visitor' then
        on_post_visitor(request, response)
    end
end

function on_get_file(request, response)
    local file_name = request:get_param('file_name')
    if file_name == nil then
        response:set_status_code('400')
        response:set_reason('Mandatory parameter "file_name" missing!')
        return
    end

    local content_type = request:get_param('content_type')
    if content_type == nil then
        content_type = 'application/octet-stream'
    end

    local result = response:set_body_from_file(file_name)
    if not result then
        response:set_status_code('404')
        response:set_reason('File not found - ' .. file_name)
        return
    end
    response:add_header('content-type', content_type)
end