-- ERROR PAGE  4xx / 5xx
-- Usage: build_error_page(code, reason)

local ERROR_ICONS = {
    [400] = '&#x26A0;',  -- ⚠
    [401] = '&#x1F512;', -- 🔒
    [403] = '&#x1F6AB;', -- 🚫
    [404] = '&#x1F50D;', -- 🔍
    [405] = '&#x1F6D1;', -- 🛑
    [500] = '&#x1F4A5;', -- 💥
    [503] = '&#x1F6A7;', -- 🚧
}

local ERROR_SUBTITLES = {
    [400] = 'The request could not be understood.',
    [401] = 'Authentication is required.',
    [403] = 'You are not allowed here.',
    [404] = 'The page you are looking for does not exist.',
    [405] = 'That method is not allowed on this resource.',
    [500] = 'Something went wrong on our end.',
    [503] = 'The service is temporarily unavailable.',
}

local ERROR_PAGE_EXTRA_CSS = [===[
<style>
/* reset global div/table rules from main stylesheet */
div {
    width: auto;
    border: none;
}
table, td, th {
    border: none;
    background: transparent;
}

.error-wrap {
    display: flex;
    flex-direction: column;
    align-items: center;
    justify-content: center;
    min-height: 80vh;
    text-align: center;
    padding: 2rem;
    animation: fadeIn 0.5s ease;
}

@keyframes fadeIn {
    from { opacity: 0; transform: translateY(-20px); }
    to   { opacity: 1; transform: translateY(0); }
}

.error-icon {
    font-size: 6rem;
    margin-bottom: 1rem;
    filter: drop-shadow(0 0 18px rgba(227,27,35,0.5));
    animation: pulse 2.5s infinite;
}

@keyframes pulse {
    0%, 100% { transform: scale(1);   }
    50%       { transform: scale(1.08); }
}

.error-code {
    font-size: 7rem;
    font-weight: 900;
    line-height: 1;
    background: linear-gradient(135deg, #e31b23, #ff6b35);
    -webkit-background-clip: text;
    -webkit-text-fill-color: transparent;
    background-clip: text;
    margin-bottom: 0.4rem;
}

.error-reason {
    font-size: 1.6rem;
    font-weight: 700;
    color: #d0d0d0;
    margin-bottom: 0.6rem;
}

.error-subtitle {
    font-size: 1rem;
    color: #6e7173;
    margin-bottom: 2.5rem;
    max-width: 420px;
}

.error-divider {
    width: 60px;
    height: 3px;
    background: linear-gradient(90deg, #e31b23, #ff6b35);
    border-radius: 2px;
    margin: 0 auto 2rem auto;
}

.error-home-btn {
    display: inline-block;
    padding: 0.75rem 2rem;
    border: 2px solid #e31b23;
    color: #afafaf;
    text-decoration: none;
    border-radius: 4px;
    font-size: 0.95rem;
    letter-spacing: 0.05em;
    transition: background 0.25s, color 0.25s;
    background: transparent;
}

.error-home-btn:hover {
    background: #e31b23;
    color: #fff;
}

.fortune-box {
    margin-top: 2.5rem;
    max-width: 520px;
    border: none;
    border-left: 4px solid #e31b23;
    background: hsl(0, 0%, 12%);
    border-radius: 0 6px 6px 0;
    padding: 1rem 1.4rem;
    text-align: left;
    box-shadow: 0 2px 16px rgba(0,0,0,0.4);
}

.fortune-label {
    font-size: 0.75rem;
    letter-spacing: 0.1em;
    text-transform: uppercase;
    color: #6e7173;
    margin-bottom: 0.5rem;
}

.fortune-text {
    font-size: 0.95rem;
    color: #c0c0c0;
    font-style: italic;
    line-height: 1.6;
}
</style>
]===]

local function get_fortune()
    local handle = io.popen('fortune 2>/dev/null')
    if handle == nil then return nil end
    local result = handle:read('*a')
    handle:close()
    if result == nil or result == '' then return nil end
    -- escape HTML special chars
    result = result:gsub('&', '&amp;'):gsub('<', '&lt;'):gsub('>', '&gt;')
    -- preserve line breaks
    result = result:gsub('\n', '<br>')
    return result
end

function build_error_page(code, reason)
    local icon     = ERROR_ICONS[code]     or '&#x2753;'
    local subtitle = ERROR_SUBTITLES[code] or 'An unexpected error occurred.'

    local fortune_html = ''
    local fortune_text = get_fortune()
    if fortune_text ~= nil then
        fortune_html = string.format([===[
<div class="fortune-box">
    <div class="fortune-label">&#x1F4DC; Motto of the day</div>
    <div class="fortune-text">%s</div>
</div>
]===], fortune_text)
    end

    local body = string.format([===[
<div class="error-wrap">
    <div class="error-icon">%s</div>
    <div class="error-code">%d</div>
    <div class="error-reason">%s</div>
    <div class="error-divider"></div>
    <div class="error-subtitle">%s</div>
    <a class="error-home-btn" href="/">&#x2190; Back to Home</a>
    %s
</div>
]===], icon, code, reason, subtitle, fortune_html)

    return '<!DOCTYPE html><html><head><meta charset="UTF-8">'
           .. '<meta name="viewport" content="width=device-width,initial-scale=1">'
           .. '<title>' .. tostring(code) .. ' ' .. reason .. '</title>'
           .. '</head><body>'
           .. HTML_MAIN_STYLE_CSS
           .. ERROR_PAGE_EXTRA_CSS
           .. body
           .. '</body></html>'
end
