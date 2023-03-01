----- visitors page
------------------------------------------

HTML_VISITORS_TABLE_HEADER = [===[
<br>
Recent visitors:
<table class="visitors_table">
    <tr>
        <th>No.</th>
        <th>Name</th>
        <th>Last name</th>
        <th>Last visit</th>
    </tr>
]===]

function on_post_visitor(request, response)
    local name = request:get_param('fname')
    local surname = request:get_param('lname')
    if name == '' or surname == '' then
        local html = HTML_MAIN_STYLE_CSS
               .. '<div><h1>Error: you must enter at least one entry!</h1>'
               .. HTML_MAIN_PAGE_RETURN .. '</div>'

        response:set_body(html)
        return
    end

    SHARED_STORAGE.put(name .. ';' .. surname, os.date())
    response:set_body(show_visitors())
end

function on_get_visitors(request, response)
    response:set_body(show_visitors())
end

function show_visitors()
    local visitors_table_html = gen_visitors_table()
    local html = '<div><h1>Hello!</h1>'
                 .. HTML_VISITORS_TABLE_HEADER
                 .. visitors_table_html
                 .. '</table><br>'
                 .. HTML_MAIN_PAGE_RETURN

    return html_head_wrap(html)
end

function gen_visitors_table()
    local visitors = SHARED_STORAGE.keys()

    local html = ''
    for i, name in ipairs(visitors) do
        local date = SHARED_STORAGE.get(name)
        if date == nil then
            date = '##deleted##'
        end

        -- unpack from 'name;surname'
        local fn = string.sub(name, 1, string.find(name, ';')-1)
        local sn = string.sub(name, string.find(name, ';')+1)
        html = html .. string.format('<tr> <td>%s</td> <td>%s</td> <td>%s</td><td>%s</td></tr>\n', i, fn, sn, date)
    end

    return html
end
