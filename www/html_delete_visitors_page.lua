HTML_DELETE_VISITORS_TEMPLATE = [===[
<div>
    <br><h2>list of visitors deleted!!</h2><br>
    Total of %s visitor(s) delete.
    <a href="/">Return to main page..</a>
</div>
]===]

function on_delete_visitors_get(request, response)
    local visitors = SHARED_STORAGE.keys()
    for _, key in ipairs(visitors) do
        SHARED_STORAGE.del(key)
    end
    local html = string.format(HTML_DELETE_VISITORS_TEMPLATE, #visitors)
    response:set_body(html_head_wrap(html))
end