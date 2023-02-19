-- script
counter=1
function handle_http_message(request, response)
    print('in handle_http_message')
    counter = counter +1
    return "<h1> HELLO FROM SCRIPT counter = " ..counter.. "</h1>"
end
