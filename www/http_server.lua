dofile('www/htlm_main_page.lua')
dofile('www/html_style.lua')
dofile('www/htlm_visitors_page.lua')

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
    elseif path == '/new_visitor' then
        response:set_status_code(404)
        response:set_reason('Not Found')
    else
        response:set_status_code(404)
        response:set_reason('Not Found at all!')
     end

end

function on_post(request, response)
    local path = request:get_path()

    if path == '/new_visitor' then
        on_post_visitor(request, response)
    end
end

