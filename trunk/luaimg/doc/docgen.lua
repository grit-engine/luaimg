#!../luaimg.linux.x86_64 -F

local docs = {
    functions = {},
    classes = {}
}

function doc_func(tab, entry)
    entry.params = {}
    entry.returns = {}
    for k=4,#tab do
        if tab[k][1] == "param" then
            local dest = entry.params
            dest[#dest+1] = { name = tab[k][2], kind = tab[k][3], optional = tab[k].optional }
        elseif tab[k][1] == "return" then
            local dest = entry.returns
            dest[#dest+1] = { kind = tab[k][2] }
        end
    end
end

function doc(tab)
    local kind=tab[1]
    local name=tab[2]
    local entry = {
        desc = tab[3],
    }
    if kind == "function" then
        doc_func(tab, entry)
        docs.functions[name] = entry
    elseif kind == "class" then
        entry.fields = {}
        entry.methods = {}
        for k=4,#tab do
            local n = tab[k][2]
            if tab[k][1] == "field" then
                entry.fields[n] = { kind = tab[k][3], desc=tab[k][4] }
            elseif tab[k][1] == "method" then
                local meth = { desc=tab[k][3] }
                doc_func(tab[k], meth)
                entry.methods[n] = meth
            end
        end
        docs.classes[name] = entry
    end
end



local function sorted_keys_from(tab)
    local keys = {}
    for name,_ in pairs(tab) do
        keys[#keys+1] = name
    end
    table.sort(keys)
    return keys
end

local function concat(...)
    local content = ""
    for i=1,select("#",...) do
        content = content..select(i,...)
    end
    return content
end


function emit_api(file)
    file:write('    <h2>Global Functions</h2>\n')
    for _,name in ipairs(sorted_keys_from(docs.functions)) do
        file:write(translate_func(name,docs.functions[name]))
    end

    file:write('    <h2>Classes</h2>\n')
    for _,name in ipairs(sorted_keys_from(docs.classes)) do
        file:write(translate_class(name,docs.classes[name]))
    end
end

local function span(class, ...)
    return "<span class=\""..class.."\">"..concat(...).."</span>"
end
local function div(class, ...)
    return "<div class=\""..class.."\">"..concat(...).."</div>"
end
function para(p)
    return "<p>"..p.."</p>"
end

-- Different whitespace variations
local function spanline(indent, class, ...)
    return indent.."<span class=\""..class.."\">"..concat(...).."</span>".."\n"
end
local function divline(indent, class, ...)
    return indent.."<div class=\""..class.."\">"..concat(...).."</div>".."\n"
end
local function divblk(indent, class, ...)
    return indent.."<div class=\""..class.."\">\n"..concat(...)..indent.."</div>\n"
end


function translate_type(kind)
    if type(kind) == "table" then
        local r = ""
        local sep = "{ "
        for _,v in ipairs(kind) do
            r = r..sep..tostring(v)
            sep = ", "
        end
        return r.." }"
    else
        return kind
    end
end

function translate_func_body(indent, nameclass, descclass, name, entry)
    local paramlist = ""
    local endline = #entry.params>2 and " <br/>" or ""
    for k,v in ipairs(entry.params) do
        local before, after = "", ""
        if v.optional then before, after = "[", "]" end
        paramlist = paramlist .. before ..  span("paramname", v.name)
                              .. span("paramtype", " : "..translate_type(v.kind)) .. after
                              .. (k==#entry.params and "" or span("paramparen",",")
                              .. endline)
    end
    paramlist = divline(indent.."    ","paramlist", paramlist..span("paramparen",")"))

    local returnlist = ""
    if #entry.returns ~= 0 then
        if #entry.returns > 2 then
            for k,v in ipairs(entry.returns) do
                returnlist = returnlist .. span("paramtype", translate_type(v.kind))
                                        .. (k==#entry.returns and "" or span("returncomma",", ").."<br/>")
            end
        else
            for k,v in ipairs(entry.returns) do
                returnlist = returnlist .. span("paramtype", translate_type(v.kind))
                                        .. (k==#entry.returns and "" or span("returncomma",", ")) 
            end
        end
        returnlist = divline(indent.."    ","returnlist", returnlist)
        returnlist = spanline(indent.."    ","returns", "Returns:")..returnlist
        returnlist = divblk(indent, "returns_sec", returnlist)
    end

    return divblk(indent, "signature",
                divline(indent.."    ", nameclass, name, span("paramparen","(")),
                paramlist
            ),
            divline(indent, descclass, para(entry.desc)),
            returnlist
end

function translate_func(name, entry)

    return divblk("    ","function",
            divline("    ","functionhead", name, " (global function)"),
            divblk("    ","functionbody", translate_func_body("        ", "name", "desc", name, entry))
        )
end

function translate_class(name, entry)
    local fieldlist = ""
    local fieldkeys = sorted_keys_from(entry.fields)
    if #fieldkeys ~= 0 then
        for _,k in ipairs(fieldkeys) do
            local v = entry.fields[k]
            fieldlist = fieldlist .. divline("                ","classfield",
                                                span("classfieldname", k),
                                                span("paramtype", " : "..translate_type(v.kind)),
                                                div("classfielddesc", para(v.desc))
                                            )
        end
        fieldlist = divblk("            ","classfieldlist", fieldlist)
        fieldlist = divline("            ","classdivider", "Fields:")..fieldlist
    end


    local methodlist = ""
    local methodkeys = sorted_keys_from(entry.methods)
    if #methodkeys ~= 0 then
        for _,k in ipairs(methodkeys) do
            local v = entry.methods[k]
            methodlist = methodlist .. divblk("                ","classmethod",translate_func_body("                    ", "classmethodname", "classmethoddesc", k, v))
        end
        methodlist = divblk("            ","classmethodlist", methodlist)
        methodlist = divline("            ","classdivider", "Methods:")..methodlist
    end

    return divblk("    ","class",
        divline("        ","classhead",name.." (class)"),
        divblk("        ","classbody",
            divline("            ","desc", para(entry.desc)),
            fieldlist,
            methodlist
        )
    )
end

local code_subs = {
    { '&' , '&amp;' }, 
    { '<' , '&lt;' }, 
    { '>' , '&gt;' }, 
    { ' ', '&nbsp;' },
    { "[0-9.]+", "<span class='codeliteral'>$0</span>" },
    { "true", "<span class='codeliteral'>$0</span>" },
    { "false", "<span class='codeliteral'>$0</span>" },
    { "function", "<span class='codekeyword'>$0</span>" },
    { "return", "<span class='codekeyword'>$0</span>" },
    { ";(end)", ";<span class='codekeyword'>$1</span>" },
    { "\n(end)", "\n<span class='codekeyword'>$1</span>" },
    { "local", "<span class='codekeyword'>$0</span>" },
    { "\"[^\"]*\"", "<span class='codestring'>$0</span>" },
    { '\n', '<br/>' },
}

function emit_code(file, code)
    for _,pair in ipairs(code_subs) do
        code = code:gsub(pair[1], pair[2])
    end
    file:write("<div class='code'>"..code.."</div>\n")
end

