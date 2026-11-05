--[[
ForeverBots_World: the World tab of the main window (after Group finder). How many world bots the realm has: Off, Low, Medium or
High, picked here instead of in the config files (chris 2026-11-03). It is for the whole realm, applies right away (a step down
logs the world bots out first, then the new number fills) and stays after a restart. Only a GM account (the realm owner on a
local realm) can change it; everyone else sees what it is. "Use config file" hands the numbers back to Bots.Size in zz-9-local.conf.
The pick goes to the server as a hidden add-on message (POP_REQ); without that line, as the chat command .bot population <word>.
]]

local ADDON, ns = ...
local Net = ns.Net
local L = ns.L or setmetatable({}, { __index = function(_, k) return (k:gsub("^short:", "")) end })   -- text by client language
local W = {}
ns.World = W

-- word sent to the server, button text, tooltip
local LEVELS = {
    { "off", L["Off"], L["No world bots. The ones in the world now log out. Your own bots, dungeon and battleground bots still work."] },
    { "low", L["Low"], L["A quiet world for a small PC: at most 120 world bots, 15-25 in a zone with you, 50 in a capital."] },
    { "medium", L["Medium"], L["At most 250 world bots, 30-45 in a zone with you, 80 in a capital."] },
    { "high", L["High"], L["A busy world: at most 500 world bots, 50-70 in a zone with you, 120 in a capital. Needs a strong PC."] },
}

-- POP_RESP: level word (or "file"), can set 0/1, logging out 0/1, the server's line, result of a set (ok, denied, bad or empty).
function W.Parse(fields)
    fields = fields or {}
    return { level = fields[1] or "file", canSet = fields[2] == "1", draining = fields[3] == "1", line = fields[4] or "",
             result = fields[5] or "" }
end

local pane, h
W.state = nil

local function Refresh()
    if not pane then return end
    local st = W.state
    for _, b in ipairs(pane.levelButtons) do
        if st and b.word == st.level then b:LockHighlight() else b:UnlockHighlight() end
        b:SetEnabled(st ~= nil and st.canSet)
    end
    pane.file:SetEnabled(st ~= nil and st.canSet and st.level ~= "file")
    if not st then
        pane.status:SetText("|cff9d9d9d" .. L["Asking the server..."] .. "|r")
        pane.who:SetText("")
        return
    end
    local text = L[st.line]
    if st.level == "file" then text = text .. "\n" .. L["Set by the config file (Bots.Size in zz-9-local.conf)."] end
    if st.draining then text = text .. "\n|cffffd100" .. L["The world bots are logging out, then the new number fills."] .. "|r" end
    pane.status:SetText(text)
    pane.who:SetText(st.canSet and "" or ("|cffff6060" .. L["Only the realm owner (a GM account) can change this."] .. "|r"))
end
W.Refresh = Refresh

local function Said(st)
    if not st or st.result == "" then return end
    if st.result == "denied" then
        ns.Print(L["only the realm owner (a GM account) can change the world bots."])
    elseif st.result == "ok" then
        ns.Print(L[st.line])
    end
end

-- word: nil = ask only; "off" .. "high" or "file" = set it.
function W.Request(word)
    if not (Net and Net.Ready()) then
        if word and ns.Queue then
            ns.Queue(".bot population " .. word)   -- the server answers in chat
        end
        W.state = nil
        if pane then pane.status:SetText("|cffff6060" .. L["The server's status cannot be asked (add-on prefix not registered)."] .. "|r") end
        return
    end
    Net.Request("POP_REQ", { word or "" }, function(fields)
        if fields then
            W.state = W.Parse(fields)
            if word then Said(W.state) end
        end
        Refresh()
    end)
end

function ns.BuildWorld(p, helpers)
    pane, h = p, helpers
    local Btn, Section, Tip = h.Btn, h.Section, h.Tip
    Section(p, 0, L["World bots"])
    p.levelButtons = {}
    for i, lv in ipairs(LEVELS) do
        local b = Btn(p, (i - 1) * 60, 22, 58, 24, lv[2], { size = 11 })
        b.word = lv[1]
        b:SetScript("OnClick", function() W.Request(lv[1]) end)
        Tip(b, lv[2], lv[3] .. " " .. L["For the whole realm, applies now and stays after a restart."])
        p.levelButtons[i] = b
    end

    Section(p, 56, L["Now"])
    p.status = p:CreateFontString(nil, "OVERLAY", "GameFontHighlightSmall")
    p.status:SetPoint("TOPLEFT", p, "TOPLEFT", 2, -78)
    p.status:SetWidth(234)
    p.status:SetJustifyH("LEFT")
    p.status:SetJustifyV("TOP")
    p.who = p:CreateFontString(nil, "OVERLAY", "GameFontHighlightSmall")
    p.who:SetPoint("TOPLEFT", p, "TOPLEFT", 2, -150)
    p.who:SetWidth(234)
    p.who:SetJustifyH("LEFT")

    local about = p:CreateFontString(nil, "OVERLAY", "GameFontDisableSmall")
    about:SetPoint("TOPLEFT", p, "TOPLEFT", 2, -172)
    about:SetWidth(234)
    about:SetJustifyH("LEFT")
    about:SetText(L["World bots are the other players you meet in the world. This does not touch your own bots."])

    p.file = Btn(p, 0, 212, 238, 24, L["Use config file"], {})
    p.file:SetScript("OnClick", function() W.Request("file") end)
    Tip(p.file, L["Use config file"], L["Forget the pick made here: the number comes from Bots.Size in zz-9-local.conf again (the launcher's Bots setting)."])

    -- the status while the tab is open
    p:HookScript("OnShow", function() W.Request() end)
    p:HookScript("OnUpdate", function(self, elapsed)
        self.fbAsk = (self.fbAsk or 0) + elapsed
        if self.fbAsk >= 3 then self.fbAsk = 0; W.Request() end
    end)
    Refresh()
end
