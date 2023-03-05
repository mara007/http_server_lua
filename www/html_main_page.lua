-- dofile('www/html_style.lua')

-- MAIN PAGE "/"

HTML_MAIN_PAGE_FORM = [===[
<div>
<h1 class="my-form">Visitors book:</h1>
</br>
<form class="my-form" action="/new_visitor" method="post">
    <label class="my-fomr" for="fname">First name:</label>
    <input class="my-fomr" type="text" id="fname" name="fname"><br><br>
    <label class="my-fomr" for="lname">Last name:</label>
    <input class="my-fomr" type="text" id="lname" name="lname"><br><br>
    <input class="my-fomr" type="submit" value="Submit">
</form>
<br>
<a href="/show_visitors">Show recent visitors..</a>
<br>
<a href="/delete_visitors">Delete visitors book content..</a>
]===]

HTML_MAIN_PAGE_RETURN = '<br><a href="/">Return to main page..</a>'

function on_main_get(request, response)
    local html = '<!DOCTYPE html><html><head>'
                 .. '<link rel="icon" type="image/x-icon" href="/favicon.ico">'
                 .. '<body>'
                 .. HTML_MAIN_STYLE_CSS
                 .. HTML_MAIN_PAGE_FORM
                 .. '</body>'
                 .. '</head></html>'

    response:set_body(html)
end