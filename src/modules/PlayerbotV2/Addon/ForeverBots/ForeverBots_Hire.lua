--[[
ForeverBots_Hire: hired bots only (the server's Playerbot.Hire.Mode, chris 2026-11-03). When the realm has hired NPC bots on, a
player cannot make bots of their own: the "Add a bot" class and role buttons on the Bots tab are hidden and a line says where
bots come from instead (the mercenary brokers). Quick party, Call saved and Raid ready stay: they bring the bots you hired. The
Group finder and the dungeon finder still bring bots for that group. The server says whether you must hire (HIRE_REQ / HIRE_RESP);
without the add-on message line nothing is hidden and the server refuses a .bot add itself.
]]

local ADDON, ns = ...
-- Loaded before ForeverBots.lua (the Bots tab is built while that file loads), so the network and text tables are looked up late.
local function L(k) local t = ns.L; return t and t[k] or k end
local H = { must = false }
ns.Hire = H

H.NOTE = "Hired bots only on this realm. Hire bots from a Sellsword Captain at an inn, a Veteran Recruiter in a capital or a Black Market Broker in Gadgetzan, Booty Bay or Ratchet. The Group finder and the dungeon finder still bring bots."

-- HIRE_RESP: must hire 0/1, mode 0/1/2.
function H.Parse(fields)
    fields = fields or {}
    return fields[1] == "1", tonumber(fields[2] or "0") or 0
end

local function Apply()
    local ui = ns.addButtons
    if not ui then return end
    local must = H.must
    for _, cb in ipairs(ui.classes or {}) do cb:SetShown(not must) end
    for _, rb in ipairs(ui.roles or {}) do
        if must then rb:Hide() elseif rb.fbRole then rb:Show() end
    end
    if ui.sel then ui.sel:SetShown(not must) end
    if not ui.note and ui.pane then
        local note = ui.pane:CreateFontString(nil, "OVERLAY", "GameFontHighlightSmall")
        note:SetPoint("TOPLEFT", ui.pane, "TOPLEFT", 2, -24)
        note:SetWidth(234)
        note:SetJustifyH("LEFT")
        note:SetJustifyV("TOP")
        note:SetText("|cffffd100" .. L(H.NOTE) .. "|r")
        ui.note = note
    end
    if ui.note then ui.note:SetShown(must) end
end
H.Apply = Apply

function H.Request()
    local Net = ns.Net
    if not (Net and Net.Ready()) then return end
    Net.Request("HIRE_REQ", {}, function(fields)
        if fields then
            H.must, H.mode = H.Parse(fields)
            Apply()
        end
    end)
end

-- Called by the Bots tab when it is built: ask when the tab shows (the realm's setting can change with a .reload config).
function H.Attach(pane)
    if not pane then return end
    pane:HookScript("OnShow", function() H.Request() end)
    Apply()
    H.Request()
end
