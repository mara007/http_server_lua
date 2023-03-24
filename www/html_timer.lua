-- TIMER PAGE
---------------

HTML_TIMER_STYLE_CSS = [===[
<style>
.rotate {
    animation: rotation 8s infinite linear;
    display: block;
    margin-left: auto;
    margin-right: auto;
    width: 80%;
}

@keyframes rotation {
    from {
        transform: rotate(0deg);
    }
    to {
        transform: rotate(359deg);
    }
}

body {
    background: #000000;
}

.timer {
    color: White;
    font-size: 60;
    font-family:Consolas;
    position: absolute;
    top: 50%;
    left: 50%;
    transform: translate(-50%, -50%);
}
</style>
]===]

HTML_TIMER_PAGE = [===[
<!DOCTYPE html>
<html>
    <head>
        <title>(Type a title for your page here)</title>
        <script type="text/javascript"> 
            function display_c(){
                var refresh=100; // Refresh rate in milli seconds
                mytime=setTimeout('display_ct()',refresh)
            }

            function display_ct() {
                var start = new Date('%s')
                var now = new Date()
                var elapsed_ms = now.getTime() - start.getTime()
                var elapsed_days = elapsed_ms / (1000 * 3600 * 24)

                var html = '<h1 style="color:White;text-align:center">' + elapsed_days.toFixed(5) + '</h1>';
                document.getElementById('ct').innerHTML = html
                tt=display_c();
            }
        </script>
        </head>

        <body onload=display_ct();>
        <img src="www/fav-mav.png" class="rotate" width="512" height="512"/>
        <span id='ct' class='timer'/>

    </body>
</html>
]===]

function on_get_timer(request, response)
    local start = request:get_param('start')

    if start == nil then
        response:set_status_code(400)
        response:set_body('Missing "start" param')
        return
    end

    local html = HTML_TIMER_STYLE_CSS
                 .. string.format(HTML_TIMER_PAGE, start)

    response:set_body(html)
end