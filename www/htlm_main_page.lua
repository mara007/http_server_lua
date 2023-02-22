dofile('www/html_style.lua')

-- MAIN PAGE "/"

html_main_page = [===[
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

function on_main_get()
    html = '<!DOCTYPE html><html><head>' .. html_main_style_css .. html_main_page .. '</head></html>'
    return html
end