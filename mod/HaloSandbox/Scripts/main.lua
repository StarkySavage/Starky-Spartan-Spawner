-- Halo Sandbox — Copyright (c) 2026 Ghost7134 — licence MIT.
local UEHelpers = require('UEHelpers')
local catalog = dofile((debug.getinfo(1, 'S').source:gsub('^@', ''):gsub('main%.lua$', 'catalog.lua')))

local VERSION = 'AI TEST 6 - BATTLE SETUP'
-- Le chemin IA ne modifie pas les blocs du scenario: il conserve l'unite
-- active creee par Halo, supprime le mannequin initial et l'ajoute a la
-- fireteam du joueur.
local AI_DIRECT_SPAWN_ENABLED = true
local scriptPath = debug.getinfo(1, 'S').source:gsub('^@', ''):gsub('/', '\\')
local modRoot = scriptPath:match('^(.*)\\Scripts\\main%.lua$')
local nativeDir = modRoot .. '\\Native\\'
local nativeCommand = nativeDir .. 'native_command.txt'
local nativeResult = nativeDir .. 'native_result.txt'
local menuStateFile = nativeDir .. 'menu_state.txt'
local nativeDll = nativeDir .. 'halosandbox_native.dll'
local controllerDll = nativeDir .. 'halosandbox_controller.dll'

local function valid(object)
    if object == nil then return false end
    local ok, result = pcall(function() return object:IsValid() end)
    return ok and result == true
end

local function readFile(path)
    local file = io.open(path, 'rb')
    if not file then return nil end
    local value = file:read('a')
    file:close()
    return value
end

local function writeFile(path, value)
    local file, err = io.open(path, 'wb')
    if not file then return false, err end
    file:write(value)
    local closed, closeError = file:close()
    if not closed then return false, closeError end
    return true
end

local function appendFile(path, value)
    local file, err = io.open(path, 'ab')
    if not file then return false, err end
    file:write(value)
    local closed, closeError = file:close()
    if not closed then return false, closeError end
    return true
end

local function testHistory(message)
    appendFile(modRoot .. '\\AI_TEST_HISTORY.log', os.date('%Y-%m-%d %H:%M:%S ') .. tostring(message) .. '\n')
end
testHistory('AI TEST 6 startup; recovery polling disabled')

local nativeEntry, nativeLoadError = package.loadlib(nativeDll, 'halosandbox_exec')
local nativeQuiesce = package.loadlib(nativeDll, 'halosandbox_quiesce')
local nativeNoclipBridge, nativeNoclipBridgeError =
    package.loadlib(nativeDll, 'halosandbox_noclip_bridge')
local controllerStart, controllerLoadError = package.loadlib(controllerDll, 'halosandbox_controller_start')
local nativeBusy = false
local nativeGeneration = 0
local nativeQueue = {}

local state = {
    open = false,
    page = 'ACCUEIL',
    selection = 1,
    pageOffset = 0,
    status = 'READY',
    ammo = false,
    bottomless = false,
    bottomlessAppliedDatum = nil,
    deathless = false,
    deathlessAppliedDatum = nil,
    noclip = false,
    noclipAppliedDatum = nil,
    noclipBridgeErrorLogged = false,
    jetpack = false,
    advanced = false,
    widget = nil,
    statusLine = nil,
    lineWidgets = {},
    lineSlots = {},
    lines = {},
    uiGeneration = 0,
    redrawBusy = false,
    redrawScheduled = false,
    redrawQueued = false,
    inputTimes = {},
    lastToggleAt = -1000.0,
    lastSpawn = nil,
    aircraftGunnerState = {},
    preloadedAssets = {},
    menuInputLocked = false,
    preloadReady = false,
    preloadInProgress = false,
    preloadRetryScheduled = false,
    preloadLoaded = 0,
    preloadFailed = 0,
    entryGeneration = 0,
    spawnGeneration = 0,
    pelicanSeatPatchGeneration = 0,
    pelicanSeatPatchTag = nil,
    pelicanBoostTag = nil,
    aiScenarioGeneration = 0,
    lastScenarioAI = nil,
    lastActiveAI = nil,
    followWatchActive = false,
    followWatchGeneration = 0,
    followWatchEmptySamples = 0,
    followWatchLastTracked = nil,
    lifecycleGeneration = 0,
    coopAuditSignature = nil,
    coop = { active = false, sessionRunning = false, authority = nil, players = 0 },
}

-- Etat lu par le petit relais XInput natif. Aucune lecture de manette ne se
-- fait dans Unreal : cela evite le crash de cette version du jeu.
local function syncControllerMenuState()
    writeFile(menuStateFile, state.open and '1\n' or '0\n')
end

local MUTATING_NATIVE_COMMANDS = {
    spawn = true,
    spawn_at = true,
    ai_spawn_at = true,
    spawn_enter = true,
    spawn_enter_relative = true,
    enterable = true,
    enter = true,
    enter_immediate = true,
    exit_force = true,
    health = true,
    damageproof = true,
    refill_ammo = true,
    player_powers = true,
    noclip = true,
    noclip_recover = true,
    phase_forward = true,
    teleport = true,
    velocity = true,
    seatflags = true,
    unlockseats = true,
    ai_place_group = true,
    ai_place_squad = true,
    ai_attach = true,
    ai_follow_watch = true,
}

-- Ces commandes ne modifient pas la simulation réseau. Elles ajustent
-- uniquement les définitions de sièges chargées dans la mémoire du processus
-- local. Elles doivent donc être exécutées aussi chez les clients coop, sinon
-- leur jeu continue de considérer le siège pilote du Pélican comme interdit.
local CLIENT_LOCAL_SAFE_NATIVE_COMMANDS = {
    seatflags = true,
    unlockseats = true,
}
local authorizeNativeMutation
local authorizeConsoleMutation

local redraw

local UI_PAGE_NAMES = {
    ACCUEIL = 'HOME',
    VEHICULES = 'VEHICLES',
    ARMES = 'WEAPONS',
    EQUIPEMENTS = 'EQUIPMENT',
    UNITES = 'ACTIVE AI / UNITS',
    OBJETS = 'OBJECTS & MODULES',
    JOUEUR = 'PLAYER',
    OPTIONS = 'OPTIONS',
    IA_ALLIES = 'ALLIED AI',
    IA_ENNEMIS = 'ENEMY AI',
}

local UI_EXACT = {
    ['PRET'] = 'READY',
    ['CHARGEMENT DE LA MISSION...'] = 'LOADING MISSION...',
    ['OUVRE LE MENU UNE FOIS DANS UNE MISSION'] = 'OPEN THE MENU ONCE YOU ARE IN A MISSION',
    ['MODE CLIENT: SPAWNS DESACTIVES'] = 'CLIENT MODE: SPAWNS DISABLED',
    ['MODE HOTE AUTORITAIRE - SPAWNS ACTIFS'] = 'AUTHORITATIVE HOST MODE - SPAWNS ENABLED',
    ['ACTION ANNULEE - VOIR UE4SS.LOG'] = 'ACTION CANCELLED - CHECK UE4SS.LOG',
    ['BUILD HALO NON COMPATIBLE - MOD BLOQUE'] = 'INCOMPATIBLE HALO BUILD - MOD BLOCKED',
    ['PRET - F4 OUVRE / FERME LE MENU'] = 'READY - F4 OPENS / CLOSES THE MENU',
    ['FILTRE MIS A JOUR'] = 'FILTER UPDATED',
    ['BUILD INCOMPATIBLE'] = 'INCOMPATIBLE BUILD',
    ['BUILD HALO COMPATIBLE'] = 'COMPATIBLE HALO BUILD',
    ['BIPED ORIGINAL + ACTEUR BLAM, EQUIPE SYNCHRONISEE'] = 'ORIGINAL BIPED + BLAM ACTOR, TEAM SYNCHRONIZED',
    ['CHECKPOINT: ECHAP > RECHARGER LE POINT DE CONTROLE'] = 'CHECKPOINT: ESC > RELOAD LAST CHECKPOINT',
    ['DIAGNOSTIC IA: AUCUNE IA SUIVIE'] = 'AI DIAGNOSTIC: NO TRACKED AI',
    ['AUDIT IA CREE - UTILISE COLLECTER-DIAGNOSTIC-IA.CMD'] = 'AI AUDIT CREATED - USE COLLECTER-DIAGNOSTIC-IA.CMD',
    ['AUTORITE RESEAU ABSENTE - ACTION RESERVEE A L HOTE'] = 'NETWORK AUTHORITY MISSING - HOST-ONLY ACTION',
    ['CAMERA NON PRETE'] = 'CAMERA NOT READY',
    ['CONTROLEUR DISTANT REFUSE'] = 'REMOTE CONTROLLER DENIED',
    ['CONTROLEUR LOCAL INTROUVABLE'] = 'LOCAL CONTROLLER NOT FOUND',
    ['SIMULATION NON PRETE'] = 'SIMULATION NOT READY',
    ['MUNITIONS INFINIES: REFUSEES'] = 'INFINITE AMMO: DENIED',
    ['MUNITIONS INFINIES: DESACTIVEES'] = 'INFINITE AMMO: DISABLED',
    ['MUNITIONS INFINIES: RESERVE / ENERGIE BLOQUEES'] = 'INFINITE AMMO: RESERVE / ENERGY LOCKED',
    ['INVINCIBILITE: REFUSEE'] = 'INVINCIBILITY: DENIED',
    ['INVINCIBILITE: ACTIVEE (MOTEUR HALO)'] = 'INVINCIBILITY: ENABLED (HALO ENGINE)',
    ['INVINCIBILITE: DESACTIVEE'] = 'INVINCIBILITY: DISABLED',
    ['INVINCIBILITE ACTIVE - SOIN REFUSE'] = 'INVINCIBILITY ENABLED - HEAL DENIED',
    ['INVINCIBILITE: NON - NOCLIP PROTEGE ENCORE'] = 'INVINCIBILITY: NO - NOCLIP STILL PROVIDES PROTECTION',
    ['RESTAURATION MUNITIONS: REFUSEE'] = 'AMMO RESTORE: DENIED',
    ['RESTAURATION NOCLIP REFUSEE'] = 'NOCLIP RESTORE DENIED',
    ['TRAVERSEE: SORS DU VEHICULE'] = 'BOUNDARY CROSSING: EXIT THE VEHICLE',
    ['TRAVERSEE: PROTECTION REFUSEE'] = 'BOUNDARY CROSSING: PROTECTION DENIED',
    ['RETOUR SUR: AUCUN POINT DISPONIBLE POUR CETTE MISSION'] = 'SAFE RETURN: NO SAFE POINT AVAILABLE FOR THIS MISSION',
    ['PELICAN: PILOTAGE, TOURELLE ET BOOST NATIFS PRETS'] = 'PELICAN: NATIVE FLIGHT, TURRET AND BOOST READY',
    ['PELICAN: PILOTAGE/TOURELLE PRETS, BOOST REFUSE'] = 'PELICAN: FLIGHT/TURRET READY, BOOST DENIED',
    ['NOCLIP REFUSE: ETAT DE MISSION INDISPONIBLE'] = 'NOCLIP DENIED: MISSION STATE UNAVAILABLE',
    ['NOCLIP ZONES: ACTIF A PIED ET EN VEHICULE - CONTROLES NORMAUX'] = 'ZONE NOCLIP: ACTIVE ON FOOT AND IN VEHICLES - NORMAL CONTROLS',
    ['NOCLIP DESACTIVE - F7 POUR REVENIR SI TU ES BLOQUE HORS MAP'] = 'NOCLIP DISABLED - PRESS F7 IF YOU ARE STUCK OUTSIDE THE MAP',
}

local UI_REPLACEMENTS = {
    {'ERREUR NATIVE', 'NATIVE ERROR'}, {'ERREUR MOTEUR', 'ENGINE ERROR'},
    {'DLL ABSENTE', 'DLL MISSING'}, {'COMMANDE EN ATTENTE', 'COMMAND QUEUED'},
    {'ECRITURE IMPOSSIBLE', 'WRITE FAILED'}, {'DELAI MOTEUR DEPASSE', 'ENGINE TIMEOUT'},
    {'ACTION RESERVEE A L HOTE RESEAU', 'HOST-ONLY ACTION'},
    {'SPAWN BLOQUE COTE CLIENT - UTILISE LE MENU DE L HOTE', 'SPAWN BLOCKED ON CLIENT - USE THE HOST MENU'},
    {'AUCUN CONTROLEUR LOCAL', 'NO LOCAL CONTROLLER'}, {'AUCUN JOUEUR LOCAL ACTIF', 'NO ACTIVE LOCAL PLAYER'},
    {'INDEX JOUEUR ABSENT', 'PLAYER INDEX MISSING'}, {'INDEX JOUEUR INVALIDE', 'INVALID PLAYER INDEX'},
    {'CHARGEMENT:', 'LOADING:'}, {'TAG INDISPONIBLE DANS CETTE VERSION', 'TAG UNAVAILABLE IN THIS BUILD'},
    {'TAG LOCAL NON CHARGE', 'LOCAL TAG NOT LOADED'}, {'TAG NON CHARGE', 'TAG NOT LOADED'},
    {'TAG LOCAL INDISPONIBLE', 'LOCAL TAG UNAVAILABLE'},
    {'DEBLOCAGE LOCAL REFUSE', 'LOCAL UNLOCK DENIED'}, {'SIEGE PILOTE LOCAL REFUSE', 'LOCAL PILOT SEAT DENIED'},
    {'CONFIRMATION DU SIEGE PILOTE', 'CONFIRMING PILOT SEAT'},
    {'ENTREE ANNULEE: JOUEUR LOCAL MODIFIE', 'ENTRY CANCELLED: LOCAL PLAYER CHANGED'},
    {'PILOTE ACTIF', 'PILOT ACTIVE'}, {'VEHICULE APPARU', 'VEHICLE SPAWNED'},
    {'ENTREE PILOTE REFUSEE', 'PILOT ENTRY DENIED'}, {'ENTREE NON CONFIRMEE', 'ENTRY NOT CONFIRMED'},
    {'VEHICULES: PREPARATION COOP EN COURS', 'VEHICLES: PREPARING CO-OP'},
    {'SPIRIT BLOQUE: COLLISION ET SORTIE INSTABLES', 'SPIRIT BLOCKED: UNSTABLE COLLISION AND EXIT'},
    {'JOUEUR LOCAL A CHANGE', 'LOCAL PLAYER CHANGED'}, {'SORS DU VEHICULE ACTUEL AVANT DE SPAWN', 'EXIT THE CURRENT VEHICLE BEFORE SPAWNING'},
    {'AUTORITE OU JOUEUR LOCAL MODIFIE', 'AUTHORITY OR LOCAL PLAYER CHANGED'},
    {'POSITION JOUEUR INTROUVABLE', 'PLAYER POSITION NOT FOUND'}, {'POSITION JOUEUR INVALIDE', 'INVALID PLAYER POSITION'},
    {'SPAWN ANNULE: AUTORITE PERDUE', 'SPAWN CANCELLED: AUTHORITY LOST'},
    {'APPARITION REFUSEE', 'SPAWN DENIED'}, {'APPARITION VEHICULE NON CONFIRMEE PAR LE MOTEUR', 'VEHICLE SPAWN NOT CONFIRMED BY ENGINE'},
    {'APPARITION NON CONFIRMEE PAR LE MOTEUR', 'SPAWN NOT CONFIRMED BY ENGINE'},
    {'CORPS PHYSIQUE SANS IA', 'PHYSICAL BODY WITHOUT AI'}, {'APPARU COTE HOTE', 'SPAWNED ON HOST'},
    {'SIEGE PILOTE INCOMPATIBLE', 'INCOMPATIBLE PILOT SEAT'}, {'SOIN REFUSE', 'HEAL DENIED'},
    {'VIE ET BOUCLIER RESTAURES', 'HEALTH AND SHIELDS RESTORED'}, {'MUNITIONS INFINIES', 'INFINITE AMMO'},
    {'RESERVE / ENERGIE BLOQUEES', 'RESERVE / ENERGY LOCKED'}, {'INVINCIBILITE', 'INVINCIBILITY'},
    {'ACTIVEE (MOTEUR HALO)', 'ENABLED (HALO ENGINE)'}, {'DESACTIVEE', 'DISABLED'},
    {'DIAGNOSTIC IA', 'AI DIAGNOSTIC'}, {'UNITE ABSENTE OU MORTE', 'UNIT MISSING OR DEAD'},
    {'POSITION JOUEUR INTROUVABLE', 'PLAYER POSITION NOT FOUND'}, {'TRAVERSEE DE LIMITE REFUSEE', 'BOUNDARY CROSSING DENIED'},
    {'LIMITE TRAVERSEE', 'BOUNDARY CROSSED'}, {'PROTECTION REFUSEE', 'PROTECTION DENIED'},
    {'AUCUN POINT DISPONIBLE POUR CETTE MISSION', 'NO SAFE POINT AVAILABLE FOR THIS MISSION'},
    {'RETOUR SUR EFFECTUE', 'SAFE RETURN COMPLETE'}, {'LIMITES ET CONTROLES RESTAURES', 'BOUNDARIES AND CONTROLS RESTORED'},
    {'ETAT DE MISSION INDISPONIBLE', 'MISSION STATE UNAVAILABLE'},
    {'NOCLIP ZONES: ACTIF A PIED ET EN VEHICULE', 'ZONE NOCLIP: ACTIVE ON FOOT AND IN VEHICLES'},
    {'CONTROLES NORMAUX', 'NORMAL CONTROLS'}, {'NOCLIP DESACTIVE', 'NOCLIP DISABLED'},
    {'F7 POUR REVENIR SI TU ES BLOQUE HORS MAP', 'PRESS F7 IF YOU ARE STUCK OUTSIDE THE MAP'},
    {'AUDIT IA IMPOSSIBLE', 'AI AUDIT FAILED'}, {'COMMANDE IA NATIVE BLOQUEE', 'NATIVE AI COMMAND BLOCKED'},
    {'ACTIVE DANS SA ZONE SCENARIO', 'ACTIVE IN ITS SCENARIO AREA'}, {'IA ACTIVES PRES DE TOI', 'ACTIVE AI NEAR YOU'},
    {'VERIFICATION DU PONT IA NATIF', 'CHECKING NATIVE AI BRIDGE'}, {'PONT IA REFUSE', 'AI BRIDGE DENIED'},
    {'MISSION IA NON IDENTIFIEE - ACTION ANNULEE', 'AI MISSION NOT IDENTIFIED - ACTION CANCELLED'},
    {'IA ANNULEE: AUTORITE PERDUE', 'AI CANCELLED: AUTHORITY LOST'}, {'CREATION BLAM', 'BLAM SPAWN'},
    {'IA CREEE MAIS INDEX EXACT ABSENT - ACTION ANNULEE', 'AI CREATED BUT EXACT INDEX IS MISSING - ACTION CANCELLED'},
    {'IA BLAM CREEE(S) - POSITIONNEMENT EXACT', 'BLAM AI CREATED - EXACT PLACEMENT'},
    {'PROFIL PRINCIPAL REFUSE - SECOURS ACTIF', 'PRIMARY PROFILE DENIED - FALLBACK ACTIVE'},
    {'CREATION IA REFUSEE', 'AI SPAWN DENIED'}, {'IA TOUTES MISSIONS: CREATION DIRECTE BLOQUEE PAR SECURITE', 'ALL-MISSION AI: DIRECT SPAWN BLOCKED FOR SAFETY'},
    {'TYPE D UNITE INTROUVABLE', 'UNIT TYPE NOT FOUND'}, {'VARIANTE IA INTROUVABLE', 'AI VARIANT NOT FOUND'},
    {'JOUEUR A CHANGE', 'PLAYER CHANGED'}, {'PACK ANNULE: AUTORITE PERDUE', 'PACK CANCELLED: AUTHORITY LOST'},
    {'IA DEGAGEE D UN OBSTACLE', 'AI FREED FROM AN OBSTACLE'}, {'JOUEURS=', 'PLAYERS='},
    {' IA ACTIVES', ' ACTIVE AI'}, {'ACTIFS x10', 'ACTIVE x10'}, {'ACTIF x1', 'ACTIVE x1'},
    {' [INERTE]', ' [INERT]'}, {'FUSIL D ASSAUT', 'ASSAULT RIFLE'}, {'FUSIL DE COMBAT', 'BATTLE RIFLE'},
}

local function replacePlain(text, from, to)
    local start = 1
    while true do
        local first, last = text:find(from, start, true)
        if not first then return text end
        text = text:sub(1, first - 1) .. to .. text:sub(last + 1)
        start = first + #to
    end
end

local function uiText(value)
    local text = tostring(value or '')
    if UI_EXACT[text] then return UI_EXACT[text] end
    for _, rule in ipairs(UI_REPLACEMENTS) do text = replacePlain(text, rule[1], rule[2]) end
    return text
end

local function setStatus(value)
    state.status = uiText(value)
    if state.open and valid(state.statusLine) then
        local ok = pcall(function() state.statusLine:SetText(FText(state.status)) end)
        if ok then return end
    end
    if state.open and redraw then redraw() end
end

local runNative

local function drainNativeQueue()
    if nativeBusy or #nativeQueue == 0 then return end
    local nextCommand = table.remove(nativeQueue, 1)
    runNative(nextCommand.command, nextCommand.callback)
end

runNative = function(command, callback)
    local verb = tostring(command or ''):match('^%s*([^%s]+)')
    if verb then verb = verb:lower() end
    local enabled = { observer=true, escort_status=true, probe=true, item_spawn=true, ai_control=true, origin=true, findtag=true, loadtag=true, ai_spawn_at=true, ai_follow_watch=true, controller_start=true }
    if not enabled[verb] then
        setStatus('NOT ENABLED IN AI TEST')
        if callback then callback(nil, 'phase1_feature_disabled') end
        return false
    end
    if verb~='escort_status' then testHistory('request: ' .. tostring(command)) end
    if verb~='escort_status' then print('[HaloSandbox AI6] request: ' .. tostring(command) .. '\n') end
    if verb and MUTATING_NATIVE_COMMANDS[verb] and not CLIENT_LOCAL_SAFE_NATIVE_COMMANDS[verb]
        and type(authorizeNativeMutation) == 'function' then
        local allowed, reason = authorizeNativeMutation(command)
        if not allowed then
            local message = reason or 'ACTION RESERVEE A L HOTE RESEAU'
            setStatus(message)
            print(string.format('[HaloSandbox] Commande native bloquee sur client: %s\n', tostring(command)))
            if callback then callback(nil, 'client_not_authoritative') end
            return false
        end
    end
    if type(nativeEntry) ~= 'function' then
        setStatus('ERREUR NATIVE: ' .. tostring(nativeLoadError or 'DLL ABSENTE'))
        if callback then callback(nil, 'native unavailable') end
        return false
    end
    if nativeBusy then
        nativeQueue[#nativeQueue + 1] = { command = command, callback = callback }
        setStatus(string.format('COMMANDE EN ATTENTE (%d)', #nativeQueue))
        return true
    end
    nativeBusy = true
    nativeGeneration = nativeGeneration + 1
    local generation = nativeGeneration
    local lifecycleGeneration = state.lifecycleGeneration
    writeFile(nativeResult, '')
    local ok, err = writeFile(nativeCommand, command)
    if not ok then
        nativeBusy = false
        drainNativeQueue()
        setStatus('ECRITURE IMPOSSIBLE: ' .. tostring(err))
        if callback then callback(nil, err) end
        return false
    end
    local called, callError = pcall(nativeEntry)
    if not called then
        nativeBusy = false
        drainNativeQueue()
        setStatus('ERREUR MOTEUR: ' .. tostring(callError))
        if callback then callback(nil, callError) end
        return false
    end
    local attempts = 0
    local function poll()
        if generation ~= nativeGeneration then return end
        attempts = attempts + 1
        local result = readFile(nativeResult) or ''
        result = result:gsub('%s+$', '')
        if result ~= '' and result ~= 'queued' then
            if verb~='escort_status' then testHistory('result for ' .. tostring(command) .. ': ' .. result) end
            if verb~='escort_status' then print('[HaloSandbox AI6] result: ' .. result .. '\n') end
            nativeBusy = false
            drainNativeQueue()
            if lifecycleGeneration ~= state.lifecycleGeneration then
                print(string.format('[HaloSandbox] Resultat natif obsolete ignore apres changement de mission: %s\n',
                    tostring(command)))
                return
            end
            if callback then callback(result, result:match('^error') and result or nil) end
            return
        end
        if attempts >= 150 then
            nativeBusy = false
            drainNativeQueue()
            if lifecycleGeneration ~= state.lifecycleGeneration then
                print(string.format('[HaloSandbox] Delai natif obsolete ignore apres changement de mission: %s\n',
                    tostring(command)))
                return
            end
            setStatus('DELAI MOTEUR DEPASSE')
            if callback then callback(nil, 'timeout') end
            return
        end
        ExecuteInGameThreadWithDelay(20, poll)
    end
    ExecuteInGameThreadWithDelay(20, poll)
    return true
end

local function controllerIsLocal(controller)
    if not valid(controller) then return false end
    local ok, result = pcall(function() return controller:IsLocalController() end)
    if ok and result == true then return true end
    ok, result = pcall(function() return controller:IsLocalPlayerController() end)
    return ok and result == true
end

-- UEHelpers.GetPlayerController() accepte le premier PlayerController trouve.
-- Sur un listen-server, cela peut etre le controleur du joueur distant. On
-- exige ici un controleur explicitement local et, si possible, celui rattache
-- au LocalPlayer du viewport de cette machine.
local function getLocalPlayerController()
    local viewport = UEHelpers.GetGameViewportClient()
    if valid(viewport) then
        local okGameInstance, gameInstance = pcall(function() return viewport.GameInstance end)
        if okGameInstance and valid(gameInstance) then
            local okPlayers, localPlayers = pcall(function() return gameInstance.LocalPlayers end)
            if okPlayers and localPlayers then
                for index = 1, #localPlayers do
                    local localPlayer = localPlayers[index]
                    if valid(localPlayer) then
                        local okController, controller = pcall(function() return localPlayer.PlayerController end)
                        if okController and valid(controller) and controllerIsLocal(controller) then
                            return controller
                        end
                    end
                end
            end
        end
    end

    local controllers = FindAllOf('PlayerController') or FindAllOf('Controller') or {}
    local best, bestScore = nil, -1
    for _, controller in ipairs(controllers) do
        if valid(controller) and controllerIsLocal(controller) then
            local score = 100
            local okPawn, pawn = pcall(function() return controller.Pawn end)
            if okPawn and valid(pawn) then score = score + 20 end
            local okCamera, camera = pcall(function() return controller.PlayerCameraManager end)
            if okCamera and valid(camera) then score = score + 5 end
            if score > bestScore then best, bestScore = controller, score end
        end
    end
    return best
end

local function getLocalWorld()
    local controller = getLocalPlayerController()
    if valid(controller) then
        local ok, world = pcall(function() return controller:GetWorld() end)
        if ok and valid(world) then return world end
    end
    local gameInstance = UEHelpers.GetGameInstance()
    if valid(gameInstance) then
        local ok, world = pcall(function() return gameInstance:GetWorld() end)
        if ok and valid(world) then return world end
    end
    return nil
end

local function console(command)
    local normalized = tostring(command or ''):lower():gsub('^%s+', '')
    local mutating = normalized:find('object_create', 1, true) == 1
        or normalized:find('cheat_', 1, true) == 1
        or normalized:find('game_revert', 1, true) == 1
        or normalized:find('ai_', 1, true) == 1
    if mutating and type(authorizeConsoleMutation) == 'function' then
        local allowed, reason = authorizeConsoleMutation(command)
        if not allowed then
            setStatus(reason or 'ACTION RESERVEE A L HOTE RESEAU')
            print(string.format('[HaloSandbox] Commande console bloquee sur client: %s\n', tostring(command)))
            return false, 'client_not_authoritative'
        end
    end
    local controller = getLocalPlayerController()
    local world = getLocalWorld()
    if not valid(controller) or not valid(world) then return false, 'console_context_unavailable' end

    local ok, output = pcall(function() return controller:ConsoleCommand(command, true) end)
    if ok then
        local text = output == nil and '' or tostring(output)
        print(string.format('[HaloSandbox] Console: %s => %s\n', tostring(command), text ~= '' and text or '<aucun retour>'))
        return true, text
    end

    local library = StaticFindObject('/Script/Engine.Default__KismetSystemLibrary')
    if valid(library) then
        local fallbackOk, fallbackOutput = pcall(function()
            return library:ExecuteConsoleCommand(world, command, controller)
        end)
        if fallbackOk then
            local text = fallbackOutput == nil and '' or tostring(fallbackOutput)
            print(string.format('[HaloSandbox] Console secours: %s => %s\n', tostring(command), text ~= '' and text or '<aucun retour>'))
            return true, text
        end
    end
    return false, tostring(output or 'console_failed')
end

local function normalizeNetMode(value)
    local numeric = nil
    if type(value) == 'number' then numeric = value
    elseif type(value) == 'string' then numeric = tonumber(value)
    else numeric = tonumber(tostring(value or '')) end
    if numeric and numeric >= 0 and numeric <= 3 then return numeric end
    local name = tostring(value or ''):lower()
    if name:find('standalone', 1, true) then return 0 end
    if name:find('dedicatedserver', 1, true) or name:find('dedicated_server', 1, true) then return 1 end
    if name:find('listenserver', 1, true) or name:find('listen_server', 1, true) then return 2 end
    if name:find('client', 1, true) then return 3 end
    return nil
end

local function readNetMode(controller, pawn, actor, world)
    local candidates = {}
    for _, object in pairs({ pawn = pawn, controller = controller, actor = actor, world = world }) do
        if valid(object) then candidates[#candidates + 1] = object end
    end
    for _, object in ipairs(candidates) do
        if valid(object) then
            local ok, mode = pcall(function() return object:GetNetMode() end)
            if ok then
                local normalized = normalizeNetMode(mode)
                if normalized ~= nil then return normalized end
            end
        end
    end
    return nil
end

local function auditCoop(forceLog)
    local report = {
        active = false,
        sessionRunning = false,
        authority = nil,
        players = 0,
        netMode = nil,
        localController = false,
    }
    local controller = getLocalPlayerController()
    report.localController = valid(controller) and controllerIsLocal(controller)
    local pawn = valid(controller) and controller.Pawn or nil
    local actor = nil
    if valid(pawn) then
        local okActor, result = pcall(function() return pawn:GetBlamObjectActor() end)
        if okActor and valid(result) then actor = result end
        local okAuthority, authority = pcall(function() return pawn:HasAuthority() end)
        if okAuthority then report.authority = authority == true end
    end
    local world = getLocalWorld()
    report.netMode = readNetMode(controller, pawn, actor, world)

    local okAudio, audioObjects = pcall(FindAllOf, 'BlamEngineAudioGameSubsystem')
    if okAudio and audioObjects then
        for _, audio in ipairs(audioObjects) do
            if valid(audio) then
                local ok, coop = pcall(function() return audio:IsNetworkCoop() end)
                if ok and coop == true then report.active = true break end
            end
        end
    end
    local okState, states = pcall(FindAllOf, 'BlamNetworkGameStateComponent')
    if okState and states then
        for _, networkState in ipairs(states) do
            if valid(networkState) then
                local ok, running = pcall(function() return networkState.bSessionRunning end)
                if ok and running == true then report.sessionRunning = true break end
            end
        end
    end
    local okPlayers, players = pcall(FindAllOf, 'BlamNetworkPlayerStateComponent')
    if okPlayers and players then
        for _, player in ipairs(players) do
            if valid(player) then report.players = report.players + 1 end
        end
    end
    if report.netMode == 1 or report.netMode == 2 or report.netMode == 3 or report.players > 1 then
        report.active = true
    end
    state.coop = report
    local signature = table.concat({
        tostring(report.active), tostring(report.sessionRunning), tostring(report.localController),
        tostring(report.authority), tostring(report.netMode), tostring(report.players),
    }, '|')
    if forceLog or signature ~= state.coopAuditSignature then
        print(string.format('[HaloSandbox] COOP audit: active=%s session=%s local=%s authority=%s netMode=%s networkPlayers=%d\n',
            tostring(report.active), tostring(report.sessionRunning), tostring(report.localController),
            tostring(report.authority), tostring(report.netMode), report.players))
        state.coopAuditSignature = signature
    end
    return report
end

-- GetNetMode est prioritaire : 2 = listen-server (hote), 3 = client.
-- HasAuthority n'est utilise qu'en repli si la fonction n'est pas refletee
-- par cette build. En cas de doute dans une vraie session reseau, on bloque
-- plutot que de creer un objet local qui sera corrige/efface par le serveur.
local function hostFeaturesAllowed()
    local coop = auditCoop()
    if not coop.localController then return false, coop, 'CONTROLEUR LOCAL INTROUVABLE' end
    if coop.netMode == 0 then return true, coop end
    if coop.netMode == 1 or coop.netMode == 2 then return true, coop end
    if coop.netMode == 3 then return false, coop, 'SPAWN BLOQUE COTE CLIENT - UTILISE LE MENU DE L HOTE' end
    if coop.active or coop.sessionRunning or coop.players > 1 then
        if coop.authority == true then return true, coop end
        return false, coop, 'AUTORITE RESEAU ABSENTE - ACTION RESERVEE A L HOTE'
    end
    return true, coop
end

local function playerContext(requireAuthority)
    local controller = getLocalPlayerController()
    if not valid(controller) then return nil, 'AUCUN CONTROLEUR LOCAL' end
    if not controllerIsLocal(controller) then return nil, 'CONTROLEUR DISTANT REFUSE' end
    local pawn = controller.Pawn
    if not valid(pawn) then return nil, 'AUCUN JOUEUR LOCAL ACTIF' end
    if requireAuthority then
        local allowed, _, reason = hostFeaturesAllowed()
        if not allowed then return nil, reason end
    end
    local okActor, actor = pcall(function() return pawn:GetBlamObjectActor() end)
    if not okActor or not valid(actor) then return nil, 'SIMULATION NON PRETE' end
    local sync = actor.BlamObjectSynchronization
    if not valid(sync) then return nil, 'INDEX JOUEUR ABSENT' end
    local datum = tonumber(sync.BlamObjectIndex)
    if not datum or datum == -1 then return nil, 'INDEX JOUEUR INVALIDE' end
    return { controller = controller, pawn = pawn, actor = actor, sync = sync, datum = datum }
end

authorizeNativeMutation = function()
    local allowed, _, reason = hostFeaturesAllowed()
    return allowed, reason
end
authorizeConsoleMutation = authorizeNativeMutation

local function auditFireteamLeader()
    local classNames = {
        'BlamOnlineSessionSubsystem',
        'MeteoriteSquadLobbyViewModel',
        'BlamNetworkPlayerStateComponent',
        'BlamNetworkGameStateComponent',
    }
    for _, className in ipairs(classNames) do
        local okObjects, objects = pcall(FindAllOf, className)
        local objectCount = okObjects and objects and #objects or 0
        print(string.format('[HaloSandbox] LEADER %s count=%d\n', className, objectCount))
        if objectCount > 0 then
            for objectIndex, object in ipairs(objects) do
                if valid(object) then
                    local okClass, class = pcall(function() return object:GetClass() end)
                    local seen = {}
                    while okClass and valid(class) do
                        pcall(function()
                            class:ForEachProperty(function(property)
                                local okName, name = pcall(function() return property:GetFName():ToString() end)
                                local key = tostring(name)
                                local lower = tostring(name):lower()
                                if okName and not seen[key] and
                                     (lower:find('leader', 1, true) or lower:find('host', 1, true) or
                                    lower:find('primary', 1, true) or lower:find('local', 1, true) or
                                      lower:find('playerid', 1, true) or lower:find('endpoint', 1, true)) then
                                    seen[key] = true
                                    local okValue, value = pcall(function() return object:GetPropertyValue(name) end)
                                    print(string.format('[HaloSandbox] LEADER %s[%d].%s=%s\n',
                                        className, objectIndex, tostring(name), tostring(okValue and value or '<illisible>')))

                                    -- UE4SS affiche les UScriptStruct sous forme d'adresse. On developpe donc
                                    -- leurs champs, notamment PrimaryPlayerId, pour obtenir l'identifiant reel.
                                    local okStruct, struct = pcall(function() return property:GetStruct() end)
                                    if okValue and okStruct and valid(struct) and value ~= nil then
                                        pcall(function()
                                            struct:ForEachProperty(function(field)
                                                local okFieldName, fieldName = pcall(function()
                                                    return field:GetFName():ToString()
                                                end)
                                                if okFieldName and fieldName then
                                                    local okFieldValue, fieldValue = pcall(function()
                                                        return value[fieldName]
                                                    end)
                                                    print(string.format('[HaloSandbox] LEADER %s[%d].%s.%s=%s\n',
                                                        className, objectIndex, tostring(name), tostring(fieldName),
                                                        tostring(okFieldValue and fieldValue or '<illisible>')))
                                                end
                                            end)
                                        end)
                                    end
                                end
                            end)
                        end)
                        local okSuper, super = pcall(function() return class:GetSuperStruct() end)
                        class = okSuper and super or nil
                    end
                end
            end
        end
    end
    print('[HaloSandbox] LEADER audit termine.\n')
end

local function parsePose(result)
    if not result then return nil end
    local px, py, pz, fx, fy, fz, ux, uy, uz = result:match(
        'position=([%-%d%.]+),([%-%d%.]+),([%-%d%.]+) forward=([%-%d%.]+),([%-%d%.]+),([%-%d%.]+) up=([%-%d%.]+),([%-%d%.]+),([%-%d%.]+)')
    if not px then return nil end
    return {
        p = { x = tonumber(px), y = tonumber(py), z = tonumber(pz) },
        f = { x = tonumber(fx), y = tonumber(fy), z = tonumber(fz) },
        u = { x = tonumber(ux), y = tonumber(uy), z = tonumber(uz) },
    }
end

local function actorBasis(actor)
    local p = actor:K2_GetActorLocation()
    local f = actor:GetActorForwardVector()
    local r = actor:GetActorRightVector()
    return p, f, r
end

local function lineTrace(startPoint, endPoint, ignoreActor)
    local system = UEHelpers.GetKismetSystemLibrary()
    if not valid(system) then return nil end
    local hit = {}
    local transparent = { R = 0, G = 0, B = 0, A = 0 }
    local ok, wasHit = pcall(function()
        return system:LineTraceSingle(ignoreActor, startPoint, endPoint, 0, false, {}, 0,
            hit, true, transparent, transparent, 0.0)
    end)
    if not ok or not wasHit or not hit.ImpactPoint then return nil end
    return hit
end

local function nativeFromUE(context, pose, uePoint)
    local originUE, forwardUE, rightUE = actorBasis(context.actor)
    local dx, dy, dz = uePoint.X - originUE.X, uePoint.Y - originUE.Y, uePoint.Z - originUE.Z
    local localForward = (dx * forwardUE.X + dy * forwardUE.Y + dz * forwardUE.Z) / 100.0
    local localRight = (dx * rightUE.X + dy * rightUE.Y + dz * rightUE.Z) / 100.0
    local localUp = dz / 100.0
    local rightNative = { x = -pose.f.y, y = pose.f.x, z = 0.0 }
    return {
        x = pose.p.x + pose.f.x * localForward + rightNative.x * localRight + pose.u.x * localUp,
        y = pose.p.y + pose.f.y * localForward + rightNative.y * localRight + pose.u.y * localUp,
        z = pose.p.z + pose.f.z * localForward + rightNative.z * localRight + pose.u.z * localUp,
    }
end

local function safePlacement(context, pose, item)
    local originUE, forwardUE = actorBasis(context.actor)
    if item.kind == 'vehicle' and not item.attachment then
        local pelican = item.path:find('pelican', 1, true)
        local aircraft = pelican or item.path:find('tuning_fork', 1, true)
            or item.path:find('banshee', 1, true) or item.path:find('seraph', 1, true)
        -- Dans les BSP interieurs, le proxy Unreal et la simulation Blam ne
        -- partagent pas toujours le meme repere. Le calcul sera effectue par
        -- le pont natif depuis l'origine Blam exacte du joueur.
        return {
            relative = true,
            forward = pelican and 3.25 or (aircraft and 2.50 or 2.00),
            up = pelican and 1.60 or (aircraft and 1.00 or 0.55),
        }
    end

    local camera = context.controller.PlayerCameraManager
    if valid(camera) then
        local startPoint = camera:GetCameraLocation()
        local cameraForward = UEHelpers.GetKismetMathLibrary():GetForwardVector(camera:GetCameraRotation())
        local endPoint = {
            X = startPoint.X + cameraForward.X * 400.0,
            Y = startPoint.Y + cameraForward.Y * 400.0,
            Z = startPoint.Z + cameraForward.Z * 400.0,
        }
        local hit = lineTrace(startPoint, endPoint, context.actor)
        local point = hit and hit.ImpactPoint or endPoint
        point = { X = point.X, Y = point.Y, Z = point.Z + (item.kind == 'biped' and 95.0 or 35.0) }
        return nativeFromUE(context, pose, point)
    end
    return nil, 'CAMERA NON PRETE'
end

local function resolveTag(item, callback)
    setStatus('CHARGEMENT: ' .. item.label)
    ExecuteInGameThread(function()
        local asset = state.preloadedAssets[item.asset]
        local okLoad = valid(asset)
        if not okLoad then okLoad, asset = pcall(LoadAsset, item.asset) end
        if not okLoad or not valid(asset) then
            setStatus('TAG INDISPONIBLE DANS CETTE VERSION')
            callback(nil)
            return
        end
        state.preloadedAssets[item.asset] = asset
        ExecuteInGameThreadWithDelay(30, function()
            runNative('findtag ' .. item.group .. ' ' .. item.path, function(result, err)
                local datum = result and tonumber(result:match('datum=([%-%d]+)'))
                if not err and datum then
                    callback(datum)
                    return
                end
                -- LoadAsset rend le paquet UE resident, mais les missions qui
                -- n'utilisent pas deja ce contenu ne l'ajoutent pas toujours a
                -- la table Blam. Le chargeur natif officiel termine alors le
                -- chargement du tag original, sans toucher au scenario.
                runNative('loadtag ' .. item.group .. ' ' .. item.path, function(loadResult, loadError)
                    local loadedDatum = loadResult and tonumber(loadResult:match('datum=([%-%d]+)'))
                    if loadError or not loadedDatum then
                        setStatus('TAG NON CHARGE: ' .. item.label)
                        print(string.format('[HaloSandbox] Chargement Blam refuse label=%s find=%s load=%s\n',
                            tostring(item.label), tostring(result or err), tostring(loadResult or loadError)))
                        callback(nil)
                        return
                    end
                    print(string.format('[HaloSandbox] Tag Blam dynamique charge label=%s datum=%d\n',
                        tostring(item.label), loadedDatum))
                    callback(loadedDatum)
                end)
            end)
        end)
    end)
end


-- Le Pélican de campagne possède un siège pilote masqué/verrouillé dans les
-- données de tag. Les données de tag ne sont pas répliquées : chaque joueur
-- doit appliquer ce correctif dans SON propre processus. La création du
-- véhicule reste, elle, strictement réservée à l'hôte.
local function patchPelicanSeatsLocal(callback, silent)
    do if callback then callback(false) end return end
    state.pelicanSeatPatchGeneration = state.pelicanSeatPatchGeneration + 1
    local generation = state.pelicanSeatPatchGeneration
    local pelican = nil
    for _, item in ipairs(catalog) do
        if item.kind == 'vehicle' and not item.attachment
            and item.path:find('\\pelican\\pelican', 1, true) then
            pelican = item
            break
        end
    end
    if not pelican then
        if callback then callback(false, 'pelican_catalog_missing') end
        return
    end

    ExecuteInGameThread(function()
        local asset = state.preloadedAssets[pelican.asset]
        local okLoad = valid(asset)
        if not okLoad then okLoad, asset = pcall(LoadAsset, pelican.asset) end
        if not okLoad or not valid(asset) then
            if not silent then setStatus('PELICAN: TAG LOCAL INDISPONIBLE') end
            if callback then callback(false, 'pelican_asset_unavailable') end
            return
        end
        state.preloadedAssets[pelican.asset] = asset

        ExecuteInGameThreadWithDelay(40, function()
            if generation ~= state.pelicanSeatPatchGeneration then return end
            if nativeBusy then
                ExecuteInGameThreadWithDelay(250, function()
                    if generation == state.pelicanSeatPatchGeneration then
                        patchPelicanSeatsLocal(callback, silent)
                    end
                end)
                return
            end
            local function unlockLoadedPelican(tagDatum)
                if generation ~= state.pelicanSeatPatchGeneration then return end
                state.pelicanSeatPatchTag = tagDatum
                local function unlockSeatsAfterBoost(boostReady, boostResult)
                    runNative(string.format('unlockseats %d', tagDatum), function(unlockResult, unlockError)
                    if generation ~= state.pelicanSeatPatchGeneration then return end
                    if unlockError then
                        if not silent then setStatus('PELICAN: DEBLOCAGE LOCAL REFUSE') end
                        if callback then callback(false, unlockError) end
                        return
                    end
                    -- Architecture stable : le siege 0 est pilote (0x04),
                    -- tireur (0x08) et camera 3e personne (0x10). La tourelle
                    -- enfant d'origine reste l'unique arme du Pelican.
                    runNative(string.format('seatflags %d 0 0000001C 000002A0', tagDatum),
                        function(flagResult, flagError)
                            if generation ~= state.pelicanSeatPatchGeneration then return end
                            if flagError then
                                if not silent then setStatus('PELICAN: SIEGE PILOTE LOCAL REFUSE') end
                                if callback then callback(false, flagError) end
                                return
                            end
                            if not silent then
                                print(string.format('[HaloSandbox] Pelican local seats patched: tag=%d boost=%s unlock=%s flags=%s\n',
                                    tagDatum, tostring(boostResult), tostring(unlockResult), tostring(flagResult)))
                                setStatus(boostReady
                                    and 'PELICAN: PILOTAGE, TOURELLE ET BOOST NATIFS PRETS'
                                    or 'PELICAN: PILOTAGE/TOURELLE PRETS, BOOST REFUSE')
                            end
                            if callback then callback(true, nil, tagDatum) end
                        end)
                    end)
                end
                -- Le Pelican retail possede deja sa puissance (3.0), sa courbe
                -- LT/Maj et son controleur human-plane. Le pont natif active
                -- uniquement son bit "unit has boost", une fois par tag. Une
                -- erreur ne doit jamais empecher le vehicule stable de spawn.
                if state.pelicanBoostTag == tagDatum then
                    unlockSeatsAfterBoost(true, 'ok pelican_boost cached=1')
                    return
                end
                runNative(string.format('pelican_boost %d', tagDatum),
                    function(boostResult, boostError)
                        if generation ~= state.pelicanSeatPatchGeneration then return end
                        local boostReady = not boostError and boostResult
                            and boostResult:find('^ok pelican_boost') ~= nil
                        if boostReady then
                            state.pelicanBoostTag = tagDatum
                        end
                        if not boostReady then
                            print('[HaloSandbox] Boost Pelican refuse sans bloquer le spawn: '
                                .. tostring(boostError or boostResult) .. '\n')
                        end
                        unlockSeatsAfterBoost(boostReady, boostResult or boostError)
                    end)
            end
            runNative('findtag ' .. pelican.group .. ' ' .. pelican.path, function(result, findError)
                if generation ~= state.pelicanSeatPatchGeneration then return end
                local tagDatum = result and tonumber(result:match('datum=([%-%d]+)'))
                if findError or not tagDatum then
                    -- Une mission sans Pelican n'inscrit pas ce tag dans la
                    -- table Blam. Charge d'abord le contenu original, puis
                    -- applique exactement le meme correctif local de sieges.
                    runNative('loadtag ' .. pelican.group .. ' ' .. pelican.path,
                        function(loadResult, loadError)
                            if generation ~= state.pelicanSeatPatchGeneration then return end
                            local loadedDatum = loadResult and tonumber(loadResult:match('datum=([%-%d]+)'))
                            if loadError or not loadedDatum then
                                if not silent then setStatus('PELICAN: TAG LOCAL NON CHARGE') end
                                if callback then callback(false, loadError or findError or 'pelican_tag_not_loaded') end
                                return
                            end
                            print(string.format('[HaloSandbox] Pelican charge dynamiquement pour cette mission: tag=%d\n',
                                loadedDatum))
                            unlockLoadedPelican(loadedDatum)
                        end)
                    return
                end
                unlockLoadedPelican(tagDatum)
            end)
        end)
    end)
end

local function ensureVehicleEntry(unitDatum, vehicleDatum, tagDatum, label)
    state.entryGeneration = state.entryGeneration + 1
    local generation = state.entryGeneration
    setStatus('CONFIRMATION DU SIEGE PILOTE...')
    ExecuteInGameThreadWithDelay(350, function()
        if generation ~= state.entryGeneration then return end
        local current, contextError = playerContext(true)
        if not current or current.datum ~= unitDatum then
            setStatus(contextError or (label .. ' - ENTREE ANNULEE: JOUEUR LOCAL MODIFIE'))
            return
        end

        local okAlreadySeated, alreadySeated = pcall(function()
            return current.actor.BlamUnit:IsSeatedInParent()
        end)
        if okAlreadySeated and alreadySeated == true then
            setStatus(label .. ' - PILOTE ACTIF')
            return
        end

        runNative(string.format('enter_immediate %d %d %d 0', unitDatum, vehicleDatum, tagDatum),
            function(result, entryError)
                if generation ~= state.entryGeneration then return end
                if entryError or not result or not result:find('^ok enter_immediate') then
                    local afterError, afterErrorContext = playerContext(true)
                    local okSeatedAfterError, seatedAfterError = false, false
                    if afterError and afterError.datum == unitDatum then
                        okSeatedAfterError, seatedAfterError = pcall(function()
                            return afterError.actor.BlamUnit:IsSeatedInParent()
                        end)
                    end
                    if okSeatedAfterError and seatedAfterError == true then
                        setStatus(label .. ' - PILOTE ACTIF')
                    else
                        setStatus(label .. ' - VEHICULE APPARU, ENTREE PILOTE REFUSEE')
                        print(string.format('[HaloSandbox] Entree pilote refusee: %s / %s\n',
                            tostring(result or entryError), tostring(afterErrorContext)))
                    end
                    return
                end

                ExecuteInGameThreadWithDelay(120, function()
                    if generation ~= state.entryGeneration then return end
                    local confirmed, confirmError = playerContext(true)
                    if not confirmed or confirmed.datum ~= unitDatum then
                        setStatus(confirmError or (label .. ' - ENTREE NON CONFIRMEE'))
                        return
                    end
                    local okSeated, seated = pcall(function()
                        return confirmed.actor.BlamUnit:IsSeatedInParent()
                    end)
                    if okSeated and seated == true then
                        setStatus(label .. ' - PILOTE ACTIF')
                    else
                        setStatus(label .. ' - VEHICULE APPARU, ENTREE NON CONFIRMEE')
                    end
                end)
            end)
    end)
end

local function spawnItem(item)
    -- Toute creation native doit partir du listen-server. Un client ne dispose
    -- d'aucun RPC vers ce code : autoriser la commande ici ne creerait qu'un
    -- objet local non autoritaire, ensuite corrige par le serveur.
    local context, contextError = playerContext(true)
    if not context then setStatus(contextError) return end
    if item.kind == 'vehicle' and not item.attachment and not state.preloadReady then
        setStatus('VEHICULES: PREPARATION COOP EN COURS')
        return
    end
    state.spawnGeneration = state.spawnGeneration + 1
    local generation = state.spawnGeneration

    if item.kind == 'vehicle' and item.path:find('tuning_fork', 1, true) then
        setStatus('SPIRIT BLOQUE: COLLISION ET SORTIE INSTABLES')
        return
    end

    resolveTag(item, function(tagDatum)
        if generation ~= state.spawnGeneration or not tagDatum then return end
        local current, currentError = playerContext(true)
        if not current or current.datum ~= context.datum then
            setStatus(currentError or 'JOUEUR LOCAL A CHANGE')
            return
        end
        if item.kind == 'vehicle' and not item.attachment then
            local okSeated, seated = pcall(function() return current.actor.BlamUnit:IsSeatedInParent() end)
            if okSeated and seated then
                setStatus('SORS DU VEHICULE ACTUEL AVANT DE SPAWN')
                return
            end
        end

        runNative('origin ' .. current.datum, function(originResult, originError)
            if generation ~= state.spawnGeneration then return end
            local authoritative, recheckError = playerContext(true)
            if not authoritative or authoritative.datum ~= context.datum then
                setStatus(recheckError or 'AUTORITE OU JOUEUR LOCAL MODIFIE')
                return
            end
            if originError then setStatus('POSITION JOUEUR INTROUVABLE') return end
            local pose = parsePose(originResult)
            if not pose then setStatus('POSITION JOUEUR INVALIDE') return end
            local point, placementError = safePlacement(authoritative, pose, item)
            if not point then setStatus(placementError) return end
            local transform = nil
            if not point.relative then
                transform = string.format('%.6f %.6f %.6f %.6f %.6f %.6f %.6f %.6f %.6f',
                    point.x, point.y, point.z, pose.f.x, pose.f.y, pose.f.z, pose.u.x, pose.u.y, pose.u.z)
            end

            local function doSpawn()
                if generation ~= state.spawnGeneration then return end
                local finalContext, finalError = playerContext(true)
                if not finalContext or finalContext.datum ~= context.datum then
                    setStatus(finalError or 'SPAWN ANNULE: AUTORITE PERDUE')
                    return
                end
                local command
                if item.kind == 'vehicle' and not item.attachment then
                    command = string.format('spawn_enter_relative %d %d 0 %.3f %.3f',
                        finalContext.datum, tagDatum, point.forward, point.up)
                else
                    command = string.format('spawn_at %d %s', tagDatum, transform)
                end
                runNative(command, function(spawnResult, spawnError)
                    if generation ~= state.spawnGeneration then return end
                    if spawnError then
                        setStatus('APPARITION REFUSEE: ' .. tostring(spawnResult or spawnError))
                        return
                    end
                    if item.kind == 'vehicle' and not item.attachment
                        and (not spawnResult or not spawnResult:find('^ok spawn_enter')) then
                        setStatus('APPARITION VEHICULE NON CONFIRMEE PAR LE MOTEUR')
                        return
                    end
                    local spawned = spawnResult and tonumber(spawnResult:match('datum=([%-%d]+)'))
                    if not spawned then
                        setStatus('APPARITION NON CONFIRMEE PAR LE MOTEUR')
                        return
                    end
                    if item.kind == 'vehicle' and not item.attachment then
                        print(string.format('[HaloSandbox] Vehicule cree dans le repere Blam du joueur: %s\n',
                            tostring(spawnResult)))
                    end
                    state.lastSpawn = { datum = spawned, item = item, tag = tagDatum }
                    if item.kind == 'vehicle' and not item.attachment then
                        local isPelican = item.path:find('\\pelican\\pelican', 1, true) ~= nil
                        if isPelican then
                            -- Rend l'instance elle-même accessible dans la
                            -- simulation autoritaire, notamment après que
                            -- l'hôte a quitté le siège pilote.
                            runNative(string.format('enterable %d 1', spawned), function(_, enterableError)
                                if enterableError then
                                    print('[HaloSandbox] Pelican enterable warning: ' .. tostring(enterableError) .. '\n')
                                end
                                ensureVehicleEntry(finalContext.datum, spawned, tagDatum, item.label)
                            end)
                        else
                            ensureVehicleEntry(finalContext.datum, spawned, tagDatum, item.label)
                        end
                    else
                        setStatus(item.kind == 'biped' and (item.label .. ' - CORPS PHYSIQUE SANS IA (ANTI-CRASH)') or (item.label .. ' - APPARU COTE HOTE'))
                    end
                end)
            end

            local isPelican = item.path:find('\\pelican\\pelican', 1, true) ~= nil
            local soloAircraft = isPelican or item.path:find('tuning_fork', 1, true)
            if isPelican and not item.attachment then
                -- Applique le correctif local avant le spawn sur le PC hôte.
                -- Le même correctif est appliqué automatiquement chez le client.
                patchPelicanSeatsLocal(function(patched, patchError)
                    if generation ~= state.spawnGeneration then return end
                    if not patched then
                        setStatus('PELICAN: SIEGE PILOTE INCOMPATIBLE - ' .. tostring(patchError))
                        return
                    end
                    doSpawn()
                end, true)
            elseif soloAircraft and not item.attachment then
                runNative(string.format('seatflags %d 0 00000014 000002A8', tagDatum), function(_, patchError)
                    if generation ~= state.spawnGeneration then return end
                    if patchError then setStatus('SIEGE PILOTE INCOMPATIBLE') return end
                    doSpawn()
                end)
            else
                doSpawn()
            end
        end)
    end)
end

local function heal()
    local context, err = playerContext(true)
    if not context then setStatus(err) return end
    runNative(string.format('health %d 1 1', context.datum), function(result, nativeError)
        setStatus(nativeError and 'SOIN REFUSE' or 'VIE ET BOUCLIER RESTAURES')
    end)
end

local function toggleBottomlessAmmo()
    local context, contextError = playerContext(true)
    if not context then setStatus(contextError) return end
    local enable = not state.bottomless
    runNative(string.format('player_powers %d -1 %d', context.datum,
        enable and 1 or 0), function(result, nativeError)
        if nativeError then
            setStatus(enable and 'MUNITIONS INFINIES: REFUSEES'
                or 'RESTAURATION MUNITIONS: REFUSEE')
            return
        end
        state.bottomless = enable
        state.bottomlessAppliedDatum = enable and context.datum or nil
        setStatus(enable and 'MUNITIONS INFINIES: RESERVE / ENERGIE BLOQUEES'
            or 'MUNITIONS INFINIES: DESACTIVEES')
        print('[HaloSandbox] Garde munitions natif: ' .. tostring(result) .. '\n')
    end)
end

local function toggleInvincibility()
    local context, contextError = playerContext(true)
    if not context then setStatus(contextError) return end
    local enable = not state.deathless
    local effectiveProtection = enable or state.noclip
    runNative(string.format('damageproof %d %d', context.datum, effectiveProtection and 1 or 0),
        function(result, nativeError)
            if nativeError then
                setStatus('INVINCIBILITE: REFUSEE')
                return
            end
            state.deathless = enable
            state.deathlessAppliedDatum = enable and context.datum or nil
            if enable then
                runNative(string.format('health %d 1 1', context.datum), function(_, healError)
                    setStatus(healError and 'INVINCIBILITE ACTIVE - SOIN REFUSE'
                        or 'INVINCIBILITE: ACTIVEE (MOTEUR HALO)')
                end)
            else
                setStatus(state.noclip and 'INVINCIBILITE: NON - NOCLIP PROTEGE ENCORE'
                    or 'INVINCIBILITE: DESACTIVEE')
            end
            print('[HaloSandbox] Drapeau de degats natif: ' .. tostring(result) .. '\n')
        end)
end

local function measureActiveAIDistance(reason)
    local tracked = state.lastActiveAI
    if not tracked or not tracked.datum then
        setStatus('DIAGNOSTIC IA: AUCUNE IA SUIVIE')
        return
    end
    local context, contextError = playerContext(false)
    if not context then
        setStatus(contextError or 'DIAGNOSTIC IA: JOUEUR INTROUVABLE')
        return
    end
    runNative('origin ' .. context.datum, function(playerResult, playerError)
        local playerPose = parsePose(playerResult)
        if playerError or not playerPose then
            setStatus('DIAGNOSTIC IA: POSITION JOUEUR INTROUVABLE')
            return
        end
        runNative('origin ' .. tracked.datum, function(aiResult, aiError)
            local aiPose = parsePose(aiResult)
            if aiError or not aiPose then
                print(string.format('[HaloSandbox][TrueBlamAI] suivi perdu reason=%s unit=%d result=%s\n',
                    tostring(reason or 'manuel'), tracked.datum, tostring(aiResult or aiError)))
                setStatus('DIAGNOSTIC IA: UNITE ABSENTE OU MORTE')
                return
            end
            local dx = aiPose.p.x - playerPose.p.x
            local dy = aiPose.p.y - playerPose.p.y
            local dz = aiPose.p.z - playerPose.p.z
            local distance = math.sqrt(dx * dx + dy * dy + dz * dz)
            print(string.format('[HaloSandbox][TrueBlamAI] suivi distance reason=%s player=%d unit=%d metres=%.3f player_pos=%.3f,%.3f,%.3f ai_pos=%.3f,%.3f,%.3f\n',
                tostring(reason or 'manuel'), context.datum, tracked.datum, distance,
                playerPose.p.x, playerPose.p.y, playerPose.p.z,
                aiPose.p.x, aiPose.p.y, aiPose.p.z))
            setStatus(string.format('DIAGNOSTIC IA: DISTANCE %.1f M', distance))
        end)
    end)
end

local function revertCheckpoint()
    local context, contextError = playerContext(false)
    if not context then setStatus(contextError) return end
    -- La commande historique game_revert n'est pas exposee par cette build.
    -- Ne pas pretendre qu'elle a fonctionne: le menu pause appelle le vrai
    -- chemin campagne et reste le test fiable de persistance du checkpoint.
    setStatus('CHECKPOINT: ECHAP > RECHARGER LE POINT DE CONTROLE')
    print('[HaloSandbox] F7: game_revert indisponible; utiliser le menu pause du jeu.\n')
end

local function phaseForward()
    local context, contextError = playerContext(true)
    if not context then setStatus(contextError) return end
    local okSeated, seated = pcall(function() return context.actor.BlamUnit:IsSeatedInParent() end)
    if okSeated and seated then setStatus('TRAVERSEE: SORS DU VEHICULE') return end

    local function executePhase()
        runNative(string.format('phase_forward %d 16 1', context.datum), function(result, phaseError)
            if phaseError then
                setStatus('TRAVERSEE DE LIMITE REFUSEE')
                return
            end
            setStatus('LIMITE TRAVERSEE: 16 M - INVINCIBILITE ACTIVE')
            print('[HaloSandbox] Traversee atomique: ' .. tostring(result) .. '\n')
            if state.lastActiveAI then
                ExecuteInGameThreadWithDelay(1200, function()
                    measureActiveAIDistance('phase_forward_1s')
                end)
                ExecuteInGameThreadWithDelay(5000, function()
                    measureActiveAIDistance('phase_forward_5s')
                end)
            end
        end)
    end

    -- En dehors du BSP, les volumes de mort peuvent s'appliquer au tick qui
    -- suit le teleport. La traversee active donc le vrai drapeau damageproof
    -- et le conserve; le joueur peut le couper ensuite dans cette page.
    if state.deathless and state.deathlessAppliedDatum == context.datum then
        executePhase()
        return
    end
    runNative(string.format('damageproof %d 1', context.datum), function(_, proofError)
        if proofError then setStatus('TRAVERSEE: PROTECTION REFUSEE') return end
        state.deathless = true
        state.deathlessAppliedDatum = context.datum
        executePhase()
    end)
end

local function recoverNoclipAnchor()
    local context, contextError = playerContext(true)
    if not context then setStatus(contextError) return end
    runNative(string.format('noclip_recover %d %d', context.datum,
        state.deathless and 1 or 0), function(result, recoveryError)
        if recoveryError then
            setStatus('RETOUR SUR: AUCUN POINT DISPONIBLE POUR CETTE MISSION')
            print('[HaloSandbox] Retour noclip refuse: ' .. tostring(result or recoveryError) .. '\n')
            return
        end
        state.noclip = false
        state.noclipAppliedDatum = nil
        setStatus('RETOUR SUR EFFECTUE - LIMITES ET CONTROLES RESTAURES')
        print('[HaloSandbox] Retour noclip: ' .. tostring(result) .. '\n')
    end)
end

local function toggleNoclip()
    local context, contextError = playerContext(true)
    if not context then setStatus(contextError) return end
    local enable = not state.noclip
    runNative(string.format('noclip %d %d %d', context.datum,
        enable and 1 or 0, state.deathless and 1 or 0), function(result, noclipError)
        if noclipError then
            setStatus(enable and 'NOCLIP REFUSE: ETAT DE MISSION INDISPONIBLE'
                or 'RESTAURATION NOCLIP REFUSEE')
            print('[HaloSandbox] Noclip refuse: ' .. tostring(result or noclipError) .. '\n')
            return
        end
        state.noclip = enable
        state.noclipAppliedDatum = enable and context.datum or nil
        state.noclipBridgeErrorLogged = false
        if enable then
            setStatus('NOCLIP ZONES: ACTIF A PIED ET EN VEHICULE - CONTROLES NORMAUX')
        else
            setStatus('NOCLIP DESACTIVE - F7 POUR REVENIR SI TU ES BLOQUE HORS MAP')
        end
        print('[HaloSandbox] Noclip natif: ' .. tostring(result) .. '\n')
    end)
end

-- Diagnostic IA en lecture seule. Les anciennes sondes scenario ont ete
-- retirees car leur retour console ne prouvait pas une execution HaloScript.
local AI_AUDIT_FILE = modRoot .. '\\AI_REFLECTION_AUDIT.txt'

local function auditObjectName(object)
    if not valid(object) then return '<invalide>' end
    local ok, value = pcall(function() return object:GetFullName() end)
    if ok then return tostring(value) end
    return tostring(object)
end

local function auditContainsKeyword(value)
    local lower = tostring(value or ''):lower()
    local keywords = {
        'blam', 'haloscript', 'script', 'scenario', 'console', 'command',
        'execute', 'spawn', 'place', 'encounter', 'squad', 'brain', 'behavior',
    }
    for _, keyword in ipairs(keywords) do
        if lower:find(keyword, 1, true) then return true end
    end
    return lower:find('ai_', 1, true) ~= nil
        or lower:find('_ai', 1, true) ~= nil
        or lower:find('.ai', 1, true) ~= nil
end

-- Audit strictement en lecture seule. Aucun UFunction trouve n'est appele.
-- Le but est d'identifier le vrai pont Blam/HaloScript charge par cette build
-- avant d'implementer la moindre creation d'IA active.
local function generateAiReflectionAudit()
    local context = playerContext(false)
    local world = getLocalWorld()
    local gameInstance = UEHelpers.GetGameInstance()
    local lines = {
        'HALO SANDBOX - AUDIT IA / SCRIPT EN LECTURE SEULE',
        'Version mod: ' .. VERSION,
        'Date locale: ' .. os.date('%Y-%m-%d %H:%M:%S'),
        'IMPORTANT: aucune fonction IA ou script n a ete executee.',
        '',
    }

    local candidates = {}
    local function addCandidate(label, object)
        if valid(object) then candidates[#candidates + 1] = { label = label, object = object } end
    end
    addCandidate('PlayerController local', context and context.controller or getLocalPlayerController())
    addCandidate('Pawn local', context and context.pawn or nil)
    addCandidate('Blam actor local', context and context.actor or nil)
    addCandidate('World local', world)
    addCandidate('GameInstance', gameInstance)

    local candidateNames = {
        'BlamEngineGameInstanceSubsystem',
        'BlamEngineAudioGameSubsystem',
        'BlamObjectActor',
        'BlamUnitActor',
        'BlamBipedActor',
        'BlamNetworkGameStateComponent',
        'BP_MJOLNIRGameMode_C',
        'BP_MJOLNIRGameState_C',
    }
    for _, name in ipairs(candidateNames) do
        local ok, object = pcall(FindFirstOf, name)
        if ok then addCandidate(name, object) end
    end

    local seenClasses = {}
    local function inspectClass(label, classObject)
        local depth = 0
        while valid(classObject) and depth < 10 do
            depth = depth + 1
            local className = auditObjectName(classObject)
            if seenClasses[className] then break end
            seenClasses[className] = true
            lines[#lines + 1] = 'CLASSE: ' .. className .. ' (source: ' .. label .. ')'

            local functionCount = 0
            local okFunctions, functionError = pcall(function()
                classObject:ForEachFunction(function(fn)
                    if functionCount >= 250 then return end
                    local fullName = auditObjectName(fn)
                    if auditContainsKeyword(fullName) then
                        functionCount = functionCount + 1
                        lines[#lines + 1] = '  FONCTION: ' .. fullName
                    end
                end)
            end)
            if not okFunctions then
                lines[#lines + 1] = '  ERREUR ENUMERATION: ' .. tostring(functionError)
            elseif functionCount == 0 then
                lines[#lines + 1] = '  AUCUNE FONCTION CIBLE TROUVEE'
            end

            local okSuper, superClass = pcall(function() return classObject:GetSuperStruct() end)
            if not okSuper or not valid(superClass) then break end
            classObject = superClass
        end
    end

    for _, candidate in ipairs(candidates) do
        lines[#lines + 1] = ''
        lines[#lines + 1] = 'OBJET: ' .. candidate.label .. ' = ' .. auditObjectName(candidate.object)
        local okClass, classObject = pcall(function() return candidate.object:GetClass() end)
        if okClass and valid(classObject) then inspectClass(candidate.label, classObject) end
    end

    lines[#lines + 1] = ''
    lines[#lines + 1] = 'FIN AUDIT. Ce fichier doit etre analyse avant toute nouvelle tentative IA.'
    local ok, err = writeFile(AI_AUDIT_FILE, table.concat(lines, '\r\n') .. '\r\n')
    if ok then
        print('[HaloSandbox] Audit IA ecrit: ' .. AI_AUDIT_FILE .. '\n')
        setStatus('AUDIT IA CREE - UTILISE COLLECTER-DIAGNOSTIC-IA.CMD')
    else
        print('[HaloSandbox] Audit IA impossible: ' .. tostring(err) .. '\n')
        setStatus('AUDIT IA IMPOSSIBLE: ' .. tostring(err))
    end
end

-- IA ACTIVE PAR LE MOTEUR BLAM NATIF
-- La console Unreal n'expose pas les commandes HaloScript ai_*. Ce pont
-- compagnon resout une escouade precise dans le scenario Blam charge,
-- construit la reference AI officielle, puis appelle ai_place sur le vrai
-- game_MAIN_THREAD de la build exacte 1.111.2544.0. Chaque profil ci-dessous
-- a ete extrait du scenario original correspondant; aucun biped Unreal inerte
-- n'est utilise pour ces deux boutons.
local AI_SCENARIO_LOG = modRoot .. '\\AI_SCENARIO_RUNTIME.log'

local SCENARIO_AI_PROFILES = {
    A15 = { ALLY = { label = 'HUMAN REINFORCEMENT', reference = 'squad', name = 'sq_airlock_1_im', team = 2, fallback = 'sq_airlock_1_anti_re' }, ENEMY = { label = 'COVENANT ENEMY', reference = 'squad', name = 'sq_shoot_anti', team = -1 } },
    A30 = { ALLY = { label = 'ALLIED MARINE', reference = 'squad', name = 'first_marine_fitzgerald', team = 2, fallback = 'plank_elite' }, ENEMY = { label = 'ENEMY ELITE', reference = 'squad', name = 'plank_elite', team = -1 } },
    A50 = { ALLY = { label = 'ALLIED MARINE', reference = 'squad', name = 'area5_marines', team = 2, fallback = 'area4_grunts_i' }, ENEMY = { label = 'ENEMY GRUNT', reference = 'squad', name = 'area4_grunts_i', team = -1 } },
    B30 = { ALLY = { label = 'ALLIED MARINE', reference = 'squad', name = 'right_marine_beach_lz', team = 2, fallback = 'back_floor_elite' }, ENEMY = { label = 'ENEMY ELITE', reference = 'squad', name = 'back_floor_elite', team = -1 } },
    B40 = { ALLY = { label = 'ALLIED MARINE', reference = 'squad', name = 'ext_b_marines_s', team = 2, fallback = 'new_elite_commander' }, ENEMY = { label = 'ENEMY GRUNT', reference = 'squad', name = 'control_cov_grunts_g', team = -1 } },
    C10 = { ALLY = { label = 'ALLIED MARINE', reference = 'squad', name = 'sq_bay_c_marines', team = 2, fallback = 'sq_tree_jackal_legendary2' }, ENEMY = { label = 'ENEMY FLOOD', reference = 'squad', name = 'sq_lift_b_infection', team = -1 } },
    C20 = { ALLY = { label = 'LOCAL ALLY', reference = 'squad', name = 'sq_marine_2_10', team = 2, fallback = 'sq_flood_marine1' }, ENEMY = { label = 'ENEMY FLOOD', reference = 'squad', name = 'sq_infectors1_6', team = -1 } },
    C45 = { ALLY = { label = 'LOCAL ALLY', reference = 'squad', name = 'sq_c3_cov_base_emppa', team = 2, fallback = 'sq_c3_cov_base_empf' }, ENEMY = { label = 'COVENANT ENEMY', reference = 'squad', name = 'sq_c3_cov_base_empf', team = -1 } },
    D20 = { ALLY = { label = 'LOCAL ALLY', reference = 'squad', name = 'sq_enc3_0_flood_shooters', team = 2, fallback = 'sq_enc3_2_flood_sniper' }, ENEMY = { label = 'ENEMY FLOOD', reference = 'squad', name = 'sq_enc3_2_flood_sniper', team = -1 } },
    D40 = { ALLY = { label = 'LOCAL ALLY', reference = 'squad', name = 'enc6_0_combats', team = 2, fallback = 'enc6_2_elite_6' }, ENEMY = { label = 'ENEMY FLOOD', reference = 'squad', name = 'sq_enc2_1_flood_i', team = -1 } },
    E10 = { ALLY = { label = 'ALLIED JOHNSON', reference = 'squad', name = 'sq_johnson_gfs', team = 2, fallback = 'sq_dock_intro_grunt_b' }, ENEMY = { label = 'ENEMY GRUNT', reference = 'squad', name = 'sq_dock_intro_grunt_b', team = -1 } },
    E20 = { ALLY = { label = 'ALLIED MARINE', reference = 'squad', name = 'sq_brute_bowl_marine', team = 2, fallback = 'sq_post_arena_gruntmajor_reins' }, ENEMY = { label = 'ENEMY GRUNT', reference = 'squad', name = 'sq_post_arena_gruntmajor_reins', team = -1 } },
    E30 = { ALLY = { label = 'ALLIED MARINE', reference = 'squad', name = 'sq_space2_marines', team = 2, fallback = 'sq_e32_w1_elites' }, ENEMY = { label = 'ENEMY HUNTER', reference = 'squad', name = 'sq_e25_hunter0', team = -1 } },
}

local SCENARIO_BY_COUNTS = {
    ['38:120'] = 'A15', ['41:133'] = 'A30', ['118:360'] = 'A50',
    ['42:136'] = 'B30', ['129:284'] = 'B40', ['80:215'] = 'C10',
    ['79:204'] = 'C20', ['109:279'] = 'C45', ['88:242'] = 'D20',
    ['61:244'] = 'D40', ['130:238'] = 'E10', ['122:186'] = 'E20',
    ['125:208'] = 'E30',
}

local function logScenarioAI(value)
    local line = string.format('[%s] %s\r\n', os.date('%Y-%m-%d %H:%M:%S'), tostring(value or ''))
    appendFile(AI_SCENARIO_LOG, line)
    print('[HaloSandbox][TrueBlamAI] ' .. tostring(value or '') .. '\n')
end

local function objectFullName(object)
    if not valid(object) then return '' end
    local okName, fullName = pcall(function() return object:GetFullName() end)
    return okName and tostring(fullName or '') or ''
end

local function missionCodeFromText(value)
    local upper = tostring(value or ''):upper():gsub('\\', '/')
    for _, code in ipairs({ 'A15', 'A30', 'A50', 'B30', 'B40', 'C10', 'C20', 'C45', 'D20', 'D40', 'E10', 'E20', 'E30' }) do
        if upper:find('/' .. code .. '/', 1, true)
            or upper:find('_' .. code .. '_', 1, true)
            or upper:find(code .. '.', 1, true)
            or upper:find(code, 1, true) then
            return code
        end
    end
    return nil
end

local function currentMissionCode()
    local candidates = { getLocalWorld(), getLocalPlayerController() }
    local controller = getLocalPlayerController()
    if valid(controller) then candidates[#candidates + 1] = controller.Pawn end
    for _, className in ipairs({ 'BlamScenarioActor', 'BP_BlamScenarioActor_C', 'BlamWorldSettings' }) do
        local okObjects, objects = pcall(FindAllOf, className)
        if okObjects and objects then
            for index, object in ipairs(objects) do
                candidates[#candidates + 1] = object
                if index >= 8 then break end
            end
        end
    end
    for _, candidate in ipairs(candidates) do
        local code = missionCodeFromText(objectFullName(candidate))
        if code then return code end
    end
    return nil
end

local function runAiNative(command, callback)
    command = tostring(command or '')
    local nativeCommand
    if command == 'probe' then
        nativeCommand = 'ai_probe'
    else
        local reference, name, count, team = command:match('^place_([%a]+) ([%w_]+) (%d+) (-?%d+)$')
        count, team = tonumber(count), tonumber(team)
        if (reference ~= 'group' and reference ~= 'squad') or not name or #name > 31 or not count or count < 1 or count > 16
            or not team or team < -1 or team > 15 then
            if callback then callback(nil, 'invalid_ai_reference_count_or_team') end
            setStatus('COMMANDE IA NATIVE BLOQUEE')
            return false
        end
        nativeCommand = 'ai_place_' .. reference .. ' ' .. name .. ' ' .. count .. ' ' .. team
    end
    return runNative(nativeCommand, function(result, err)
        logScenarioAI('native command=' .. nativeCommand .. ' result=' .. tostring(result or err))
        if callback then callback(result, err) end
    end)
end

local function missionFromProbe(result)
    local groups = tostring(result or ''):match('groups=(%d+)')
    local squads = tostring(result or ''):match('squads=(%d+)')
    if not groups or not squads then return nil end
    return SCENARIO_BY_COUNTS[groups .. ':' .. squads]
end

local function moveScenarioAIToHost(context, datum, profile, slot, total, callback)
    runNative('origin ' .. context.datum, function(originResult, originError)
        local pose = parsePose(originResult)
        if originError or not pose then
            setStatus(profile.label .. ': ACTIVE DANS SA ZONE SCENARIO')
            logScenarioAI('acteur cree datum=' .. tostring(datum) .. ' origine hote indisponible')
            if callback then callback(false) end
            return
        end
        slot, total = tonumber(slot) or 1, tonumber(total) or 1
        local rightX, rightY = -pose.f.y, pose.f.x
        local row = math.floor((slot - 1) / 5)
        local column = ((slot - 1) % 5) - math.min(2, (total - 1) * 0.5)
        local forwardDistance = 5.0 + row * 2.0
        local rightDistance = column * 1.55
        local x = pose.p.x + pose.f.x * forwardDistance + rightX * rightDistance
        local y = pose.p.y + pose.f.y * forwardDistance + rightY * rightDistance
        local z = pose.p.z + pose.f.z * forwardDistance + 0.35
        runNative(string.format('teleport %d %.6f %.6f %.6f', datum, x, y, z), function(_, teleportError)
            if teleportError then
                setStatus(profile.label .. ': ACTIVE DANS SA ZONE SCENARIO')
                logScenarioAI('teleport refuse datum=' .. tostring(datum) .. ' err=' .. tostring(teleportError))
                if callback then callback(false) end
                return
            end
            runNative(string.format('velocity %d 0 0 0', datum), function(_, velocityError)
                state.lastScenarioAI = { datum = datum, profile = profile, slot = slot, total = total }
                logScenarioAI('succes datum=' .. tostring(datum) .. ' slot=' .. tostring(slot) .. '/' .. tostring(total)
                    .. ' velocity=' .. tostring(velocityError or 'ok'))
                if callback then callback(true) end
            end)
        end)
    end)
end

local function moveScenarioAIList(context, datums, profile, requested)
    local moved = 0
    local function moveNext(index)
        if index > #datums then
            setStatus(string.format('%s: %d/%d IA ACTIVES PRES DE TOI', profile.label, moved, requested))
            logScenarioAI(string.format('placement termine profile=%s moved=%d created=%d requested=%d',
                profile.name, moved, #datums, requested))
            return
        end
        moveScenarioAIToHost(context, datums[index], profile, index, #datums, function(ok)
            if ok then moved = moved + 1 end
            moveNext(index + 1)
        end)
    end
    moveNext(1)
end

local function spawnScenarioAI(kind, requested)
    requested = math.max(1, math.min(tonumber(requested) or 1, 10))
    local mission = currentMissionCode()
    local context, contextError = playerContext(true)
    if not context then setStatus(contextError) return end
    state.aiScenarioGeneration = state.aiScenarioGeneration + 1
    local generation = state.aiScenarioGeneration
    setStatus('VERIFICATION DU PONT IA NATIF...')

    runAiNative('probe', function(probeResult, probeError)
        if generation ~= state.aiScenarioGeneration then return end
        if probeError then
            setStatus('PONT IA REFUSE: ' .. tostring(probeResult or probeError))
            return
        end
        mission = mission or missionFromProbe(probeResult)
        local missionProfiles = mission and SCENARIO_AI_PROFILES[mission] or nil
        local profile = missionProfiles and missionProfiles[kind] or nil
        if not profile then
            setStatus('MISSION IA NON IDENTIFIEE - ACTION ANNULEE')
            logScenarioAI('mission inconnue probe=' .. tostring(probeResult))
            return
        end
        local authoritative, authorityError = playerContext(true)
        if not authoritative or authoritative.datum ~= context.datum then
            setStatus(authorityError or 'IA ANNULEE: AUTORITE PERDUE')
            return
        end

        logScenarioAI(string.format('debut mission=%s kind=%s reference=%s name=%s fallback=%s team=%d requested=%d probe=%s',
            tostring(mission), tostring(kind), profile.reference, profile.name, tostring(profile.fallback), profile.team, requested, tostring(probeResult)))
        setStatus('CREATION BLAM: ' .. profile.label .. '...')

        local function finishPlacement(placeResult)
            if generation ~= state.aiScenarioGeneration then return end
            local datumText = tostring(placeResult or ''):match('datums=([^%s]+)') or ''
            local datums = {}
            for rawDatum in datumText:gmatch('-?%d+') do
                local datum = tonumber(rawDatum)
                if datum and datum ~= -1 then datums[#datums + 1] = datum end
            end
            if #datums == 0 then
                setStatus('IA CREEE MAIS INDEX EXACT ABSENT - ACTION ANNULEE')
                logScenarioAI('resultat sans datum exact=' .. tostring(placeResult))
                return
            end
            local host, authorityError = playerContext(true)
            if not host or host.datum ~= context.datum then
                setStatus(authorityError or 'IA ANNULEE: AUTORITE PERDUE')
                return
            end
            setStatus(string.format('%d IA BLAM CREEE(S) - POSITIONNEMENT EXACT...', #datums))
            logScenarioAI('ai_place datums exacts=' .. table.concat(datums, ',') .. ' result=' .. tostring(placeResult))
            moveScenarioAIList(host, datums, profile, requested)
        end

        local function attemptPlacement(name, fallbackAttempt)
            runAiNative(string.format('place_%s %s %d %d', profile.reference, name, requested, profile.team), function(placeResult, placeError)
                if generation ~= state.aiScenarioGeneration then return end
                if placeError then
                    if not fallbackAttempt and profile.fallback and profile.fallback ~= name then
                        logScenarioAI('profil principal refuse=' .. tostring(placeResult or placeError)
                            .. ' fallback=' .. tostring(profile.fallback))
                        setStatus('PROFIL PRINCIPAL REFUSE - SECOURS ACTIF...')
                        attemptPlacement(profile.fallback, true)
                        return
                    end
                    setStatus('CREATION IA REFUSEE: ' .. tostring(placeResult or placeError))
                    return
                end
                finishPlacement(placeResult)
            end)
        end
        attemptPlacement(profile.name, false)
    end)
end

local function findQuick(pathNeedle)
    for _, item in ipairs(catalog) do
        if item.kind == 'vehicle' and not item.attachment and item.path:find(pathNeedle, 1, true) then return item end
    end
end

local function findCatalogItem(pathNeedle)
    for _, item in ipairs(catalog) do
        if item.path == pathNeedle then return item end
    end
end

local AI_BIPED_CATEGORIES = {
    ALLY = {
        { label = 'MARINE', path = 'objects\\characters\\marine\\marine', characterPath = 'objects\\characters\\marine\\ai\\trooper', team = 2, loadout = 1 },
        { label = 'FEMALE MARINE', path = 'objects\\characters\\marine_female\\marine_female', characterPath = 'objects\\characters\\marine_female\\ai\\trooper_female', team = 2, loadout = 1 },
        { label = 'ALLIED ELITE', path = 'objects\\characters\\elite\\elite', characterPath = 'objects\\characters\\elite\\ai\\elite', team = 2, loadout = 2 },
        { label = 'ALLIED GRUNT', path = 'objects\\characters\\grunt\\grunt', characterPath = 'objects\\characters\\grunt\\ai\\grunt', team = 2, loadout = 3 },
        { label = 'ALLIED JACKAL', path = 'objects\\characters\\jackal\\jackal', characterPath = 'objects\\characters\\jackal\\ai\\jackal', team = 2, loadout = 4 },
        { label = 'ALLIED HUNTER', path = 'objects\\characters\\hunter\\hunter', characterPath = 'objects\\characters\\hunter\\ai\\hunter', team = 2, loadout = 0 },
        { label = 'ALLIED SENTINEL', path = 'objects\\characters\\sentinel_aggressor\\sentinel_aggressor', characterPath = 'objects\\characters\\sentinel_aggressor\\ai\\sentinel_aggressor', team = 2, loadout = 0 },
        { label = 'ALLIED HUMAN FLOOD', path = 'objects\\characters\\floodcombat_human\\floodcombat_human', characterPath = 'objects\\characters\\floodcombat_human\\ai\\flood_combat_human', team = 2, loadout = 5 },
        { label = 'ALLIED ELITE FLOOD', path = 'objects\\characters\\floodcombat_elite\\floodcombat_elite', characterPath = 'objects\\characters\\floodcombat_elite\\ai\\floodcombat_elite', team = 2, loadout = 5 },
        { label = 'ALLIED CARRIER FLOOD', path = 'objects\\characters\\floodcarrier\\floodcarrier', characterPath = 'objects\\characters\\floodcarrier\\ai\\flood_carrier', team = 2, loadout = 0 },
        { label = 'ALLIED FLOOD TANK', path = 'objects\\characters\\flood_tank\\flood_tank', characterPath = 'objects\\characters\\flood_tank\\ai\\flood_pureform_tank', team = 2, loadout = 0 },
    },
    ENEMY = {
        { label = 'GRUNT', path = 'objects\\characters\\grunt\\grunt', characterPath = 'objects\\characters\\grunt\\ai\\grunt', team = 3, loadout = 3 },
        { label = 'JACKAL', path = 'objects\\characters\\jackal\\jackal', characterPath = 'objects\\characters\\jackal\\ai\\jackal', team = 3, loadout = 4 },
        { label = 'ELITE', path = 'objects\\characters\\elite\\elite', characterPath = 'objects\\characters\\elite\\ai\\elite', team = 3, loadout = 2 },
        { label = 'HUNTER', path = 'objects\\characters\\hunter\\hunter', characterPath = 'objects\\characters\\hunter\\ai\\hunter', team = 3, loadout = 0 },
        { label = 'HUMAN FLOOD', path = 'objects\\characters\\floodcombat_human\\floodcombat_human', characterPath = 'objects\\characters\\floodcombat_human\\ai\\flood_combat_human', team = 8, loadout = 5 },
        { label = 'FLOOD ELITE', path = 'objects\\characters\\floodcombat_elite\\floodcombat_elite', characterPath = 'objects\\characters\\floodcombat_elite\\ai\\floodcombat_elite', team = 8, loadout = 5 },
        { label = 'CARRIER FLOOD', path = 'objects\\characters\\floodcarrier\\floodcarrier', characterPath = 'objects\\characters\\floodcarrier\\ai\\flood_carrier', team = 8, loadout = 0 },
        { label = 'FLOOD TANK', path = 'objects\\characters\\flood_tank\\flood_tank', characterPath = 'objects\\characters\\flood_tank\\ai\\flood_pureform_tank', team = 8, loadout = 0 },
        { label = 'SENTINEL', path = 'objects\\characters\\sentinel_aggressor\\sentinel_aggressor', characterPath = 'objects\\characters\\sentinel_aggressor\\ai\\sentinel_aggressor', team = 9, loadout = 0 },
    },
}

-- Cree une unite active sans modifier le scenario. Halo cree l'acteur et son
-- biped actif; la couche native valide ce couple, supprime le biped modele et
-- rattache les allies a la fireteam du joueur.
local mixedRanks = {
    normal = {
        ELITE = {"objects\\characters\\elite\\ai\\elite","objects\\characters\\elite\\ai\\elite","objects\\characters\\elite\\ai\\elite","objects\\characters\\elite\\ai\\elite","objects\\characters\\elite\\ai\\elite","objects\\characters\\elite\\ai\\elite","objects\\characters\\elite\\ai\\elite_officer","objects\\characters\\elite\\ai\\elite_officer","objects\\characters\\elite\\ai\\elite_ultra","objects\\characters\\elite\\ai\\elite_specops"},
        GRUNT = {"objects\\characters\\grunt\\ai\\grunt","objects\\characters\\grunt\\ai\\grunt","objects\\characters\\grunt\\ai\\grunt","objects\\characters\\grunt\\ai\\grunt","objects\\characters\\grunt\\ai\\grunt","objects\\characters\\grunt\\ai\\grunt","objects\\characters\\grunt\\ai\\grunt_major","objects\\characters\\grunt\\ai\\grunt_major","objects\\characters\\grunt\\ai\\grunt_major","objects\\characters\\grunt\\ai\\grunt_heavy"},
        JACKAL = {"objects\\characters\\jackal\\ai\\jackal","objects\\characters\\jackal\\ai\\jackal","objects\\characters\\jackal\\ai\\jackal","objects\\characters\\jackal\\ai\\jackal","objects\\characters\\jackal\\ai\\jackal","objects\\characters\\jackal\\ai\\jackal","objects\\characters\\jackal\\ai\\jackal","objects\\characters\\jackal\\ai\\jackal_major","objects\\characters\\jackal\\ai\\jackal_major","objects\\characters\\jackal\\ai\\jackal_major"},
    },
    E10 = {
        ELITE = {"objects\\characters\\elite\\ai\\elite_sacristan_minor","objects\\characters\\elite\\ai\\elite_sacristan_minor","objects\\characters\\elite\\ai\\elite_sacristan_minor","objects\\characters\\elite\\ai\\elite_sacristan_minor","objects\\characters\\elite\\ai\\elite_sacristan_minor","objects\\characters\\elite\\ai\\elite_sacristan_minor","objects\\characters\\elite\\ai\\elite_sacristan_major","objects\\characters\\elite\\ai\\elite_sacristan_major","objects\\characters\\elite\\ai\\elite_sacristan_specops","objects\\characters\\elite\\ai\\elite_sacristan_zealot"},
        GRUNT = {"objects\\characters\\grunt\\ai\\grunt_sacristan_minor","objects\\characters\\grunt\\ai\\grunt_sacristan_minor","objects\\characters\\grunt\\ai\\grunt_sacristan_minor","objects\\characters\\grunt\\ai\\grunt_sacristan_minor","objects\\characters\\grunt\\ai\\grunt_sacristan_minor","objects\\characters\\grunt\\ai\\grunt_sacristan_minor","objects\\characters\\grunt\\ai\\grunt_sacristan_major","objects\\characters\\grunt\\ai\\grunt_sacristan_major","objects\\characters\\grunt\\ai\\grunt_sacristan_major","objects\\characters\\grunt\\ai\\grunt_sacristan_specops"},
        JACKAL = {"objects\\characters\\jackal\\ai\\jackal_sacristan_minor","objects\\characters\\jackal\\ai\\jackal_sacristan_minor","objects\\characters\\jackal\\ai\\jackal_sacristan_minor","objects\\characters\\jackal\\ai\\jackal_sacristan_minor","objects\\characters\\jackal\\ai\\jackal_sacristan_minor","objects\\characters\\jackal\\ai\\jackal_sacristan_minor","objects\\characters\\jackal\\ai\\jackal_sacristan_minor","objects\\characters\\jackal\\ai\\jackal_sacristan_major","objects\\characters\\jackal\\ai\\jackal_sacristan_major","objects\\characters\\jackal\\ai\\jackal_sacristan_major"},
    },
}

local function planCharacters(label, path, mission, count, rankLocked)
    local group = mixedRanks[mission] or mixedRanks.normal
    local pool = not rankLocked and group[label]
    if not pool then
        local result = {}
        for i=1,count do result[i]=path end
        return result
    end
    local deck = {}
    for i,v in ipairs(pool) do deck[i]=v end
    for i=#deck,2,-1 do
        local j=math.random(i);deck[i],deck[j]=deck[j],deck[i]
    end
    local result={}
    for i=1,count do result[i]=deck[(i-1)%#deck+1] end
    return result
end

local function resolveCharacterPlan(paths, callback)
    local datums, cache = {}, {}
    local function nextPath(i)
        if i>#paths then callback(datums) return end
        local path=paths[i]
        if cache[path] then datums[i]=cache[path];nextPath(i+1);return end
        local name=path:match('([^\\]+)$')
        resolveTag({label=name,path=path,group='63686172',
            asset='/Game/Tags/' .. path:gsub('\\','/') .. '-character.' .. name .. '-character'}, function(datum)
            if not datum then callback(nil) return end
            cache[path]=datum;datums[i]=datum;nextPath(i+1)
        end)
    end
    nextPath(1)
end

local function spawnActiveSquad(label, pathNeedle, characterPath, requested, team, loadoutProfile, diagnosticOverride, rankLocked, escortRequest, marineMode)
    if not AI_DIRECT_SPAWN_ENABLED and diagnosticOverride ~= true then
        setStatus('IA TOUTES MISSIONS: CREATION DIRECTE BLOQUEE PAR SECURITE')
        print('[HaloSandbox] Spawn IA direct refuse par le verrou de securite; attente du chemin squad runtime.\n')
        return
    end
    marineMode = (not escortRequest and team == 2 and (marineMode == 1 or marineMode == 2)) and marineMode or 0
    local mission = currentMissionCode()
    testHistory('spawn selection=' .. label .. ' mission=' .. tostring(mission) .. ' character=' .. tostring(characterPath))
    requested = math.max(1, math.min(tonumber(requested) or 1, 10))
    local context, contextError = playerContext(true)
    if not context then setStatus(contextError) return end
    local item = findCatalogItem(pathNeedle)
    if not item then setStatus('TYPE D UNITE INTROUVABLE') return end
    if state.spawnPackGeneration == state.lifecycleGeneration then
        setStatus('FINISHING CURRENT GROUP - PLEASE WAIT') return
    end
    local packGeneration=state.lifecycleGeneration
    state.spawnPackGeneration=packGeneration
    local function finishPack()
        if state.spawnPackGeneration==packGeneration then state.spawnPackGeneration=nil end
        if escortRequest and escortRequest.done then escortRequest.done();escortRequest.done=nil end
    end
    local characterPlan=planCharacters(label,characterPath,mission,requested,rankLocked)
    resolveTag(item, function(tagDatum)
        if not tagDatum then finishPack() return end
        resolveCharacterPlan(characterPlan, function(actorDatums)
        if not actorDatums then finishPack() return end
        local current, currentError = playerContext(true)
        if not current or current.datum ~= context.datum then finishPack() setStatus(currentError or 'JOUEUR A CHANGE') return end
        runNative('origin ' .. current.datum, function(originResult, originError)
            local pose = parsePose(originResult)
            if originError or not pose then finishPack() setStatus('POSITION JOUEUR INTROUVABLE') return end
            local allOffsets
            if team == 2 then
                -- Les renforts apparaissent proches du groupe afin d'entrer
                -- immédiatement dans sa bulle de perception et de le défendre.
                allOffsets = {
                    {0, 180}, {-70, 230}, {70, 230}, {-140, 280}, {140, 280},
                    {0, 330}, {-70, 380}, {70, 380}, {-140, 430}, {140, 430},
                }
            else
                -- Les ennemis sont placés face au groupe, assez loin pour ne
                -- pas se superposer aux alliés mais toujours à portée de combat.
                allOffsets = {
                    {0, 520}, {-80, 570}, {80, 570}, {-160, 620}, {160, 620},
                    {0, 670}, {-80, 720}, {80, 720}, {-160, 770}, {160, 770},
                }
            end
            -- Le BSP historique de Halo n'est pas une collision Unreal fiable.
            -- On place donc chaque acteur dans le repere Blam natif du joueur.
            local points = {}
            -- La visee peut etre inclinee. Pour un spawn fiable dans les BSP
            -- etroits, la formation reste sur le plan XY et chaque IA est
            -- creee debout selon l'axe Z du monde.
            local horizontalLength = math.sqrt(pose.f.x * pose.f.x + pose.f.y * pose.f.y)
            local spawnForward
            if horizontalLength > 0.001 then
                spawnForward = {
                    x = pose.f.x / horizontalLength,
                    y = pose.f.y / horizontalLength,
                    z = 0.0,
                }
            else
                spawnForward = { x = 1.0, y = 0.0, z = 0.0 }
            end
            local spawnUp = { x = 0.0, y = 0.0, z = 1.0 }
            local rightNative = { x = -spawnForward.y, y = spawnForward.x, z = 0.0 }
            for offsetIndex = 1, requested do
                local offset = allOffsets[offsetIndex]
                local lateral = offset[1] / 100.0 * ((label == "HUNTER" or label == "FLOOD TANK" or label == "FLOOD CARRIER") and 2.5 or 1)
                local forward = offset[2] / 100.0 + ((label == "HUNTER" or label == "FLOOD TANK" or label == "FLOOD CARRIER") and (offsetIndex-1)*0.8 or 0)
                local lift = team == 9 and 2.5 or 0.45
                points[#points + 1] = {
                    x = pose.p.x + spawnForward.x * forward + rightNative.x * lateral,
                    y = pose.p.y + spawnForward.y * forward + rightNative.y * lateral,
                    z = pose.p.z + lift,
                }
            end
            local index, created = 1, 0
            local function createNext()
                if escortRequest and not escortRequest.allowed() then finishPack() return end
                local authoritative, authorityError = playerContext(true)
                if packGeneration ~= state.lifecycleGeneration or not authoritative or authoritative.datum ~= context.datum then
                    finishPack()
                    setStatus(authorityError or 'PACK ANNULE: AUTORITE PERDUE')
                    return
                end
                if index > #points then
                    finishPack()
                    setStatus(label .. ': ' .. created .. '/' .. requested .. ' IA ACTIVES')
                    return
                end
                local point = points[index]
                local actorVariantDatum=actorDatums[index]
                testHistory("pack unit=" .. index .. " rank=" .. characterPlan[index])
                index = index + 1
                local directCommand = string.format(
                    'ai_spawn_at %d %d %d %d %d %.6f %.6f %.6f %.6f %.6f %.6f %.6f %.6f %.6f',
                    authoritative.datum, tagDatum, actorVariantDatum, team, tonumber(loadoutProfile) or 0,
                    point.x, point.y, point.z,
                    spawnForward.x, spawnForward.y, spawnForward.z,
                    spawnUp.x, spawnUp.y, spawnUp.z)
                if marineMode ~= 0 then directCommand = directCommand .. ' ' .. tostring(marineMode) end
                runNative(directCommand, function(result, spawnError)
                    local datum = not spawnError and tonumber(tostring(result or ''):match('unit=(-?%d+)')) or nil
                    if not spawnError and datum and datum ~= -1 then
                        created = created + 1
                        if escortRequest then escortRequest.spawned(datum) end
                        if label == 'HUNTER' then
                            local capturedGeneration = state.lifecycleGeneration
                            for _, delay in ipairs({500, 2000}) do
                                ExecuteInGameThreadWithDelay(delay, function()
                                    if capturedGeneration ~= state.lifecycleGeneration then return end
                                    runNative('origin ' .. datum, function(positionResult, positionError)
                                        testHistory('HUNTER follow-up ms=' .. delay .. ' datum=' .. datum .. ' ' .. tostring(positionResult or positionError))
                                    end)
                                end)
                            end
                        end

                        state.lastActiveAI = {
                            datum = datum,
                            team = team,
                            label = label,
                        }
                        if team == 2 and marineMode == 0 then
                            state.followWatchGeneration = state.followWatchGeneration + 1
                            state.followWatchActive = true
                            state.followWatchEmptySamples = 0
                        end
                        logScenarioAI(string.format(
                            'acteur direct actif category=%s datum=%d team=%d point=%.3f,%.3f,%.3f result=%s',
                            label, datum, team, point.x, point.y, point.z, tostring(result)))
                    else
                        logScenarioAI('acteur direct refuse category=' .. label
                            .. ' result=' .. tostring(result or spawnError))
                    end
                    setStatus(label .. ': ' .. created .. '/' .. requested .. ' IA ACTIVES')
                    if spawnError then
                        finishPack()
                        setStatus(label .. ': GROUP STOPPED ' .. created .. '/' .. requested .. ' - SEE LOG')
                        return
                    end
                    ExecuteInGameThreadWithDelay(60, createNext)
                end)
            end
            createNext()
        end)
        end)
    end)
end

local function aiMenuItems()
    local choices = {}
    choices[#choices+1] = { label = 'SPAWN MARINE x1 - MIXED', close = false, action = function()
        spawnActiveSquad("MARINE","objects\\characters\\marine\\marine","objects\\characters\\marine\\ai\\trooper",1,2,1)
    end }
    choices[#choices+1] = { label = 'SPAWN MARINE x10 - MIXED', close = false, action = function()
        spawnActiveSquad("MARINE","objects\\characters\\marine\\marine","objects\\characters\\marine\\ai\\trooper",10,2,1)
    end }
    choices[#choices+1] = { label = 'SPAWN FEMALE MARINE x1 - MIXED', close = false, action = function()
        spawnActiveSquad("FEMALE MARINE","objects\\characters\\marine_female\\marine_female","objects\\characters\\marine_female\\ai\\trooper_female",1,2,1)
    end }
    choices[#choices+1] = { label = 'SPAWN FEMALE MARINE x10 - MIXED', close = false, action = function()
        spawnActiveSquad("FEMALE MARINE","objects\\characters\\marine_female\\marine_female","objects\\characters\\marine_female\\ai\\trooper_female",10,2,1)
    end }
    choices[#choices+1] = { label = 'SPAWN GRUNT x1 - MIXED', close = false, action = function()
        spawnActiveSquad("GRUNT","objects\\characters\\grunt\\grunt","objects\\characters\\grunt\\ai\\grunt",1,3,3)
    end }
    choices[#choices+1] = { label = 'SPAWN GRUNT x10 - MIXED', close = false, action = function()
        spawnActiveSquad("GRUNT","objects\\characters\\grunt\\grunt","objects\\characters\\grunt\\ai\\grunt",10,3,3)
    end }
    choices[#choices+1] = { label = 'SPAWN JACKAL x1 - MIXED', close = false, action = function()
        spawnActiveSquad("JACKAL","objects\\characters\\jackal\\jackal","objects\\characters\\jackal\\ai\\jackal",1,3,4)
    end }
    choices[#choices+1] = { label = 'SPAWN JACKAL x10 - MIXED', close = false, action = function()
        spawnActiveSquad("JACKAL","objects\\characters\\jackal\\jackal","objects\\characters\\jackal\\ai\\jackal",10,3,4)
    end }
    choices[#choices+1] = { label = 'SPAWN ELITE x1 - MIXED', close = false, action = function()
        spawnActiveSquad("ELITE","objects\\characters\\elite\\elite","objects\\characters\\elite\\ai\\elite",1,3,2)
    end }
    choices[#choices+1] = { label = 'SPAWN ELITE x10 - MIXED', close = false, action = function()
        spawnActiveSquad("ELITE","objects\\characters\\elite\\elite","objects\\characters\\elite\\ai\\elite",10,3,2)
    end }
    choices[#choices+1] = { label = 'SPAWN HUNTER x1', close = false, action = function()
        spawnActiveSquad("HUNTER","objects\\characters\\hunter\\hunter","objects\\characters\\hunter\\ai\\hunter",1,3,0)
    end }
    choices[#choices+1] = { label = 'SPAWN HUNTER x10', close = false, action = function()
        spawnActiveSquad("HUNTER","objects\\characters\\hunter\\hunter","objects\\characters\\hunter\\ai\\hunter",10,3,0)
    end }
    choices[#choices+1] = { label = 'CHECK AI BUILD', action = function() runNative('probe', function(result, err) setStatus(err and 'BUILD CHECK FAILED' or 'AI BUILD MATCHES') end) end }
    return choices
end


local observerEnabled=false
local observerDatum=nil
local observerBusy=false
local function toggleObserver()
    if observerBusy then return end
    local context,err=playerContext(true)
    if not context then setStatus(err or 'NO ACTIVE PLAYER');return end
    local desired=(observerEnabled and observerDatum==context.datum) and 0 or 1
    local generation=state.lifecycleGeneration
    observerBusy=true
    runNative(string.format('observer %d %d',context.datum,desired),function(result,failure)
        observerBusy=false
        if generation~=state.lifecycleGeneration then return end
        if failure or not tostring(result):match('^ok observer enabled='..desired) then
            setStatus('OBSERVER CHANGE FAILED - SEE LOG');return
        end
        observerEnabled=desired==1;observerDatum=context.datum
        setStatus(observerEnabled and 'OBSERVER ON - AI IGNORES YOU; SPLASH DAMAGE STILL HURTS' or 'OBSERVER OFF - NORMAL TARGETING')
    end)
end

local function battleAIControl(enabled)
    local context,err=playerContext(true)
    if not context then setStatus(err) return end
    if state.spawnPackGeneration==state.lifecycleGeneration then setStatus('WAIT FOR CURRENT GROUP TO FINISH') return end
    runNative(string.format('ai_control %d %d',context.datum,enabled),function(result,failure)
        if failure then setStatus('AI CONTROL FAILED - SEE LOG') return end
        setStatus(enabled==1 and 'ALL AI RESUMED' or 'ALL AI PAUSED - READY TO SET UP')
    end)
end
local function spawnBattleItem(item)
    local context,err=playerContext(true)
    if not context then setStatus(err) return end
    local generation=state.lifecycleGeneration
    resolveTag(item,function(tag)
        if not tag or generation~=state.lifecycleGeneration then return end
        runNative('origin '..context.datum,function(result,failure)
            local current=playerContext(true)
            if failure or not current or current.datum~=context.datum or generation~=state.lifecycleGeneration then return end
            local pose=parsePose(result)
            if not pose then setStatus('PLAYER POSITION UNAVAILABLE') return end
            local length=math.sqrt(pose.f.x*pose.f.x+pose.f.y*pose.f.y)
            local fx,fy=1,0
            if length>0.001 then fx,fy=pose.f.x/length,pose.f.y/length end
            local distance=item.kind=='vehicle' and 10 or 2
            local lift=item.kind=='vehicle' and 1 or 0.45
            runNative(string.format('item_spawn %d %d %.6f %.6f %.6f %.6f %.6f 0 0 0 1',
                current.datum,tag,pose.p.x+fx*distance,pose.p.y+fy*distance,pose.p.z+lift,fx,fy),function(spawned,spawnError)
                setStatus(spawnError and 'SPAWN FAILED - SEE LOG' or item.label..' SPAWNED')
            end)
        end)
    end)
end
local function newFactionItems(team)
    local choices={}
    if team==8 then choices[#choices+1]={label="FLOOD HUMAN x1",close=false,action=function() spawnActiveSquad("FLOOD HUMAN","objects\\characters\\floodcombat_human\\floodcombat_human","objects\\characters\\floodcombat_human\\ai\\flood_combat_human",1,8,0) end} end
    if team==8 then choices[#choices+1]={label="FLOOD HUMAN x10",close=false,action=function() spawnActiveSquad("FLOOD HUMAN","objects\\characters\\floodcombat_human\\floodcombat_human","objects\\characters\\floodcombat_human\\ai\\flood_combat_human",10,8,0) end} end
    if team==8 then choices[#choices+1]={label="FLOOD ELITE x1",close=false,action=function() spawnActiveSquad("FLOOD ELITE","objects\\characters\\floodcombat_elite\\floodcombat_elite","objects\\characters\\floodcombat_elite\\ai\\floodcombat_elite",1,8,0) end} end
    if team==8 then choices[#choices+1]={label="FLOOD ELITE x10",close=false,action=function() spawnActiveSquad("FLOOD ELITE","objects\\characters\\floodcombat_elite\\floodcombat_elite","objects\\characters\\floodcombat_elite\\ai\\floodcombat_elite",10,8,0) end} end
    if team==8 then choices[#choices+1]={label="FLOOD CARRIER x1",close=false,action=function() spawnActiveSquad("FLOOD CARRIER","objects\\characters\\floodcarrier\\floodcarrier","objects\\characters\\floodcarrier\\ai\\flood_carrier",1,8,0) end} end
    if team==8 then choices[#choices+1]={label="FLOOD CARRIER x10",close=false,action=function() spawnActiveSquad("FLOOD CARRIER","objects\\characters\\floodcarrier\\floodcarrier","objects\\characters\\floodcarrier\\ai\\flood_carrier",10,8,0) end} end
    if team==8 then choices[#choices+1]={label="FLOOD TANK x1",close=false,action=function() spawnActiveSquad("FLOOD TANK","objects\\characters\\flood_tank\\flood_tank","objects\\characters\\flood_tank\\ai\\flood_pureform_tank",1,8,0) end} end
    if team==8 then choices[#choices+1]={label="FLOOD TANK x10",close=false,action=function() spawnActiveSquad("FLOOD TANK","objects\\characters\\flood_tank\\flood_tank","objects\\characters\\flood_tank\\ai\\flood_pureform_tank",10,8,0) end} end
    if team==9 then choices[#choices+1]={label="SENTINEL x1",close=false,action=function() spawnActiveSquad("SENTINEL","objects\\characters\\sentinel_aggressor\\sentinel_aggressor","objects\\characters\\sentinel_aggressor\\ai\\sentinel_aggressor",1,9,0) end} end
    if team==9 then choices[#choices+1]={label="SENTINEL x10",close=false,action=function() spawnActiveSquad("SENTINEL","objects\\characters\\sentinel_aggressor\\sentinel_aggressor","objects\\characters\\sentinel_aggressor\\ai\\sentinel_aggressor",10,9,0) end} end
    if team==9 then choices[#choices+1]={label="SENTINEL MAJOR x1",close=false,action=function() spawnActiveSquad("SENTINEL MAJOR","objects\\characters\\sentinel_aggressor\\sentinel_aggressor","objects\\characters\\sentinel_aggressor\\ai\\sentinel_aggressor_major",1,9,0) end} end
    if team==9 then choices[#choices+1]={label="SENTINEL MAJOR x10",close=false,action=function() spawnActiveSquad("SENTINEL MAJOR","objects\\characters\\sentinel_aggressor\\sentinel_aggressor","objects\\characters\\sentinel_aggressor\\ai\\sentinel_aggressor_major",10,9,0) end} end
    return choices
end
local function bruteItems()
    local choices={}
    choices[#choices+1]={label="BRUTE x1",close=false,action=function() spawnActiveSquad("BRUTE","objects\\characters\\brute\\brute","objects\\characters\\brute\\ai\\brute",1,3,6) end}
    choices[#choices+1]={label="BRUTE x10",close=false,action=function() spawnActiveSquad("BRUTE","objects\\characters\\brute\\brute","objects\\characters\\brute\\ai\\brute",10,3,6) end}
    choices[#choices+1]={label="BRUTE CAPTAIN x1",close=false,action=function() spawnActiveSquad("BRUTE CAPTAIN","objects\\characters\\brute\\brute","objects\\characters\\brute\\ai\\brute_captain",1,3,6) end}
    choices[#choices+1]={label="BRUTE CAPTAIN x10",close=false,action=function() spawnActiveSquad("BRUTE CAPTAIN","objects\\characters\\brute\\brute","objects\\characters\\brute\\ai\\brute_captain",10,3,6) end}
    choices[#choices+1]={label="BRUTE CHIEFTAIN - WEAPON x1",close=false,action=function() spawnActiveSquad("BRUTE CHIEFTAIN - WEAPON","objects\\characters\\brute\\brute","objects\\characters\\brute\\ai\\brute_chieftain_weapon",1,3,6) end}
    choices[#choices+1]={label="BRUTE CHIEFTAIN - WEAPON x10",close=false,action=function() spawnActiveSquad("BRUTE CHIEFTAIN - WEAPON","objects\\characters\\brute\\brute","objects\\characters\\brute\\ai\\brute_chieftain_weapon",10,3,6) end}
    choices[#choices+1]={label="BRUTE CHIEFTAIN - ARMOR x1",close=false,action=function() spawnActiveSquad("BRUTE CHIEFTAIN - ARMOR","objects\\characters\\brute\\brute","objects\\characters\\brute\\ai\\brute_chieftain_armor",1,3,6) end}
    choices[#choices+1]={label="BRUTE CHIEFTAIN - ARMOR x10",close=false,action=function() spawnActiveSquad("BRUTE CHIEFTAIN - ARMOR","objects\\characters\\brute\\brute","objects\\characters\\brute\\ai\\brute_chieftain_armor",10,3,6) end}
    return choices
end

local escort={enabled=false,epoch=0,busy=false,slots={{},{},{}}}
local escortTick
local function escortSchedule(epoch)
    if escort.enabled and escort.epoch==epoch and escort.scheduledEpoch~=epoch then
        escort.scheduledEpoch=epoch
        ExecuteInGameThreadWithDelay(2000,function()
            if escort.scheduledEpoch==epoch then escort.scheduledEpoch=nil end
            if escort.enabled and escort.epoch==epoch then escortTick(epoch) end
        end)
    end
end
escortTick=function(epoch)
    if not escort.enabled or escort.epoch~=epoch then return end
    if escort.busy or nativeBusy or state.spawnPackGeneration==state.lifecycleGeneration then escortSchedule(epoch) return end
    local context=escort.context
    local ok,currentDatum=pcall(function()
        if not context or not valid(context.controller) or not controllerIsLocal(context.controller) then return nil end
        -- UE4SS can return a new Lua wrapper for the same UObject. Follow
        -- current references from the controller; compare the native datum.
        local pawn=context.controller.Pawn
        if not valid(pawn) then return nil end
        local actor=pawn:GetBlamObjectActor()
        if not valid(actor) then return nil end
        local sync=actor.BlamObjectSynchronization
        if not valid(sync) then return nil end
        return tonumber(sync.BlamObjectIndex)
    end)
    if not ok or not currentDatum or currentDatum~=context.datum then
        escort.enabled=false;escort.epoch=escort.epoch+1;escort.context=nil
        setStatus('MARINE ESCORT OFF - PLAYER CHANGED OR UNAVAILABLE')
        testHistory('escort stopped: player reference check ok='..tostring(ok)..' current='..tostring(currentDatum)..' expected='..tostring(context and context.datum))
        return
    end
    local slots=escort.slots
    escort.busy=true
    runNative(string.format('escort_status %d %d %d %d 1',context.datum,slots[1].datum or -1,slots[2].datum or -1,slots[3].datum or -1),function(result,err)
        escort.busy=false
        if escort.slots~=slots or not escort.enabled or escort.epoch~=epoch then return end
        local s1,s2,s3=tostring(result):match('s1=(-?%d+) s2=(-?%d+) s3=(-?%d+)')
        local alive={tonumber(s1),tonumber(s2),tonumber(s3)}
        if err or #alive~=3 or alive[1]<0 or alive[2]<0 or alive[3]<0 then
            escort.enabled=false;escort.epoch=escort.epoch+1;setStatus('MARINE ESCORT OFF - STATUS CHECK FAILED');return
        end
        if state.spawnPackGeneration==state.lifecycleGeneration then escortSchedule(epoch) return end
        local now=os.time();local fill
        for i,slot in ipairs(slots) do
            if alive[i]==0 then
                if slot.datum then slot.datum=nil;slot.due=now+10 end
                if not fill and now>=(slot.due or 0) then fill=i end
            else slot.due=nil end
        end
        if not fill then escortSchedule(epoch) return end
        local slot=slots[fill];escort.busy=true
        local request={
            allowed=function() return escort.enabled and escort.epoch==epoch and escort.slots==slots end,
            spawned=function(datum) if escort.slots==slots then slot.datum=datum;slot.due=nil end end,
            done=function()
                escort.busy=false
                if escort.slots==slots and not slot.datum then slot.due=os.time()+10 end
                escortSchedule(escort.epoch)
            end,
        }
        spawnActiveSquad('MARINE','objects\\characters\\marine\\marine','objects\\characters\\marine\\ai\\trooper',1,2,1,false,false,request)
    end)
end
local function toggleEscort()
    if not escort.enabled then
        local context,err=playerContext(true)
        if not context then setStatus(err or 'NO ACTIVE PLAYER');return end
        escort.context=context
    end
    escort.epoch=escort.epoch+1;escort.enabled=not escort.enabled
    setStatus(escort.enabled and 'MARINE ESCORT ON - MAINTAINING THREE' or 'MARINE ESCORT OFF - REPLACEMENTS STOPPED')
    if escort.enabled then escortTick(escort.epoch) end
end

local johnsonEscort={enabled=false,epoch=0,busy=false,slots={{},{},{}}}
local johnsonTick
local function johnsonSchedule(epoch)
    if johnsonEscort.enabled and johnsonEscort.epoch==epoch and johnsonEscort.scheduledEpoch~=epoch then
        johnsonEscort.scheduledEpoch=epoch
        ExecuteInGameThreadWithDelay(2000,function()
            if johnsonEscort.scheduledEpoch==epoch then johnsonEscort.scheduledEpoch=nil end
            if johnsonEscort.enabled and johnsonEscort.epoch==epoch then johnsonTick(epoch) end
        end)
    end
end
johnsonTick=function(epoch)
    if not johnsonEscort.enabled or johnsonEscort.epoch~=epoch then return end
    if johnsonEscort.busy or nativeBusy or state.spawnPackGeneration==state.lifecycleGeneration then johnsonSchedule(epoch) return end
    local context=johnsonEscort.context
    local ok,currentDatum=pcall(function()
        if not context or not valid(context.controller) or not controllerIsLocal(context.controller) then return nil end
        -- UE4SS can return a new Lua wrapper for the same UObject. Follow
        -- current references from the controller; compare the native datum.
        local pawn=context.controller.Pawn
        if not valid(pawn) then return nil end
        local actor=pawn:GetBlamObjectActor()
        if not valid(actor) then return nil end
        local sync=actor.BlamObjectSynchronization
        if not valid(sync) then return nil end
        return tonumber(sync.BlamObjectIndex)
    end)
    if not ok or not currentDatum or currentDatum~=context.datum then
        johnsonEscort.enabled=false;johnsonEscort.epoch=johnsonEscort.epoch+1;johnsonEscort.context=nil
        setStatus('JOHNSON ESCORT OFF - PLAYER CHANGED OR UNAVAILABLE')
        testHistory('johnsonEscort stopped: player reference check ok='..tostring(ok)..' current='..tostring(currentDatum)..' expected='..tostring(context and context.datum))
        return
    end
    local slots=johnsonEscort.slots
    johnsonEscort.busy=true
    runNative(string.format('escort_status %d %d %d %d 2',context.datum,slots[1].datum or -1,slots[2].datum or -1,slots[3].datum or -1),function(result,err)
        johnsonEscort.busy=false
        if johnsonEscort.slots~=slots or not johnsonEscort.enabled or johnsonEscort.epoch~=epoch then return end
        local s1,s2,s3=tostring(result):match('s1=(-?%d+) s2=(-?%d+) s3=(-?%d+)')
        local alive={tonumber(s1),tonumber(s2),tonumber(s3)}
        if err or #alive~=3 or alive[1]<0 or alive[2]<0 or alive[3]<0 then
            johnsonEscort.enabled=false;johnsonEscort.epoch=johnsonEscort.epoch+1;setStatus('JOHNSON ESCORT OFF - STATUS CHECK FAILED');return
        end
        if state.spawnPackGeneration==state.lifecycleGeneration then johnsonSchedule(epoch) return end
        local now=os.time();local fill
        for i=1,1 do
            local slot=slots[i]
            if alive[i]==0 then
                if slot.datum then slot.datum=nil;slot.due=now+10 end
                if not fill and now>=(slot.due or 0) then fill=i end
            else slot.due=nil end
        end
        if not fill then johnsonSchedule(epoch) return end
        local slot=slots[fill];johnsonEscort.busy=true
        local request={
            allowed=function() return johnsonEscort.enabled and johnsonEscort.epoch==epoch and johnsonEscort.slots==slots end,
            spawned=function(datum) if johnsonEscort.slots==slots then slot.datum=datum;slot.due=nil end end,
            done=function()
                johnsonEscort.busy=false
                if johnsonEscort.slots==slots and not slot.datum then slot.due=os.time()+10 end
                johnsonSchedule(johnsonEscort.epoch)
            end,
        }
        spawnActiveSquad('SGT. JOHNSON',"objects\\characters\\johnson\\johnsonn","objects\\characters\\johnson\\ai\\johnson",1,2,1,false,false,request)
    end)
end
local function toggleJohnson()
    if not johnsonEscort.enabled then
        local context,err=playerContext(true)
        if not context then setStatus(err or 'NO ACTIVE PLAYER');return end
        johnsonEscort.context=context
    end
    johnsonEscort.epoch=johnsonEscort.epoch+1;johnsonEscort.enabled=not johnsonEscort.enabled
    setStatus(johnsonEscort.enabled and 'JOHNSON ESCORT ON - MAINTAINING ONE' or 'JOHNSON ESCORT OFF - REPLACEMENTS STOPPED')
    if johnsonEscort.enabled then johnsonTick(johnsonEscort.epoch) end
end


local aiPages={["AI_0W109"]={{["spawn"]={"BRUTE","objects\\characters\\brute\\brute","objects\\characters\\brute\\ai\\brute",1,3,109},["label"]="SPAWN x1"},{["spawn"]={"BRUTE","objects\\characters\\brute\\brute","objects\\characters\\brute\\ai\\brute",10,3,109},["label"]="SPAWN x10"}},["AI_5W104"]={{["spawn"]={"BRUTE CAPTAIN","objects\\characters\\brute\\brute","objects\\characters\\brute\\ai\\brute_captain",1,3,104},["label"]="SPAWN x1"},{["spawn"]={"BRUTE CAPTAIN","objects\\characters\\brute\\brute","objects\\characters\\brute\\ai\\brute_captain",10,3,104},["label"]="SPAWN x10"}},["AI_10W6"]={{["spawn"]={"BRUTE CHIEFTAIN - WEAPON","objects\\characters\\brute\\brute","objects\\characters\\brute\\ai\\brute_chieftain_weapon",1,3,6},["label"]="SPAWN x1"},{["spawn"]={"BRUTE CHIEFTAIN - WEAPON","objects\\characters\\brute\\brute","objects\\characters\\brute\\ai\\brute_chieftain_weapon",10,3,6},["label"]="SPAWN x10"}},["BATTLE_AI"]={{["page"]="AI_MARINES",["label"]="SPAWN MARINES"},{["page"]="AI_COVENANT",["label"]="SPAWN COVENANT"},{["page"]="AI_FLOOD",["label"]="SPAWN FLOOD"},{["page"]="AI_SENTINELS",["label"]="SPAWN SENTINELS"}},["AI_34_R7_W0"]={{["spawn"]={"ELITE","objects\\characters\\elite\\elite","objects\\characters\\elite\\ai\\elite_sacristan_specops",1,3,2,false,true},["label"]="SPAWN x1"},{["spawn"]={"ELITE","objects\\characters\\elite\\elite","objects\\characters\\elite\\ai\\elite_sacristan_specops",10,3,2,false,true},["label"]="SPAWN x10"}},["AI_34_R5"]={{["page"]="AI_34_R5_W0",["label"]="MIXED WEAPONS"},{["page"]="AI_34_R5_W1",["label"]="PLASMA RIFLE"},{["page"]="AI_34_R5_W2",["label"]="RED PLASMA RIFLE"},{["page"]="AI_34_R5_W3",["label"]="NEEDLER"},{["page"]="AI_34_R5_W4",["label"]="ENERGY SWORD"}},["AI_57"]={{["page"]="AI_57W0",["label"]="DEFAULT EQUIPMENT"}},["AI_40W3"]={{["spawn"]={"GRUNT","objects\\characters\\grunt\\grunt","objects\\characters\\grunt\\ai\\grunt",1,3,3},["label"]="SPAWN x1"},{["spawn"]={"GRUNT","objects\\characters\\grunt\\grunt","objects\\characters\\grunt\\ai\\grunt",10,3,3},["label"]="SPAWN x10"}},["AI_34_R7"]={{["page"]="AI_34_R7_W0",["label"]="MIXED WEAPONS"},{["page"]="AI_34_R7_W1",["label"]="PLASMA RIFLE"},{["page"]="AI_34_R7_W2",["label"]="RED PLASMA RIFLE"},{["page"]="AI_34_R7_W3",["label"]="NEEDLER"},{["page"]="AI_34_R7_W4",["label"]="ENERGY SWORD"}},["AI_15W6"]={{["spawn"]={"BRUTE CHIEFTAIN - ARMOR","objects\\characters\\brute\\brute","objects\\characters\\brute\\ai\\brute_chieftain_armor",1,3,6},["label"]="SPAWN x1"},{["spawn"]={"BRUTE CHIEFTAIN - ARMOR","objects\\characters\\brute\\brute","objects\\characters\\brute\\ai\\brute_chieftain_armor",10,3,6},["label"]="SPAWN x10"}},["AI_48"]={{["page"]="AI_48W0",["label"]="DEFAULT EQUIPMENT"}},["AI_34_R4_W1"]={{["spawn"]={"ELITE","objects\\characters\\elite\\elite","objects\\characters\\elite\\ai\\elite_general",1,3,104,false,true},["label"]="SPAWN x1"},{["spawn"]={"ELITE","objects\\characters\\elite\\elite","objects\\characters\\elite\\ai\\elite_general",10,3,104,false,true},["label"]="SPAWN x10"}},["AI_34W104"]={{["spawn"]={"ELITE","objects\\characters\\elite\\elite","objects\\characters\\elite\\ai\\elite",1,3,104},["label"]="SPAWN x1"},{["spawn"]={"ELITE","objects\\characters\\elite\\elite","objects\\characters\\elite\\ai\\elite",10,3,104},["label"]="SPAWN x10"}},["AI_34_R5_W4"]={{["spawn"]={"ELITE","objects\\characters\\elite\\elite","objects\\characters\\elite\\ai\\elite_sacristan_minor",1,3,107,false,true},["label"]="SPAWN x1"},{["spawn"]={"ELITE","objects\\characters\\elite\\elite","objects\\characters\\elite\\ai\\elite_sacristan_minor",10,3,107,false,true},["label"]="SPAWN x10"}},["AI_34_R8_W0"]={{["spawn"]={"ELITE","objects\\characters\\elite\\elite","objects\\characters\\elite\\ai\\elite_sacristan_zealot",1,3,2,false,true},["label"]="SPAWN x1"},{["spawn"]={"ELITE","objects\\characters\\elite\\elite","objects\\characters\\elite\\ai\\elite_sacristan_zealot",10,3,2,false,true},["label"]="SPAWN x10"}},["AI_10W104"]={{["spawn"]={"BRUTE CHIEFTAIN - WEAPON","objects\\characters\\brute\\brute","objects\\characters\\brute\\ai\\brute_chieftain_weapon",1,3,104},["label"]="SPAWN x1"},{["spawn"]={"BRUTE CHIEFTAIN - WEAPON","objects\\characters\\brute\\brute","objects\\characters\\brute\\ai\\brute_chieftain_weapon",10,3,104},["label"]="SPAWN x10"}},["AI_34W105"]={{["spawn"]={"ELITE","objects\\characters\\elite\\elite","objects\\characters\\elite\\ai\\elite",1,3,105},["label"]="SPAWN x1"},{["spawn"]={"ELITE","objects\\characters\\elite\\elite","objects\\characters\\elite\\ai\\elite",10,3,105},["label"]="SPAWN x10"}},["AI_5W106"]={{["spawn"]={"BRUTE CAPTAIN","objects\\characters\\brute\\brute","objects\\characters\\brute\\ai\\brute_captain",1,3,106},["label"]="SPAWN x1"},{["spawn"]={"BRUTE CAPTAIN","objects\\characters\\brute\\brute","objects\\characters\\brute\\ai\\brute_captain",10,3,106},["label"]="SPAWN x10"}},["AI_34_R0_W1"]={{["spawn"]={"ELITE","objects\\characters\\elite\\elite","objects\\characters\\elite\\ai\\elite",1,3,104,false,true},["label"]="SPAWN x1"},{["spawn"]={"ELITE","objects\\characters\\elite\\elite","objects\\characters\\elite\\ai\\elite",10,3,104,false,true},["label"]="SPAWN x10"}},["AI_40W106"]={{["spawn"]={"GRUNT","objects\\characters\\grunt\\grunt","objects\\characters\\grunt\\ai\\grunt",1,3,106},["label"]="SPAWN x1"},{["spawn"]={"GRUNT","objects\\characters\\grunt\\grunt","objects\\characters\\grunt\\ai\\grunt",10,3,106},["label"]="SPAWN x10"}},["AI_34_R6_W3"]={{["spawn"]={"ELITE","objects\\characters\\elite\\elite","objects\\characters\\elite\\ai\\elite_sacristan_major",1,3,106,false,true},["label"]="SPAWN x1"},{["spawn"]={"ELITE","objects\\characters\\elite\\elite","objects\\characters\\elite\\ai\\elite_sacristan_major",10,3,106,false,true},["label"]="SPAWN x10"}},["AI_34_R6"]={{["page"]="AI_34_R6_W0",["label"]="MIXED WEAPONS"},{["page"]="AI_34_R6_W1",["label"]="PLASMA RIFLE"},{["page"]="AI_34_R6_W2",["label"]="RED PLASMA RIFLE"},{["page"]="AI_34_R6_W3",["label"]="NEEDLER"},{["page"]="AI_34_R6_W4",["label"]="ENERGY SWORD"}},["AI_0W6"]={{["spawn"]={"BRUTE","objects\\characters\\brute\\brute","objects\\characters\\brute\\ai\\brute",1,3,6},["label"]="SPAWN x1"},{["spawn"]={"BRUTE","objects\\characters\\brute\\brute","objects\\characters\\brute\\ai\\brute",10,3,6},["label"]="SPAWN x10"}},["AI_34_R3_W2"]={{["spawn"]={"ELITE","objects\\characters\\elite\\elite","objects\\characters\\elite\\ai\\elite_specops",1,3,105,false,true},["label"]="SPAWN x1"},{["spawn"]={"ELITE","objects\\characters\\elite\\elite","objects\\characters\\elite\\ai\\elite_specops",10,3,105,false,true},["label"]="SPAWN x10"}},["AI_34_R5_W0"]={{["spawn"]={"ELITE","objects\\characters\\elite\\elite","objects\\characters\\elite\\ai\\elite_sacristan_minor",1,3,2,false,true},["label"]="SPAWN x1"},{["spawn"]={"ELITE","objects\\characters\\elite\\elite","objects\\characters\\elite\\ai\\elite_sacristan_minor",10,3,2,false,true},["label"]="SPAWN x10"}},["AI_44"]={{["page"]="AI_44W4",["label"]="MIXED WEAPONS"},{["page"]="AI_44W108",["label"]="PLASMA PISTOL"},{["page"]="AI_44W106",["label"]="NEEDLER"}},["AI_34_R2_W3"]={{["spawn"]={"ELITE","objects\\characters\\elite\\elite","objects\\characters\\elite\\ai\\elite_ultra",1,3,106,false,true},["label"]="SPAWN x1"},{["spawn"]={"ELITE","objects\\characters\\elite\\elite","objects\\characters\\elite\\ai\\elite_ultra",10,3,106,false,true},["label"]="SPAWN x10"}},["AI_34_R4_W0"]={{["spawn"]={"ELITE","objects\\characters\\elite\\elite","objects\\characters\\elite\\ai\\elite_general",1,3,2,false,true},["label"]="SPAWN x1"},{["spawn"]={"ELITE","objects\\characters\\elite\\elite","objects\\characters\\elite\\ai\\elite_general",10,3,2,false,true},["label"]="SPAWN x10"}},["AI_34_R5_W2"]={{["spawn"]={"ELITE","objects\\characters\\elite\\elite","objects\\characters\\elite\\ai\\elite_sacristan_minor",1,3,105,false,true},["label"]="SPAWN x1"},{["spawn"]={"ELITE","objects\\characters\\elite\\elite","objects\\characters\\elite\\ai\\elite_sacristan_minor",10,3,105,false,true},["label"]="SPAWN x10"}},["AI_34W2"]={{["spawn"]={"ELITE","objects\\characters\\elite\\elite","objects\\characters\\elite\\ai\\elite",1,3,2},["label"]="SPAWN x1"},{["spawn"]={"ELITE","objects\\characters\\elite\\elite","objects\\characters\\elite\\ai\\elite",10,3,2},["label"]="SPAWN x10"}},["AI_34_R8_W1"]={{["spawn"]={"ELITE","objects\\characters\\elite\\elite","objects\\characters\\elite\\ai\\elite_sacristan_zealot",1,3,104,false,true},["label"]="SPAWN x1"},{["spawn"]={"ELITE","objects\\characters\\elite\\elite","objects\\characters\\elite\\ai\\elite_sacristan_zealot",10,3,104,false,true},["label"]="SPAWN x10"}},["AI_34_R1"]={{["page"]="AI_34_R1_W0",["label"]="MIXED WEAPONS"},{["page"]="AI_34_R1_W1",["label"]="PLASMA RIFLE"},{["page"]="AI_34_R1_W2",["label"]="RED PLASMA RIFLE"},{["page"]="AI_34_R1_W3",["label"]="NEEDLER"},{["page"]="AI_34_R1_W4",["label"]="ENERGY SWORD"}},["AI_60W0"]={{["spawn"]={"SENTINEL","objects\\characters\\sentinel_aggressor\\sentinel_aggressor","objects\\characters\\sentinel_aggressor\\ai\\sentinel_aggressor",1,9,0},["label"]="SPAWN x1"},{["spawn"]={"SENTINEL","objects\\characters\\sentinel_aggressor\\sentinel_aggressor","objects\\characters\\sentinel_aggressor\\ai\\sentinel_aggressor",10,9,0},["label"]="SPAWN x10"}},["AI_51W0"]={{["spawn"]={"FLOOD HUMAN","objects\\characters\\floodcombat_human\\floodcombat_human","objects\\characters\\floodcombat_human\\ai\\flood_combat_human",1,8,0},["label"]="SPAWN x1"},{["spawn"]={"FLOOD HUMAN","objects\\characters\\floodcombat_human\\floodcombat_human","objects\\characters\\floodcombat_human\\ai\\flood_combat_human",10,8,0},["label"]="SPAWN x10"}},["AI_34_R3_W4"]={{["spawn"]={"ELITE","objects\\characters\\elite\\elite","objects\\characters\\elite\\ai\\elite_specops",1,3,107,false,true},["label"]="SPAWN x1"},{["spawn"]={"ELITE","objects\\characters\\elite\\elite","objects\\characters\\elite\\ai\\elite_specops",10,3,107,false,true},["label"]="SPAWN x10"}},["AI_48W0"]={{["spawn"]={"HUNTER","objects\\characters\\hunter\\hunter","objects\\characters\\hunter\\ai\\hunter",1,3,0},["label"]="SPAWN x1"},{["spawn"]={"HUNTER","objects\\characters\\hunter\\hunter","objects\\characters\\hunter\\ai\\hunter",10,3,0},["label"]="SPAWN x10"}},["AI_34_R4_W2"]={{["spawn"]={"ELITE","objects\\characters\\elite\\elite","objects\\characters\\elite\\ai\\elite_general",1,3,105,false,true},["label"]="SPAWN x1"},{["spawn"]={"ELITE","objects\\characters\\elite\\elite","objects\\characters\\elite\\ai\\elite_general",10,3,105,false,true},["label"]="SPAWN x10"}},["AI_34_R1_W4"]={{["spawn"]={"ELITE","objects\\characters\\elite\\elite","objects\\characters\\elite\\ai\\elite_officer",1,3,107,false,true},["label"]="SPAWN x1"},{["spawn"]={"ELITE","objects\\characters\\elite\\elite","objects\\characters\\elite\\ai\\elite_officer",10,3,107,false,true},["label"]="SPAWN x10"}},["AI_5"]={{["page"]="AI_5W6",["label"]="MIXED WEAPONS"},{["page"]="AI_5W104",["label"]="PLASMA RIFLE"},{["page"]="AI_5W106",["label"]="NEEDLER"},{["page"]="AI_5W109",["label"]="NEEDLE RIFLE"}},["AI_34_R1_W0"]={{["spawn"]={"ELITE","objects\\characters\\elite\\elite","objects\\characters\\elite\\ai\\elite_officer",1,3,2,false,true},["label"]="SPAWN x1"},{["spawn"]={"ELITE","objects\\characters\\elite\\elite","objects\\characters\\elite\\ai\\elite_officer",10,3,2,false,true},["label"]="SPAWN x10"}},["AI_62"]={{["page"]="AI_62W0",["label"]="DEFAULT EQUIPMENT"}},["AI_0W106"]={{["spawn"]={"BRUTE","objects\\characters\\brute\\brute","objects\\characters\\brute\\ai\\brute",1,3,106},["label"]="SPAWN x1"},{["spawn"]={"BRUTE","objects\\characters\\brute\\brute","objects\\characters\\brute\\ai\\brute",10,3,106},["label"]="SPAWN x10"}},["AI_34_R5_W3"]={{["spawn"]={"ELITE","objects\\characters\\elite\\elite","objects\\characters\\elite\\ai\\elite_sacristan_minor",1,3,106,false,true},["label"]="SPAWN x1"},{["spawn"]={"ELITE","objects\\characters\\elite\\elite","objects\\characters\\elite\\ai\\elite_sacristan_minor",10,3,106,false,true},["label"]="SPAWN x10"}},["AI_51"]={{["page"]="AI_51W0",["label"]="DEFAULT EQUIPMENT"}},["AI_MARINES"]={{["page"]="MARINE_FOLLOW",["label"]="FOLLOW"},{["page"]="MARINE_FREE",["label"]="FREE COMBAT - NO FOLLOW"},{["label"]="JOIN LOCAL SQUAD ORDERS",["page"]="MARINE_SEARCH"}},["AI_0"]={{["page"]="AI_0W6",["label"]="MIXED WEAPONS"},{["page"]="AI_0W104",["label"]="PLASMA RIFLE"},{["page"]="AI_0W106",["label"]="NEEDLER"},{["page"]="AI_0W109",["label"]="NEEDLE RIFLE"}},["AI_34_R0"]={{["page"]="AI_34_R0_W0",["label"]="MIXED WEAPONS"},{["page"]="AI_34_R0_W1",["label"]="PLASMA RIFLE"},{["page"]="AI_34_R0_W2",["label"]="RED PLASMA RIFLE"},{["page"]="AI_34_R0_W3",["label"]="NEEDLER"},{["page"]="AI_34_R0_W4",["label"]="ENERGY SWORD"}},["AI_34_R0_W3"]={{["spawn"]={"ELITE","objects\\characters\\elite\\elite","objects\\characters\\elite\\ai\\elite",1,3,106,false,true},["label"]="SPAWN x1"},{["spawn"]={"ELITE","objects\\characters\\elite\\elite","objects\\characters\\elite\\ai\\elite",10,3,106,false,true},["label"]="SPAWN x10"}},["AI_34_R8_W2"]={{["spawn"]={"ELITE","objects\\characters\\elite\\elite","objects\\characters\\elite\\ai\\elite_sacristan_zealot",1,3,105,false,true},["label"]="SPAWN x1"},{["spawn"]={"ELITE","objects\\characters\\elite\\elite","objects\\characters\\elite\\ai\\elite_sacristan_zealot",10,3,105,false,true},["label"]="SPAWN x10"}},["AI_34_R4_W4"]={{["spawn"]={"ELITE","objects\\characters\\elite\\elite","objects\\characters\\elite\\ai\\elite_general",1,3,107,false,true},["label"]="SPAWN x1"},{["spawn"]={"ELITE","objects\\characters\\elite\\elite","objects\\characters\\elite\\ai\\elite_general",10,3,107,false,true},["label"]="SPAWN x10"}},["AI_34_R6_W0"]={{["spawn"]={"ELITE","objects\\characters\\elite\\elite","objects\\characters\\elite\\ai\\elite_sacristan_major",1,3,2,false,true},["label"]="SPAWN x1"},{["spawn"]={"ELITE","objects\\characters\\elite\\elite","objects\\characters\\elite\\ai\\elite_sacristan_major",10,3,2,false,true},["label"]="SPAWN x10"}},["AI_53W0"]={{["spawn"]={"FLOOD ELITE","objects\\characters\\floodcombat_elite\\floodcombat_elite","objects\\characters\\floodcombat_elite\\ai\\floodcombat_elite",1,8,0},["label"]="SPAWN x1"},{["spawn"]={"FLOOD ELITE","objects\\characters\\floodcombat_elite\\floodcombat_elite","objects\\characters\\floodcombat_elite\\ai\\floodcombat_elite",10,8,0},["label"]="SPAWN x10"}},["AI_34_R2"]={{["page"]="AI_34_R2_W0",["label"]="MIXED WEAPONS"},{["page"]="AI_34_R2_W1",["label"]="PLASMA RIFLE"},{["page"]="AI_34_R2_W2",["label"]="RED PLASMA RIFLE"},{["page"]="AI_34_R2_W3",["label"]="NEEDLER"},{["page"]="AI_34_R2_W4",["label"]="ENERGY SWORD"}},["AI_55W0"]={{["spawn"]={"FLOOD CARRIER","objects\\characters\\floodcarrier\\floodcarrier","objects\\characters\\floodcarrier\\ai\\flood_carrier",1,8,0},["label"]="SPAWN x1"},{["spawn"]={"FLOOD CARRIER","objects\\characters\\floodcarrier\\floodcarrier","objects\\characters\\floodcarrier\\ai\\flood_carrier",10,8,0},["label"]="SPAWN x10"}},["AI_34_R2_W0"]={{["spawn"]={"ELITE","objects\\characters\\elite\\elite","objects\\characters\\elite\\ai\\elite_ultra",1,3,2,false,true},["label"]="SPAWN x1"},{["spawn"]={"ELITE","objects\\characters\\elite\\elite","objects\\characters\\elite\\ai\\elite_ultra",10,3,2,false,true},["label"]="SPAWN x10"}},["AI_BRUTES"]={{["page"]="AI_0",["label"]="BRUTE"},{["page"]="AI_5",["label"]="BRUTE CAPTAIN"},{["page"]="AI_10",["label"]="BRUTE CHIEFTAIN - WEAPON"},{["page"]="AI_15",["label"]="BRUTE CHIEFTAIN - ARMOR"}},["AI_34_R5_W1"]={{["spawn"]={"ELITE","objects\\characters\\elite\\elite","objects\\characters\\elite\\ai\\elite_sacristan_minor",1,3,104,false,true},["label"]="SPAWN x1"},{["spawn"]={"ELITE","objects\\characters\\elite\\elite","objects\\characters\\elite\\ai\\elite_sacristan_minor",10,3,104,false,true},["label"]="SPAWN x10"}},["AI_10"]={{["page"]="AI_10W6",["label"]="MIXED WEAPONS"},{["page"]="AI_10W104",["label"]="PLASMA RIFLE"},{["page"]="AI_10W106",["label"]="NEEDLER"},{["page"]="AI_10W109",["label"]="NEEDLE RIFLE"}},["AI_34_R0_W2"]={{["spawn"]={"ELITE","objects\\characters\\elite\\elite","objects\\characters\\elite\\ai\\elite",1,3,105,false,true},["label"]="SPAWN x1"},{["spawn"]={"ELITE","objects\\characters\\elite\\elite","objects\\characters\\elite\\ai\\elite",10,3,105,false,true},["label"]="SPAWN x10"}},["AI_34_R3_W1"]={{["spawn"]={"ELITE","objects\\characters\\elite\\elite","objects\\characters\\elite\\ai\\elite_specops",1,3,104,false,true},["label"]="SPAWN x1"},{["spawn"]={"ELITE","objects\\characters\\elite\\elite","objects\\characters\\elite\\ai\\elite_specops",10,3,104,false,true},["label"]="SPAWN x10"}},["MARINE_FOLLOW"]={{["label"]="MARINE",["page"]="MANUAL_FOLLOW_M"},{["label"]="FEMALE MARINE",["page"]="MANUAL_FOLLOW_F"}},["AI_0W104"]={{["spawn"]={"BRUTE","objects\\characters\\brute\\brute","objects\\characters\\brute\\ai\\brute",1,3,104},["label"]="SPAWN x1"},{["spawn"]={"BRUTE","objects\\characters\\brute\\brute","objects\\characters\\brute\\ai\\brute",10,3,104},["label"]="SPAWN x10"}},["AI_34_MIXED"]={{["page"]="AI_34W2",["label"]="MIXED WEAPONS"},{["page"]="AI_34W104",["label"]="PLASMA RIFLE"},{["page"]="AI_34W105",["label"]="RED PLASMA RIFLE"},{["page"]="AI_34W106",["label"]="NEEDLER"},{["page"]="AI_34W107",["label"]="ENERGY SWORD"}},["AI_15W104"]={{["spawn"]={"BRUTE CHIEFTAIN - ARMOR","objects\\characters\\brute\\brute","objects\\characters\\brute\\ai\\brute_chieftain_armor",1,3,104},["label"]="SPAWN x1"},{["spawn"]={"BRUTE CHIEFTAIN - ARMOR","objects\\characters\\brute\\brute","objects\\characters\\brute\\ai\\brute_chieftain_armor",10,3,104},["label"]="SPAWN x10"}},["AI_5W109"]={{["spawn"]={"BRUTE CAPTAIN","objects\\characters\\brute\\brute","objects\\characters\\brute\\ai\\brute_captain",1,3,109},["label"]="SPAWN x1"},{["spawn"]={"BRUTE CAPTAIN","objects\\characters\\brute\\brute","objects\\characters\\brute\\ai\\brute_captain",10,3,109},["label"]="SPAWN x10"}},["MARINE_FREE"]={{["label"]="MARINE",["page"]="MANUAL_FREE_M"},{["label"]="FEMALE MARINE",["page"]="MANUAL_FREE_F"}},["AI_40W108"]={{["spawn"]={"GRUNT","objects\\characters\\grunt\\grunt","objects\\characters\\grunt\\ai\\grunt",1,3,108},["label"]="SPAWN x1"},{["spawn"]={"GRUNT","objects\\characters\\grunt\\grunt","objects\\characters\\grunt\\ai\\grunt",10,3,108},["label"]="SPAWN x10"}},["AI_COVENANT"]={{["page"]="AI_34",["label"]="ELITE"},{["page"]="AI_BRUTES",["label"]="BRUTES"},{["page"]="AI_40",["label"]="GRUNT"},{["page"]="AI_44",["label"]="JACKAL"},{["page"]="AI_48",["label"]="HUNTER"}},["AI_34_R7_W4"]={{["spawn"]={"ELITE","objects\\characters\\elite\\elite","objects\\characters\\elite\\ai\\elite_sacristan_specops",1,3,107,false,true},["label"]="SPAWN x1"},{["spawn"]={"ELITE","objects\\characters\\elite\\elite","objects\\characters\\elite\\ai\\elite_sacristan_specops",10,3,107,false,true},["label"]="SPAWN x10"}},["AI_FLOOD"]={{["page"]="AI_51",["label"]="FLOOD HUMAN"},{["page"]="AI_53",["label"]="FLOOD ELITE"},{["page"]="AI_55",["label"]="FLOOD CARRIER"},{["page"]="AI_57",["label"]="FLOOD TANK"}},["AI_5W6"]={{["spawn"]={"BRUTE CAPTAIN","objects\\characters\\brute\\brute","objects\\characters\\brute\\ai\\brute_captain",1,3,6},["label"]="SPAWN x1"},{["spawn"]={"BRUTE CAPTAIN","objects\\characters\\brute\\brute","objects\\characters\\brute\\ai\\brute_captain",10,3,6},["label"]="SPAWN x10"}},["AI_44W108"]={{["spawn"]={"JACKAL","objects\\characters\\jackal\\jackal","objects\\characters\\jackal\\ai\\jackal",1,3,108},["label"]="SPAWN x1"},{["spawn"]={"JACKAL","objects\\characters\\jackal\\jackal","objects\\characters\\jackal\\ai\\jackal",10,3,108},["label"]="SPAWN x10"}},["AI_34_R6_W1"]={{["spawn"]={"ELITE","objects\\characters\\elite\\elite","objects\\characters\\elite\\ai\\elite_sacristan_major",1,3,104,false,true},["label"]="SPAWN x1"},{["spawn"]={"ELITE","objects\\characters\\elite\\elite","objects\\characters\\elite\\ai\\elite_sacristan_major",10,3,104,false,true},["label"]="SPAWN x10"}},["AI_34_R1_W3"]={{["spawn"]={"ELITE","objects\\characters\\elite\\elite","objects\\characters\\elite\\ai\\elite_officer",1,3,106,false,true},["label"]="SPAWN x1"},{["spawn"]={"ELITE","objects\\characters\\elite\\elite","objects\\characters\\elite\\ai\\elite_officer",10,3,106,false,true},["label"]="SPAWN x10"}},["AI_34_R8_W3"]={{["spawn"]={"ELITE","objects\\characters\\elite\\elite","objects\\characters\\elite\\ai\\elite_sacristan_zealot",1,3,106,false,true},["label"]="SPAWN x1"},{["spawn"]={"ELITE","objects\\characters\\elite\\elite","objects\\characters\\elite\\ai\\elite_sacristan_zealot",10,3,106,false,true},["label"]="SPAWN x10"}},["AI_34_R8_W4"]={{["spawn"]={"ELITE","objects\\characters\\elite\\elite","objects\\characters\\elite\\ai\\elite_sacristan_zealot",1,3,107,false,true},["label"]="SPAWN x1"},{["spawn"]={"ELITE","objects\\characters\\elite\\elite","objects\\characters\\elite\\ai\\elite_sacristan_zealot",10,3,107,false,true},["label"]="SPAWN x10"}},["AI_34_R0_W4"]={{["spawn"]={"ELITE","objects\\characters\\elite\\elite","objects\\characters\\elite\\ai\\elite",1,3,107,false,true},["label"]="SPAWN x1"},{["spawn"]={"ELITE","objects\\characters\\elite\\elite","objects\\characters\\elite\\ai\\elite",10,3,107,false,true},["label"]="SPAWN x10"}},["AI_34_R1_W2"]={{["spawn"]={"ELITE","objects\\characters\\elite\\elite","objects\\characters\\elite\\ai\\elite_officer",1,3,105,false,true},["label"]="SPAWN x1"},{["spawn"]={"ELITE","objects\\characters\\elite\\elite","objects\\characters\\elite\\ai\\elite_officer",10,3,105,false,true},["label"]="SPAWN x10"}},["AI_34_R7_W2"]={{["spawn"]={"ELITE","objects\\characters\\elite\\elite","objects\\characters\\elite\\ai\\elite_sacristan_specops",1,3,105,false,true},["label"]="SPAWN x1"},{["spawn"]={"ELITE","objects\\characters\\elite\\elite","objects\\characters\\elite\\ai\\elite_sacristan_specops",10,3,105,false,true},["label"]="SPAWN x10"}},["AI_34_R7_W1"]={{["spawn"]={"ELITE","objects\\characters\\elite\\elite","objects\\characters\\elite\\ai\\elite_sacristan_specops",1,3,104,false,true},["label"]="SPAWN x1"},{["spawn"]={"ELITE","objects\\characters\\elite\\elite","objects\\characters\\elite\\ai\\elite_sacristan_specops",10,3,104,false,true},["label"]="SPAWN x10"}},["AI_34_R6_W4"]={{["spawn"]={"ELITE","objects\\characters\\elite\\elite","objects\\characters\\elite\\ai\\elite_sacristan_major",1,3,107,false,true},["label"]="SPAWN x1"},{["spawn"]={"ELITE","objects\\characters\\elite\\elite","objects\\characters\\elite\\ai\\elite_sacristan_major",10,3,107,false,true},["label"]="SPAWN x10"}},["AI_60"]={{["page"]="AI_60W0",["label"]="DEFAULT EQUIPMENT"}},["AI_34_R6_W2"]={{["spawn"]={"ELITE","objects\\characters\\elite\\elite","objects\\characters\\elite\\ai\\elite_sacristan_major",1,3,105,false,true},["label"]="SPAWN x1"},{["spawn"]={"ELITE","objects\\characters\\elite\\elite","objects\\characters\\elite\\ai\\elite_sacristan_major",10,3,105,false,true},["label"]="SPAWN x10"}},["AI_34_R4"]={{["page"]="AI_34_R4_W0",["label"]="MIXED WEAPONS"},{["page"]="AI_34_R4_W1",["label"]="PLASMA RIFLE"},{["page"]="AI_34_R4_W2",["label"]="RED PLASMA RIFLE"},{["page"]="AI_34_R4_W3",["label"]="NEEDLER"},{["page"]="AI_34_R4_W4",["label"]="ENERGY SWORD"}},["AI_34_R4_W3"]={{["spawn"]={"ELITE","objects\\characters\\elite\\elite","objects\\characters\\elite\\ai\\elite_general",1,3,106,false,true},["label"]="SPAWN x1"},{["spawn"]={"ELITE","objects\\characters\\elite\\elite","objects\\characters\\elite\\ai\\elite_general",10,3,106,false,true},["label"]="SPAWN x10"}},["AI_15W106"]={{["spawn"]={"BRUTE CHIEFTAIN - ARMOR","objects\\characters\\brute\\brute","objects\\characters\\brute\\ai\\brute_chieftain_armor",1,3,106},["label"]="SPAWN x1"},{["spawn"]={"BRUTE CHIEFTAIN - ARMOR","objects\\characters\\brute\\brute","objects\\characters\\brute\\ai\\brute_chieftain_armor",10,3,106},["label"]="SPAWN x10"}},["AI_34_R3"]={{["page"]="AI_34_R3_W0",["label"]="MIXED WEAPONS"},{["page"]="AI_34_R3_W1",["label"]="PLASMA RIFLE"},{["page"]="AI_34_R3_W2",["label"]="RED PLASMA RIFLE"},{["page"]="AI_34_R3_W3",["label"]="NEEDLER"},{["page"]="AI_34_R3_W4",["label"]="ENERGY SWORD"}},["AI_34_R3_W0"]={{["spawn"]={"ELITE","objects\\characters\\elite\\elite","objects\\characters\\elite\\ai\\elite_specops",1,3,2,false,true},["label"]="SPAWN x1"},{["spawn"]={"ELITE","objects\\characters\\elite\\elite","objects\\characters\\elite\\ai\\elite_specops",10,3,2,false,true},["label"]="SPAWN x10"}},["AI_34_R2_W4"]={{["spawn"]={"ELITE","objects\\characters\\elite\\elite","objects\\characters\\elite\\ai\\elite_ultra",1,3,107,false,true},["label"]="SPAWN x1"},{["spawn"]={"ELITE","objects\\characters\\elite\\elite","objects\\characters\\elite\\ai\\elite_ultra",10,3,107,false,true},["label"]="SPAWN x10"}},["AI_34_R2_W1"]={{["spawn"]={"ELITE","objects\\characters\\elite\\elite","objects\\characters\\elite\\ai\\elite_ultra",1,3,104,false,true},["label"]="SPAWN x1"},{["spawn"]={"ELITE","objects\\characters\\elite\\elite","objects\\characters\\elite\\ai\\elite_ultra",10,3,104,false,true},["label"]="SPAWN x10"}},["AI_34_R2_W2"]={{["spawn"]={"ELITE","objects\\characters\\elite\\elite","objects\\characters\\elite\\ai\\elite_ultra",1,3,105,false,true},["label"]="SPAWN x1"},{["spawn"]={"ELITE","objects\\characters\\elite\\elite","objects\\characters\\elite\\ai\\elite_ultra",10,3,105,false,true},["label"]="SPAWN x10"}},["AI_34_R8"]={{["page"]="AI_34_R8_W0",["label"]="MIXED WEAPONS"},{["page"]="AI_34_R8_W1",["label"]="PLASMA RIFLE"},{["page"]="AI_34_R8_W2",["label"]="RED PLASMA RIFLE"},{["page"]="AI_34_R8_W3",["label"]="NEEDLER"},{["page"]="AI_34_R8_W4",["label"]="ENERGY SWORD"}},["AI_34_R1_W1"]={{["spawn"]={"ELITE","objects\\characters\\elite\\elite","objects\\characters\\elite\\ai\\elite_officer",1,3,104,false,true},["label"]="SPAWN x1"},{["spawn"]={"ELITE","objects\\characters\\elite\\elite","objects\\characters\\elite\\ai\\elite_officer",10,3,104,false,true},["label"]="SPAWN x10"}},["AI_10W109"]={{["spawn"]={"BRUTE CHIEFTAIN - WEAPON","objects\\characters\\brute\\brute","objects\\characters\\brute\\ai\\brute_chieftain_weapon",1,3,109},["label"]="SPAWN x1"},{["spawn"]={"BRUTE CHIEFTAIN - WEAPON","objects\\characters\\brute\\brute","objects\\characters\\brute\\ai\\brute_chieftain_weapon",10,3,109},["label"]="SPAWN x10"}},["AI_15"]={{["page"]="AI_15W6",["label"]="MIXED WEAPONS"},{["page"]="AI_15W104",["label"]="PLASMA RIFLE"},{["page"]="AI_15W106",["label"]="NEEDLER"},{["page"]="AI_15W109",["label"]="NEEDLE RIFLE"}},["AI_15W109"]={{["spawn"]={"BRUTE CHIEFTAIN - ARMOR","objects\\characters\\brute\\brute","objects\\characters\\brute\\ai\\brute_chieftain_armor",1,3,109},["label"]="SPAWN x1"},{["spawn"]={"BRUTE CHIEFTAIN - ARMOR","objects\\characters\\brute\\brute","objects\\characters\\brute\\ai\\brute_chieftain_armor",10,3,109},["label"]="SPAWN x10"}},["AI_10W106"]={{["spawn"]={"BRUTE CHIEFTAIN - WEAPON","objects\\characters\\brute\\brute","objects\\characters\\brute\\ai\\brute_chieftain_weapon",1,3,106},["label"]="SPAWN x1"},{["spawn"]={"BRUTE CHIEFTAIN - WEAPON","objects\\characters\\brute\\brute","objects\\characters\\brute\\ai\\brute_chieftain_weapon",10,3,106},["label"]="SPAWN x10"}},["AI_SENTINELS"]={{["page"]="AI_60",["label"]="SENTINEL"},{["page"]="AI_62",["label"]="SENTINEL MAJOR"}},["AI_62W0"]={{["spawn"]={"SENTINEL MAJOR","objects\\characters\\sentinel_aggressor\\sentinel_aggressor","objects\\characters\\sentinel_aggressor\\ai\\sentinel_aggressor_major",1,9,0},["label"]="SPAWN x1"},{["spawn"]={"SENTINEL MAJOR","objects\\characters\\sentinel_aggressor\\sentinel_aggressor","objects\\characters\\sentinel_aggressor\\ai\\sentinel_aggressor_major",10,9,0},["label"]="SPAWN x10"}},["AI_44W4"]={{["spawn"]={"JACKAL","objects\\characters\\jackal\\jackal","objects\\characters\\jackal\\ai\\jackal",1,3,4},["label"]="SPAWN x1"},{["spawn"]={"JACKAL","objects\\characters\\jackal\\jackal","objects\\characters\\jackal\\ai\\jackal",10,3,4},["label"]="SPAWN x10"}},["AI_34_R3_W3"]={{["spawn"]={"ELITE","objects\\characters\\elite\\elite","objects\\characters\\elite\\ai\\elite_specops",1,3,106,false,true},["label"]="SPAWN x1"},{["spawn"]={"ELITE","objects\\characters\\elite\\elite","objects\\characters\\elite\\ai\\elite_specops",10,3,106,false,true},["label"]="SPAWN x10"}},["AI_57W0"]={{["spawn"]={"FLOOD TANK","objects\\characters\\flood_tank\\flood_tank","objects\\characters\\flood_tank\\ai\\flood_pureform_tank",1,8,0},["label"]="SPAWN x1"},{["spawn"]={"FLOOD TANK","objects\\characters\\flood_tank\\flood_tank","objects\\characters\\flood_tank\\ai\\flood_pureform_tank",10,8,0},["label"]="SPAWN x10"}},["AI_55"]={{["page"]="AI_55W0",["label"]="DEFAULT EQUIPMENT"}},["AI_44W106"]={{["spawn"]={"JACKAL","objects\\characters\\jackal\\jackal","objects\\characters\\jackal\\ai\\jackal",1,3,106},["label"]="SPAWN x1"},{["spawn"]={"JACKAL","objects\\characters\\jackal\\jackal","objects\\characters\\jackal\\ai\\jackal",10,3,106},["label"]="SPAWN x10"}},["AI_34_R7_W3"]={{["spawn"]={"ELITE","objects\\characters\\elite\\elite","objects\\characters\\elite\\ai\\elite_sacristan_specops",1,3,106,false,true},["label"]="SPAWN x1"},{["spawn"]={"ELITE","objects\\characters\\elite\\elite","objects\\characters\\elite\\ai\\elite_sacristan_specops",10,3,106,false,true},["label"]="SPAWN x10"}},["AI_34W106"]={{["spawn"]={"ELITE","objects\\characters\\elite\\elite","objects\\characters\\elite\\ai\\elite",1,3,106},["label"]="SPAWN x1"},{["spawn"]={"ELITE","objects\\characters\\elite\\elite","objects\\characters\\elite\\ai\\elite",10,3,106},["label"]="SPAWN x10"}},["AI_53"]={{["page"]="AI_53W0",["label"]="DEFAULT EQUIPMENT"}},["AI_34"]={{["page"]="AI_34_MIXED",["label"]="MIXED RANKS"},{["page"]="AI_34_R0",["label"]="MINOR"},{["page"]="AI_34_R1",["label"]="OFFICER"},{["page"]="AI_34_R2",["label"]="ULTRA"},{["page"]="AI_34_R3",["label"]="SPEC OPS"},{["page"]="AI_34_R4",["label"]="GENERAL"},{["page"]="AI_34_R5",["label"]="BONUS MINOR"},{["page"]="AI_34_R6",["label"]="BONUS MAJOR"},{["page"]="AI_34_R7",["label"]="BONUS SPEC OPS"},{["page"]="AI_34_R8",["label"]="BONUS ZEALOT"}},["AI_40"]={{["page"]="AI_40W3",["label"]="MIXED WEAPONS"},{["page"]="AI_40W108",["label"]="PLASMA PISTOL"},{["page"]="AI_40W106",["label"]="NEEDLER"}},["AI_34W107"]={{["spawn"]={"ELITE","objects\\characters\\elite\\elite","objects\\characters\\elite\\ai\\elite",1,3,107},["label"]="SPAWN x1"},{["spawn"]={"ELITE","objects\\characters\\elite\\elite","objects\\characters\\elite\\ai\\elite",10,3,107},["label"]="SPAWN x10"}},["AI_34_R0_W0"]={{["spawn"]={"ELITE","objects\\characters\\elite\\elite","objects\\characters\\elite\\ai\\elite",1,3,2,false,true},["label"]="SPAWN x1"},{["spawn"]={"ELITE","objects\\characters\\elite\\elite","objects\\characters\\elite\\ai\\elite",10,3,2,false,true},["label"]="SPAWN x10"}},["MANUAL_FOLLOW_M"]={{["label"]="MIXED WEAPONS",["page"]="MANUAL_FOLLOW_M_1"},{["label"]="ASSAULT RIFLE",["page"]="MANUAL_FOLLOW_M_100"},{["label"]="BATTLE RIFLE",["page"]="MANUAL_FOLLOW_M_111"},{["label"]="BEAM RIFLE",["page"]="MANUAL_FOLLOW_M_112"},{["label"]="FUEL ROD CANNON",["page"]="MANUAL_FOLLOW_M_113"},{["label"]="MAGNUM",["page"]="MANUAL_FOLLOW_M_101"},{["label"]="NEEDLE RIFLE",["page"]="MANUAL_FOLLOW_M_109"},{["label"]="NEEDLER",["page"]="MANUAL_FOLLOW_M_106"},{["label"]="PLASMA PISTOL",["page"]="MANUAL_FOLLOW_M_108"},{["label"]="PLASMA RIFLE",["page"]="MANUAL_FOLLOW_M_104"},{["label"]="PLASMA RIFLE RED",["page"]="MANUAL_FOLLOW_M_105"},{["label"]="ROCKET LAUNCHER",["page"]="MANUAL_FOLLOW_M_114"},{["label"]="SENTINEL BEAM",["page"]="MANUAL_FOLLOW_M_115"},{["label"]="SHOTGUN",["page"]="MANUAL_FOLLOW_M_102"},{["label"]="SNIPER RIFLE",["page"]="MANUAL_FOLLOW_M_103"},{["label"]="SMG",["page"]="MANUAL_FOLLOW_M_110"}},["MANUAL_FOLLOW_M_1"]={{["label"]="SPAWN x1",["spawn"]={"MARINE","objects\\characters\\marine\\marine","objects\\characters\\marine\\ai\\trooper",1,2,1,false,false,false,0}},{["label"]="SPAWN x10",["spawn"]={"MARINE","objects\\characters\\marine\\marine","objects\\characters\\marine\\ai\\trooper",10,2,1,false,false,false,0}}},["MANUAL_FOLLOW_M_100"]={{["label"]="SPAWN x1",["spawn"]={"MARINE","objects\\characters\\marine\\marine","objects\\characters\\marine\\ai\\trooper",1,2,100,false,false,false,0}},{["label"]="SPAWN x10",["spawn"]={"MARINE","objects\\characters\\marine\\marine","objects\\characters\\marine\\ai\\trooper",10,2,100,false,false,false,0}}},["MANUAL_FOLLOW_M_111"]={{["label"]="SPAWN x1",["spawn"]={"MARINE","objects\\characters\\marine\\marine","objects\\characters\\marine\\ai\\trooper",1,2,111,false,false,false,0}},{["label"]="SPAWN x10",["spawn"]={"MARINE","objects\\characters\\marine\\marine","objects\\characters\\marine\\ai\\trooper",10,2,111,false,false,false,0}}},["MANUAL_FOLLOW_M_112"]={{["label"]="SPAWN x1",["spawn"]={"MARINE","objects\\characters\\marine\\marine","objects\\characters\\marine\\ai\\trooper",1,2,112,false,false,false,0}},{["label"]="SPAWN x10",["spawn"]={"MARINE","objects\\characters\\marine\\marine","objects\\characters\\marine\\ai\\trooper",10,2,112,false,false,false,0}}},["MANUAL_FOLLOW_M_113"]={{["label"]="SPAWN x1",["spawn"]={"MARINE","objects\\characters\\marine\\marine","objects\\characters\\marine\\ai\\trooper",1,2,113,false,false,false,0}},{["label"]="SPAWN x10",["spawn"]={"MARINE","objects\\characters\\marine\\marine","objects\\characters\\marine\\ai\\trooper",10,2,113,false,false,false,0}}},["MANUAL_FOLLOW_M_101"]={{["label"]="SPAWN x1",["spawn"]={"MARINE","objects\\characters\\marine\\marine","objects\\characters\\marine\\ai\\trooper",1,2,101,false,false,false,0}},{["label"]="SPAWN x10",["spawn"]={"MARINE","objects\\characters\\marine\\marine","objects\\characters\\marine\\ai\\trooper",10,2,101,false,false,false,0}}},["MANUAL_FOLLOW_M_109"]={{["label"]="SPAWN x1",["spawn"]={"MARINE","objects\\characters\\marine\\marine","objects\\characters\\marine\\ai\\trooper",1,2,109,false,false,false,0}},{["label"]="SPAWN x10",["spawn"]={"MARINE","objects\\characters\\marine\\marine","objects\\characters\\marine\\ai\\trooper",10,2,109,false,false,false,0}}},["MANUAL_FOLLOW_M_106"]={{["label"]="SPAWN x1",["spawn"]={"MARINE","objects\\characters\\marine\\marine","objects\\characters\\marine\\ai\\trooper",1,2,106,false,false,false,0}},{["label"]="SPAWN x10",["spawn"]={"MARINE","objects\\characters\\marine\\marine","objects\\characters\\marine\\ai\\trooper",10,2,106,false,false,false,0}}},["MANUAL_FOLLOW_M_108"]={{["label"]="SPAWN x1",["spawn"]={"MARINE","objects\\characters\\marine\\marine","objects\\characters\\marine\\ai\\trooper",1,2,108,false,false,false,0}},{["label"]="SPAWN x10",["spawn"]={"MARINE","objects\\characters\\marine\\marine","objects\\characters\\marine\\ai\\trooper",10,2,108,false,false,false,0}}},["MANUAL_FOLLOW_M_104"]={{["label"]="SPAWN x1",["spawn"]={"MARINE","objects\\characters\\marine\\marine","objects\\characters\\marine\\ai\\trooper",1,2,104,false,false,false,0}},{["label"]="SPAWN x10",["spawn"]={"MARINE","objects\\characters\\marine\\marine","objects\\characters\\marine\\ai\\trooper",10,2,104,false,false,false,0}}},["MANUAL_FOLLOW_M_105"]={{["label"]="SPAWN x1",["spawn"]={"MARINE","objects\\characters\\marine\\marine","objects\\characters\\marine\\ai\\trooper",1,2,105,false,false,false,0}},{["label"]="SPAWN x10",["spawn"]={"MARINE","objects\\characters\\marine\\marine","objects\\characters\\marine\\ai\\trooper",10,2,105,false,false,false,0}}},["MANUAL_FOLLOW_M_114"]={{["label"]="SPAWN x1",["spawn"]={"MARINE","objects\\characters\\marine\\marine","objects\\characters\\marine\\ai\\trooper",1,2,114,false,false,false,0}},{["label"]="SPAWN x10",["spawn"]={"MARINE","objects\\characters\\marine\\marine","objects\\characters\\marine\\ai\\trooper",10,2,114,false,false,false,0}}},["MANUAL_FOLLOW_M_115"]={{["label"]="SPAWN x1",["spawn"]={"MARINE","objects\\characters\\marine\\marine","objects\\characters\\marine\\ai\\trooper",1,2,115,false,false,false,0}},{["label"]="SPAWN x10",["spawn"]={"MARINE","objects\\characters\\marine\\marine","objects\\characters\\marine\\ai\\trooper",10,2,115,false,false,false,0}}},["MANUAL_FOLLOW_M_102"]={{["label"]="SPAWN x1",["spawn"]={"MARINE","objects\\characters\\marine\\marine","objects\\characters\\marine\\ai\\trooper",1,2,102,false,false,false,0}},{["label"]="SPAWN x10",["spawn"]={"MARINE","objects\\characters\\marine\\marine","objects\\characters\\marine\\ai\\trooper",10,2,102,false,false,false,0}}},["MANUAL_FOLLOW_M_103"]={{["label"]="SPAWN x1",["spawn"]={"MARINE","objects\\characters\\marine\\marine","objects\\characters\\marine\\ai\\trooper",1,2,103,false,false,false,0}},{["label"]="SPAWN x10",["spawn"]={"MARINE","objects\\characters\\marine\\marine","objects\\characters\\marine\\ai\\trooper",10,2,103,false,false,false,0}}},["MANUAL_FOLLOW_M_110"]={{["label"]="SPAWN x1",["spawn"]={"MARINE","objects\\characters\\marine\\marine","objects\\characters\\marine\\ai\\trooper",1,2,110,false,false,false,0}},{["label"]="SPAWN x10",["spawn"]={"MARINE","objects\\characters\\marine\\marine","objects\\characters\\marine\\ai\\trooper",10,2,110,false,false,false,0}}},["MANUAL_FOLLOW_F"]={{["label"]="MIXED WEAPONS",["page"]="MANUAL_FOLLOW_F_1"},{["label"]="ASSAULT RIFLE",["page"]="MANUAL_FOLLOW_F_100"},{["label"]="BATTLE RIFLE",["page"]="MANUAL_FOLLOW_F_111"},{["label"]="BEAM RIFLE",["page"]="MANUAL_FOLLOW_F_112"},{["label"]="FUEL ROD CANNON",["page"]="MANUAL_FOLLOW_F_113"},{["label"]="MAGNUM",["page"]="MANUAL_FOLLOW_F_101"},{["label"]="NEEDLE RIFLE",["page"]="MANUAL_FOLLOW_F_109"},{["label"]="NEEDLER",["page"]="MANUAL_FOLLOW_F_106"},{["label"]="PLASMA PISTOL",["page"]="MANUAL_FOLLOW_F_108"},{["label"]="PLASMA RIFLE",["page"]="MANUAL_FOLLOW_F_104"},{["label"]="PLASMA RIFLE RED",["page"]="MANUAL_FOLLOW_F_105"},{["label"]="ROCKET LAUNCHER",["page"]="MANUAL_FOLLOW_F_114"},{["label"]="SENTINEL BEAM",["page"]="MANUAL_FOLLOW_F_115"},{["label"]="SHOTGUN",["page"]="MANUAL_FOLLOW_F_102"},{["label"]="SNIPER RIFLE",["page"]="MANUAL_FOLLOW_F_103"},{["label"]="SMG",["page"]="MANUAL_FOLLOW_F_110"}},["MANUAL_FOLLOW_F_1"]={{["label"]="SPAWN x1",["spawn"]={"FEMALE MARINE","objects\\characters\\marine_female\\marine_female","objects\\characters\\marine_female\\ai\\trooper_female",1,2,1,false,false,false,0}},{["label"]="SPAWN x10",["spawn"]={"FEMALE MARINE","objects\\characters\\marine_female\\marine_female","objects\\characters\\marine_female\\ai\\trooper_female",10,2,1,false,false,false,0}}},["MANUAL_FOLLOW_F_100"]={{["label"]="SPAWN x1",["spawn"]={"FEMALE MARINE","objects\\characters\\marine_female\\marine_female","objects\\characters\\marine_female\\ai\\trooper_female",1,2,100,false,false,false,0}},{["label"]="SPAWN x10",["spawn"]={"FEMALE MARINE","objects\\characters\\marine_female\\marine_female","objects\\characters\\marine_female\\ai\\trooper_female",10,2,100,false,false,false,0}}},["MANUAL_FOLLOW_F_111"]={{["label"]="SPAWN x1",["spawn"]={"FEMALE MARINE","objects\\characters\\marine_female\\marine_female","objects\\characters\\marine_female\\ai\\trooper_female",1,2,111,false,false,false,0}},{["label"]="SPAWN x10",["spawn"]={"FEMALE MARINE","objects\\characters\\marine_female\\marine_female","objects\\characters\\marine_female\\ai\\trooper_female",10,2,111,false,false,false,0}}},["MANUAL_FOLLOW_F_112"]={{["label"]="SPAWN x1",["spawn"]={"FEMALE MARINE","objects\\characters\\marine_female\\marine_female","objects\\characters\\marine_female\\ai\\trooper_female",1,2,112,false,false,false,0}},{["label"]="SPAWN x10",["spawn"]={"FEMALE MARINE","objects\\characters\\marine_female\\marine_female","objects\\characters\\marine_female\\ai\\trooper_female",10,2,112,false,false,false,0}}},["MANUAL_FOLLOW_F_113"]={{["label"]="SPAWN x1",["spawn"]={"FEMALE MARINE","objects\\characters\\marine_female\\marine_female","objects\\characters\\marine_female\\ai\\trooper_female",1,2,113,false,false,false,0}},{["label"]="SPAWN x10",["spawn"]={"FEMALE MARINE","objects\\characters\\marine_female\\marine_female","objects\\characters\\marine_female\\ai\\trooper_female",10,2,113,false,false,false,0}}},["MANUAL_FOLLOW_F_101"]={{["label"]="SPAWN x1",["spawn"]={"FEMALE MARINE","objects\\characters\\marine_female\\marine_female","objects\\characters\\marine_female\\ai\\trooper_female",1,2,101,false,false,false,0}},{["label"]="SPAWN x10",["spawn"]={"FEMALE MARINE","objects\\characters\\marine_female\\marine_female","objects\\characters\\marine_female\\ai\\trooper_female",10,2,101,false,false,false,0}}},["MANUAL_FOLLOW_F_109"]={{["label"]="SPAWN x1",["spawn"]={"FEMALE MARINE","objects\\characters\\marine_female\\marine_female","objects\\characters\\marine_female\\ai\\trooper_female",1,2,109,false,false,false,0}},{["label"]="SPAWN x10",["spawn"]={"FEMALE MARINE","objects\\characters\\marine_female\\marine_female","objects\\characters\\marine_female\\ai\\trooper_female",10,2,109,false,false,false,0}}},["MANUAL_FOLLOW_F_106"]={{["label"]="SPAWN x1",["spawn"]={"FEMALE MARINE","objects\\characters\\marine_female\\marine_female","objects\\characters\\marine_female\\ai\\trooper_female",1,2,106,false,false,false,0}},{["label"]="SPAWN x10",["spawn"]={"FEMALE MARINE","objects\\characters\\marine_female\\marine_female","objects\\characters\\marine_female\\ai\\trooper_female",10,2,106,false,false,false,0}}},["MANUAL_FOLLOW_F_108"]={{["label"]="SPAWN x1",["spawn"]={"FEMALE MARINE","objects\\characters\\marine_female\\marine_female","objects\\characters\\marine_female\\ai\\trooper_female",1,2,108,false,false,false,0}},{["label"]="SPAWN x10",["spawn"]={"FEMALE MARINE","objects\\characters\\marine_female\\marine_female","objects\\characters\\marine_female\\ai\\trooper_female",10,2,108,false,false,false,0}}},["MANUAL_FOLLOW_F_104"]={{["label"]="SPAWN x1",["spawn"]={"FEMALE MARINE","objects\\characters\\marine_female\\marine_female","objects\\characters\\marine_female\\ai\\trooper_female",1,2,104,false,false,false,0}},{["label"]="SPAWN x10",["spawn"]={"FEMALE MARINE","objects\\characters\\marine_female\\marine_female","objects\\characters\\marine_female\\ai\\trooper_female",10,2,104,false,false,false,0}}},["MANUAL_FOLLOW_F_105"]={{["label"]="SPAWN x1",["spawn"]={"FEMALE MARINE","objects\\characters\\marine_female\\marine_female","objects\\characters\\marine_female\\ai\\trooper_female",1,2,105,false,false,false,0}},{["label"]="SPAWN x10",["spawn"]={"FEMALE MARINE","objects\\characters\\marine_female\\marine_female","objects\\characters\\marine_female\\ai\\trooper_female",10,2,105,false,false,false,0}}},["MANUAL_FOLLOW_F_114"]={{["label"]="SPAWN x1",["spawn"]={"FEMALE MARINE","objects\\characters\\marine_female\\marine_female","objects\\characters\\marine_female\\ai\\trooper_female",1,2,114,false,false,false,0}},{["label"]="SPAWN x10",["spawn"]={"FEMALE MARINE","objects\\characters\\marine_female\\marine_female","objects\\characters\\marine_female\\ai\\trooper_female",10,2,114,false,false,false,0}}},["MANUAL_FOLLOW_F_115"]={{["label"]="SPAWN x1",["spawn"]={"FEMALE MARINE","objects\\characters\\marine_female\\marine_female","objects\\characters\\marine_female\\ai\\trooper_female",1,2,115,false,false,false,0}},{["label"]="SPAWN x10",["spawn"]={"FEMALE MARINE","objects\\characters\\marine_female\\marine_female","objects\\characters\\marine_female\\ai\\trooper_female",10,2,115,false,false,false,0}}},["MANUAL_FOLLOW_F_102"]={{["label"]="SPAWN x1",["spawn"]={"FEMALE MARINE","objects\\characters\\marine_female\\marine_female","objects\\characters\\marine_female\\ai\\trooper_female",1,2,102,false,false,false,0}},{["label"]="SPAWN x10",["spawn"]={"FEMALE MARINE","objects\\characters\\marine_female\\marine_female","objects\\characters\\marine_female\\ai\\trooper_female",10,2,102,false,false,false,0}}},["MANUAL_FOLLOW_F_103"]={{["label"]="SPAWN x1",["spawn"]={"FEMALE MARINE","objects\\characters\\marine_female\\marine_female","objects\\characters\\marine_female\\ai\\trooper_female",1,2,103,false,false,false,0}},{["label"]="SPAWN x10",["spawn"]={"FEMALE MARINE","objects\\characters\\marine_female\\marine_female","objects\\characters\\marine_female\\ai\\trooper_female",10,2,103,false,false,false,0}}},["MANUAL_FOLLOW_F_110"]={{["label"]="SPAWN x1",["spawn"]={"FEMALE MARINE","objects\\characters\\marine_female\\marine_female","objects\\characters\\marine_female\\ai\\trooper_female",1,2,110,false,false,false,0}},{["label"]="SPAWN x10",["spawn"]={"FEMALE MARINE","objects\\characters\\marine_female\\marine_female","objects\\characters\\marine_female\\ai\\trooper_female",10,2,110,false,false,false,0}}},["MANUAL_FREE_M"]={{["label"]="MIXED WEAPONS",["page"]="MANUAL_FREE_M_1"},{["label"]="ASSAULT RIFLE",["page"]="MANUAL_FREE_M_100"},{["label"]="BATTLE RIFLE",["page"]="MANUAL_FREE_M_111"},{["label"]="BEAM RIFLE",["page"]="MANUAL_FREE_M_112"},{["label"]="FUEL ROD CANNON",["page"]="MANUAL_FREE_M_113"},{["label"]="MAGNUM",["page"]="MANUAL_FREE_M_101"},{["label"]="NEEDLE RIFLE",["page"]="MANUAL_FREE_M_109"},{["label"]="NEEDLER",["page"]="MANUAL_FREE_M_106"},{["label"]="PLASMA PISTOL",["page"]="MANUAL_FREE_M_108"},{["label"]="PLASMA RIFLE",["page"]="MANUAL_FREE_M_104"},{["label"]="PLASMA RIFLE RED",["page"]="MANUAL_FREE_M_105"},{["label"]="ROCKET LAUNCHER",["page"]="MANUAL_FREE_M_114"},{["label"]="SENTINEL BEAM",["page"]="MANUAL_FREE_M_115"},{["label"]="SHOTGUN",["page"]="MANUAL_FREE_M_102"},{["label"]="SNIPER RIFLE",["page"]="MANUAL_FREE_M_103"},{["label"]="SMG",["page"]="MANUAL_FREE_M_110"}},["MANUAL_FREE_M_1"]={{["label"]="SPAWN x1",["spawn"]={"MARINE","objects\\characters\\marine\\marine","objects\\characters\\marine\\ai\\trooper",1,2,1,false,false,false,1}},{["label"]="SPAWN x10",["spawn"]={"MARINE","objects\\characters\\marine\\marine","objects\\characters\\marine\\ai\\trooper",10,2,1,false,false,false,1}}},["MANUAL_FREE_M_100"]={{["label"]="SPAWN x1",["spawn"]={"MARINE","objects\\characters\\marine\\marine","objects\\characters\\marine\\ai\\trooper",1,2,100,false,false,false,1}},{["label"]="SPAWN x10",["spawn"]={"MARINE","objects\\characters\\marine\\marine","objects\\characters\\marine\\ai\\trooper",10,2,100,false,false,false,1}}},["MANUAL_FREE_M_111"]={{["label"]="SPAWN x1",["spawn"]={"MARINE","objects\\characters\\marine\\marine","objects\\characters\\marine\\ai\\trooper",1,2,111,false,false,false,1}},{["label"]="SPAWN x10",["spawn"]={"MARINE","objects\\characters\\marine\\marine","objects\\characters\\marine\\ai\\trooper",10,2,111,false,false,false,1}}},["MANUAL_FREE_M_112"]={{["label"]="SPAWN x1",["spawn"]={"MARINE","objects\\characters\\marine\\marine","objects\\characters\\marine\\ai\\trooper",1,2,112,false,false,false,1}},{["label"]="SPAWN x10",["spawn"]={"MARINE","objects\\characters\\marine\\marine","objects\\characters\\marine\\ai\\trooper",10,2,112,false,false,false,1}}},["MANUAL_FREE_M_113"]={{["label"]="SPAWN x1",["spawn"]={"MARINE","objects\\characters\\marine\\marine","objects\\characters\\marine\\ai\\trooper",1,2,113,false,false,false,1}},{["label"]="SPAWN x10",["spawn"]={"MARINE","objects\\characters\\marine\\marine","objects\\characters\\marine\\ai\\trooper",10,2,113,false,false,false,1}}},["MANUAL_FREE_M_101"]={{["label"]="SPAWN x1",["spawn"]={"MARINE","objects\\characters\\marine\\marine","objects\\characters\\marine\\ai\\trooper",1,2,101,false,false,false,1}},{["label"]="SPAWN x10",["spawn"]={"MARINE","objects\\characters\\marine\\marine","objects\\characters\\marine\\ai\\trooper",10,2,101,false,false,false,1}}},["MANUAL_FREE_M_109"]={{["label"]="SPAWN x1",["spawn"]={"MARINE","objects\\characters\\marine\\marine","objects\\characters\\marine\\ai\\trooper",1,2,109,false,false,false,1}},{["label"]="SPAWN x10",["spawn"]={"MARINE","objects\\characters\\marine\\marine","objects\\characters\\marine\\ai\\trooper",10,2,109,false,false,false,1}}},["MANUAL_FREE_M_106"]={{["label"]="SPAWN x1",["spawn"]={"MARINE","objects\\characters\\marine\\marine","objects\\characters\\marine\\ai\\trooper",1,2,106,false,false,false,1}},{["label"]="SPAWN x10",["spawn"]={"MARINE","objects\\characters\\marine\\marine","objects\\characters\\marine\\ai\\trooper",10,2,106,false,false,false,1}}},["MANUAL_FREE_M_108"]={{["label"]="SPAWN x1",["spawn"]={"MARINE","objects\\characters\\marine\\marine","objects\\characters\\marine\\ai\\trooper",1,2,108,false,false,false,1}},{["label"]="SPAWN x10",["spawn"]={"MARINE","objects\\characters\\marine\\marine","objects\\characters\\marine\\ai\\trooper",10,2,108,false,false,false,1}}},["MANUAL_FREE_M_104"]={{["label"]="SPAWN x1",["spawn"]={"MARINE","objects\\characters\\marine\\marine","objects\\characters\\marine\\ai\\trooper",1,2,104,false,false,false,1}},{["label"]="SPAWN x10",["spawn"]={"MARINE","objects\\characters\\marine\\marine","objects\\characters\\marine\\ai\\trooper",10,2,104,false,false,false,1}}},["MANUAL_FREE_M_105"]={{["label"]="SPAWN x1",["spawn"]={"MARINE","objects\\characters\\marine\\marine","objects\\characters\\marine\\ai\\trooper",1,2,105,false,false,false,1}},{["label"]="SPAWN x10",["spawn"]={"MARINE","objects\\characters\\marine\\marine","objects\\characters\\marine\\ai\\trooper",10,2,105,false,false,false,1}}},["MANUAL_FREE_M_114"]={{["label"]="SPAWN x1",["spawn"]={"MARINE","objects\\characters\\marine\\marine","objects\\characters\\marine\\ai\\trooper",1,2,114,false,false,false,1}},{["label"]="SPAWN x10",["spawn"]={"MARINE","objects\\characters\\marine\\marine","objects\\characters\\marine\\ai\\trooper",10,2,114,false,false,false,1}}},["MANUAL_FREE_M_115"]={{["label"]="SPAWN x1",["spawn"]={"MARINE","objects\\characters\\marine\\marine","objects\\characters\\marine\\ai\\trooper",1,2,115,false,false,false,1}},{["label"]="SPAWN x10",["spawn"]={"MARINE","objects\\characters\\marine\\marine","objects\\characters\\marine\\ai\\trooper",10,2,115,false,false,false,1}}},["MANUAL_FREE_M_102"]={{["label"]="SPAWN x1",["spawn"]={"MARINE","objects\\characters\\marine\\marine","objects\\characters\\marine\\ai\\trooper",1,2,102,false,false,false,1}},{["label"]="SPAWN x10",["spawn"]={"MARINE","objects\\characters\\marine\\marine","objects\\characters\\marine\\ai\\trooper",10,2,102,false,false,false,1}}},["MANUAL_FREE_M_103"]={{["label"]="SPAWN x1",["spawn"]={"MARINE","objects\\characters\\marine\\marine","objects\\characters\\marine\\ai\\trooper",1,2,103,false,false,false,1}},{["label"]="SPAWN x10",["spawn"]={"MARINE","objects\\characters\\marine\\marine","objects\\characters\\marine\\ai\\trooper",10,2,103,false,false,false,1}}},["MANUAL_FREE_M_110"]={{["label"]="SPAWN x1",["spawn"]={"MARINE","objects\\characters\\marine\\marine","objects\\characters\\marine\\ai\\trooper",1,2,110,false,false,false,1}},{["label"]="SPAWN x10",["spawn"]={"MARINE","objects\\characters\\marine\\marine","objects\\characters\\marine\\ai\\trooper",10,2,110,false,false,false,1}}},["MANUAL_FREE_F"]={{["label"]="MIXED WEAPONS",["page"]="MANUAL_FREE_F_1"},{["label"]="ASSAULT RIFLE",["page"]="MANUAL_FREE_F_100"},{["label"]="BATTLE RIFLE",["page"]="MANUAL_FREE_F_111"},{["label"]="BEAM RIFLE",["page"]="MANUAL_FREE_F_112"},{["label"]="FUEL ROD CANNON",["page"]="MANUAL_FREE_F_113"},{["label"]="MAGNUM",["page"]="MANUAL_FREE_F_101"},{["label"]="NEEDLE RIFLE",["page"]="MANUAL_FREE_F_109"},{["label"]="NEEDLER",["page"]="MANUAL_FREE_F_106"},{["label"]="PLASMA PISTOL",["page"]="MANUAL_FREE_F_108"},{["label"]="PLASMA RIFLE",["page"]="MANUAL_FREE_F_104"},{["label"]="PLASMA RIFLE RED",["page"]="MANUAL_FREE_F_105"},{["label"]="ROCKET LAUNCHER",["page"]="MANUAL_FREE_F_114"},{["label"]="SENTINEL BEAM",["page"]="MANUAL_FREE_F_115"},{["label"]="SHOTGUN",["page"]="MANUAL_FREE_F_102"},{["label"]="SNIPER RIFLE",["page"]="MANUAL_FREE_F_103"},{["label"]="SMG",["page"]="MANUAL_FREE_F_110"}},["MANUAL_FREE_F_1"]={{["label"]="SPAWN x1",["spawn"]={"FEMALE MARINE","objects\\characters\\marine_female\\marine_female","objects\\characters\\marine_female\\ai\\trooper_female",1,2,1,false,false,false,1}},{["label"]="SPAWN x10",["spawn"]={"FEMALE MARINE","objects\\characters\\marine_female\\marine_female","objects\\characters\\marine_female\\ai\\trooper_female",10,2,1,false,false,false,1}}},["MANUAL_FREE_F_100"]={{["label"]="SPAWN x1",["spawn"]={"FEMALE MARINE","objects\\characters\\marine_female\\marine_female","objects\\characters\\marine_female\\ai\\trooper_female",1,2,100,false,false,false,1}},{["label"]="SPAWN x10",["spawn"]={"FEMALE MARINE","objects\\characters\\marine_female\\marine_female","objects\\characters\\marine_female\\ai\\trooper_female",10,2,100,false,false,false,1}}},["MANUAL_FREE_F_111"]={{["label"]="SPAWN x1",["spawn"]={"FEMALE MARINE","objects\\characters\\marine_female\\marine_female","objects\\characters\\marine_female\\ai\\trooper_female",1,2,111,false,false,false,1}},{["label"]="SPAWN x10",["spawn"]={"FEMALE MARINE","objects\\characters\\marine_female\\marine_female","objects\\characters\\marine_female\\ai\\trooper_female",10,2,111,false,false,false,1}}},["MANUAL_FREE_F_112"]={{["label"]="SPAWN x1",["spawn"]={"FEMALE MARINE","objects\\characters\\marine_female\\marine_female","objects\\characters\\marine_female\\ai\\trooper_female",1,2,112,false,false,false,1}},{["label"]="SPAWN x10",["spawn"]={"FEMALE MARINE","objects\\characters\\marine_female\\marine_female","objects\\characters\\marine_female\\ai\\trooper_female",10,2,112,false,false,false,1}}},["MANUAL_FREE_F_113"]={{["label"]="SPAWN x1",["spawn"]={"FEMALE MARINE","objects\\characters\\marine_female\\marine_female","objects\\characters\\marine_female\\ai\\trooper_female",1,2,113,false,false,false,1}},{["label"]="SPAWN x10",["spawn"]={"FEMALE MARINE","objects\\characters\\marine_female\\marine_female","objects\\characters\\marine_female\\ai\\trooper_female",10,2,113,false,false,false,1}}},["MANUAL_FREE_F_101"]={{["label"]="SPAWN x1",["spawn"]={"FEMALE MARINE","objects\\characters\\marine_female\\marine_female","objects\\characters\\marine_female\\ai\\trooper_female",1,2,101,false,false,false,1}},{["label"]="SPAWN x10",["spawn"]={"FEMALE MARINE","objects\\characters\\marine_female\\marine_female","objects\\characters\\marine_female\\ai\\trooper_female",10,2,101,false,false,false,1}}},["MANUAL_FREE_F_109"]={{["label"]="SPAWN x1",["spawn"]={"FEMALE MARINE","objects\\characters\\marine_female\\marine_female","objects\\characters\\marine_female\\ai\\trooper_female",1,2,109,false,false,false,1}},{["label"]="SPAWN x10",["spawn"]={"FEMALE MARINE","objects\\characters\\marine_female\\marine_female","objects\\characters\\marine_female\\ai\\trooper_female",10,2,109,false,false,false,1}}},["MANUAL_FREE_F_106"]={{["label"]="SPAWN x1",["spawn"]={"FEMALE MARINE","objects\\characters\\marine_female\\marine_female","objects\\characters\\marine_female\\ai\\trooper_female",1,2,106,false,false,false,1}},{["label"]="SPAWN x10",["spawn"]={"FEMALE MARINE","objects\\characters\\marine_female\\marine_female","objects\\characters\\marine_female\\ai\\trooper_female",10,2,106,false,false,false,1}}},["MANUAL_FREE_F_108"]={{["label"]="SPAWN x1",["spawn"]={"FEMALE MARINE","objects\\characters\\marine_female\\marine_female","objects\\characters\\marine_female\\ai\\trooper_female",1,2,108,false,false,false,1}},{["label"]="SPAWN x10",["spawn"]={"FEMALE MARINE","objects\\characters\\marine_female\\marine_female","objects\\characters\\marine_female\\ai\\trooper_female",10,2,108,false,false,false,1}}},["MANUAL_FREE_F_104"]={{["label"]="SPAWN x1",["spawn"]={"FEMALE MARINE","objects\\characters\\marine_female\\marine_female","objects\\characters\\marine_female\\ai\\trooper_female",1,2,104,false,false,false,1}},{["label"]="SPAWN x10",["spawn"]={"FEMALE MARINE","objects\\characters\\marine_female\\marine_female","objects\\characters\\marine_female\\ai\\trooper_female",10,2,104,false,false,false,1}}},["MANUAL_FREE_F_105"]={{["label"]="SPAWN x1",["spawn"]={"FEMALE MARINE","objects\\characters\\marine_female\\marine_female","objects\\characters\\marine_female\\ai\\trooper_female",1,2,105,false,false,false,1}},{["label"]="SPAWN x10",["spawn"]={"FEMALE MARINE","objects\\characters\\marine_female\\marine_female","objects\\characters\\marine_female\\ai\\trooper_female",10,2,105,false,false,false,1}}},["MANUAL_FREE_F_114"]={{["label"]="SPAWN x1",["spawn"]={"FEMALE MARINE","objects\\characters\\marine_female\\marine_female","objects\\characters\\marine_female\\ai\\trooper_female",1,2,114,false,false,false,1}},{["label"]="SPAWN x10",["spawn"]={"FEMALE MARINE","objects\\characters\\marine_female\\marine_female","objects\\characters\\marine_female\\ai\\trooper_female",10,2,114,false,false,false,1}}},["MANUAL_FREE_F_115"]={{["label"]="SPAWN x1",["spawn"]={"FEMALE MARINE","objects\\characters\\marine_female\\marine_female","objects\\characters\\marine_female\\ai\\trooper_female",1,2,115,false,false,false,1}},{["label"]="SPAWN x10",["spawn"]={"FEMALE MARINE","objects\\characters\\marine_female\\marine_female","objects\\characters\\marine_female\\ai\\trooper_female",10,2,115,false,false,false,1}}},["MANUAL_FREE_F_102"]={{["label"]="SPAWN x1",["spawn"]={"FEMALE MARINE","objects\\characters\\marine_female\\marine_female","objects\\characters\\marine_female\\ai\\trooper_female",1,2,102,false,false,false,1}},{["label"]="SPAWN x10",["spawn"]={"FEMALE MARINE","objects\\characters\\marine_female\\marine_female","objects\\characters\\marine_female\\ai\\trooper_female",10,2,102,false,false,false,1}}},["MANUAL_FREE_F_103"]={{["label"]="SPAWN x1",["spawn"]={"FEMALE MARINE","objects\\characters\\marine_female\\marine_female","objects\\characters\\marine_female\\ai\\trooper_female",1,2,103,false,false,false,1}},{["label"]="SPAWN x10",["spawn"]={"FEMALE MARINE","objects\\characters\\marine_female\\marine_female","objects\\characters\\marine_female\\ai\\trooper_female",10,2,103,false,false,false,1}}},["MANUAL_FREE_F_110"]={{["label"]="SPAWN x1",["spawn"]={"FEMALE MARINE","objects\\characters\\marine_female\\marine_female","objects\\characters\\marine_female\\ai\\trooper_female",1,2,110,false,false,false,1}},{["label"]="SPAWN x10",["spawn"]={"FEMALE MARINE","objects\\characters\\marine_female\\marine_female","objects\\characters\\marine_female\\ai\\trooper_female",10,2,110,false,false,false,1}}},["MARINE_SEARCH"]={{["label"]="MARINE",["page"]="MANUAL_SEARCH_M"},{["label"]="FEMALE MARINE",["page"]="MANUAL_SEARCH_F"}},["MANUAL_SEARCH_M"]={{["label"]="MIXED WEAPONS",["page"]="MANUAL_SEARCH_M_1"},{["label"]="ASSAULT RIFLE",["page"]="MANUAL_SEARCH_M_100"},{["label"]="BATTLE RIFLE",["page"]="MANUAL_SEARCH_M_111"},{["label"]="BEAM RIFLE",["page"]="MANUAL_SEARCH_M_112"},{["label"]="FUEL ROD CANNON",["page"]="MANUAL_SEARCH_M_113"},{["label"]="MAGNUM",["page"]="MANUAL_SEARCH_M_101"},{["label"]="NEEDLE RIFLE",["page"]="MANUAL_SEARCH_M_109"},{["label"]="NEEDLER",["page"]="MANUAL_SEARCH_M_106"},{["label"]="PLASMA PISTOL",["page"]="MANUAL_SEARCH_M_108"},{["label"]="PLASMA RIFLE",["page"]="MANUAL_SEARCH_M_104"},{["label"]="PLASMA RIFLE RED",["page"]="MANUAL_SEARCH_M_105"},{["label"]="ROCKET LAUNCHER",["page"]="MANUAL_SEARCH_M_114"},{["label"]="SENTINEL BEAM",["page"]="MANUAL_SEARCH_M_115"},{["label"]="SHOTGUN",["page"]="MANUAL_SEARCH_M_102"},{["label"]="SNIPER RIFLE",["page"]="MANUAL_SEARCH_M_103"},{["label"]="SMG",["page"]="MANUAL_SEARCH_M_110"}},["MANUAL_SEARCH_M_1"]={{["label"]="SPAWN x1",["spawn"]={"MARINE","objects\\characters\\marine\\marine","objects\\characters\\marine\\ai\\trooper",1,2,1,false,false,false,2}},{["label"]="SPAWN x10",["spawn"]={"MARINE","objects\\characters\\marine\\marine","objects\\characters\\marine\\ai\\trooper",10,2,1,false,false,false,2}}},["MANUAL_SEARCH_M_100"]={{["label"]="SPAWN x1",["spawn"]={"MARINE","objects\\characters\\marine\\marine","objects\\characters\\marine\\ai\\trooper",1,2,100,false,false,false,2}},{["label"]="SPAWN x10",["spawn"]={"MARINE","objects\\characters\\marine\\marine","objects\\characters\\marine\\ai\\trooper",10,2,100,false,false,false,2}}},["MANUAL_SEARCH_M_111"]={{["label"]="SPAWN x1",["spawn"]={"MARINE","objects\\characters\\marine\\marine","objects\\characters\\marine\\ai\\trooper",1,2,111,false,false,false,2}},{["label"]="SPAWN x10",["spawn"]={"MARINE","objects\\characters\\marine\\marine","objects\\characters\\marine\\ai\\trooper",10,2,111,false,false,false,2}}},["MANUAL_SEARCH_M_112"]={{["label"]="SPAWN x1",["spawn"]={"MARINE","objects\\characters\\marine\\marine","objects\\characters\\marine\\ai\\trooper",1,2,112,false,false,false,2}},{["label"]="SPAWN x10",["spawn"]={"MARINE","objects\\characters\\marine\\marine","objects\\characters\\marine\\ai\\trooper",10,2,112,false,false,false,2}}},["MANUAL_SEARCH_M_113"]={{["label"]="SPAWN x1",["spawn"]={"MARINE","objects\\characters\\marine\\marine","objects\\characters\\marine\\ai\\trooper",1,2,113,false,false,false,2}},{["label"]="SPAWN x10",["spawn"]={"MARINE","objects\\characters\\marine\\marine","objects\\characters\\marine\\ai\\trooper",10,2,113,false,false,false,2}}},["MANUAL_SEARCH_M_101"]={{["label"]="SPAWN x1",["spawn"]={"MARINE","objects\\characters\\marine\\marine","objects\\characters\\marine\\ai\\trooper",1,2,101,false,false,false,2}},{["label"]="SPAWN x10",["spawn"]={"MARINE","objects\\characters\\marine\\marine","objects\\characters\\marine\\ai\\trooper",10,2,101,false,false,false,2}}},["MANUAL_SEARCH_M_109"]={{["label"]="SPAWN x1",["spawn"]={"MARINE","objects\\characters\\marine\\marine","objects\\characters\\marine\\ai\\trooper",1,2,109,false,false,false,2}},{["label"]="SPAWN x10",["spawn"]={"MARINE","objects\\characters\\marine\\marine","objects\\characters\\marine\\ai\\trooper",10,2,109,false,false,false,2}}},["MANUAL_SEARCH_M_106"]={{["label"]="SPAWN x1",["spawn"]={"MARINE","objects\\characters\\marine\\marine","objects\\characters\\marine\\ai\\trooper",1,2,106,false,false,false,2}},{["label"]="SPAWN x10",["spawn"]={"MARINE","objects\\characters\\marine\\marine","objects\\characters\\marine\\ai\\trooper",10,2,106,false,false,false,2}}},["MANUAL_SEARCH_M_108"]={{["label"]="SPAWN x1",["spawn"]={"MARINE","objects\\characters\\marine\\marine","objects\\characters\\marine\\ai\\trooper",1,2,108,false,false,false,2}},{["label"]="SPAWN x10",["spawn"]={"MARINE","objects\\characters\\marine\\marine","objects\\characters\\marine\\ai\\trooper",10,2,108,false,false,false,2}}},["MANUAL_SEARCH_M_104"]={{["label"]="SPAWN x1",["spawn"]={"MARINE","objects\\characters\\marine\\marine","objects\\characters\\marine\\ai\\trooper",1,2,104,false,false,false,2}},{["label"]="SPAWN x10",["spawn"]={"MARINE","objects\\characters\\marine\\marine","objects\\characters\\marine\\ai\\trooper",10,2,104,false,false,false,2}}},["MANUAL_SEARCH_M_105"]={{["label"]="SPAWN x1",["spawn"]={"MARINE","objects\\characters\\marine\\marine","objects\\characters\\marine\\ai\\trooper",1,2,105,false,false,false,2}},{["label"]="SPAWN x10",["spawn"]={"MARINE","objects\\characters\\marine\\marine","objects\\characters\\marine\\ai\\trooper",10,2,105,false,false,false,2}}},["MANUAL_SEARCH_M_114"]={{["label"]="SPAWN x1",["spawn"]={"MARINE","objects\\characters\\marine\\marine","objects\\characters\\marine\\ai\\trooper",1,2,114,false,false,false,2}},{["label"]="SPAWN x10",["spawn"]={"MARINE","objects\\characters\\marine\\marine","objects\\characters\\marine\\ai\\trooper",10,2,114,false,false,false,2}}},["MANUAL_SEARCH_M_115"]={{["label"]="SPAWN x1",["spawn"]={"MARINE","objects\\characters\\marine\\marine","objects\\characters\\marine\\ai\\trooper",1,2,115,false,false,false,2}},{["label"]="SPAWN x10",["spawn"]={"MARINE","objects\\characters\\marine\\marine","objects\\characters\\marine\\ai\\trooper",10,2,115,false,false,false,2}}},["MANUAL_SEARCH_M_102"]={{["label"]="SPAWN x1",["spawn"]={"MARINE","objects\\characters\\marine\\marine","objects\\characters\\marine\\ai\\trooper",1,2,102,false,false,false,2}},{["label"]="SPAWN x10",["spawn"]={"MARINE","objects\\characters\\marine\\marine","objects\\characters\\marine\\ai\\trooper",10,2,102,false,false,false,2}}},["MANUAL_SEARCH_M_103"]={{["label"]="SPAWN x1",["spawn"]={"MARINE","objects\\characters\\marine\\marine","objects\\characters\\marine\\ai\\trooper",1,2,103,false,false,false,2}},{["label"]="SPAWN x10",["spawn"]={"MARINE","objects\\characters\\marine\\marine","objects\\characters\\marine\\ai\\trooper",10,2,103,false,false,false,2}}},["MANUAL_SEARCH_M_110"]={{["label"]="SPAWN x1",["spawn"]={"MARINE","objects\\characters\\marine\\marine","objects\\characters\\marine\\ai\\trooper",1,2,110,false,false,false,2}},{["label"]="SPAWN x10",["spawn"]={"MARINE","objects\\characters\\marine\\marine","objects\\characters\\marine\\ai\\trooper",10,2,110,false,false,false,2}}},["MANUAL_SEARCH_F"]={{["label"]="MIXED WEAPONS",["page"]="MANUAL_SEARCH_F_1"},{["label"]="ASSAULT RIFLE",["page"]="MANUAL_SEARCH_F_100"},{["label"]="BATTLE RIFLE",["page"]="MANUAL_SEARCH_F_111"},{["label"]="BEAM RIFLE",["page"]="MANUAL_SEARCH_F_112"},{["label"]="FUEL ROD CANNON",["page"]="MANUAL_SEARCH_F_113"},{["label"]="MAGNUM",["page"]="MANUAL_SEARCH_F_101"},{["label"]="NEEDLE RIFLE",["page"]="MANUAL_SEARCH_F_109"},{["label"]="NEEDLER",["page"]="MANUAL_SEARCH_F_106"},{["label"]="PLASMA PISTOL",["page"]="MANUAL_SEARCH_F_108"},{["label"]="PLASMA RIFLE",["page"]="MANUAL_SEARCH_F_104"},{["label"]="PLASMA RIFLE RED",["page"]="MANUAL_SEARCH_F_105"},{["label"]="ROCKET LAUNCHER",["page"]="MANUAL_SEARCH_F_114"},{["label"]="SENTINEL BEAM",["page"]="MANUAL_SEARCH_F_115"},{["label"]="SHOTGUN",["page"]="MANUAL_SEARCH_F_102"},{["label"]="SNIPER RIFLE",["page"]="MANUAL_SEARCH_F_103"},{["label"]="SMG",["page"]="MANUAL_SEARCH_F_110"}},["MANUAL_SEARCH_F_1"]={{["label"]="SPAWN x1",["spawn"]={"FEMALE MARINE","objects\\characters\\marine_female\\marine_female","objects\\characters\\marine_female\\ai\\trooper_female",1,2,1,false,false,false,2}},{["label"]="SPAWN x10",["spawn"]={"FEMALE MARINE","objects\\characters\\marine_female\\marine_female","objects\\characters\\marine_female\\ai\\trooper_female",10,2,1,false,false,false,2}}},["MANUAL_SEARCH_F_100"]={{["label"]="SPAWN x1",["spawn"]={"FEMALE MARINE","objects\\characters\\marine_female\\marine_female","objects\\characters\\marine_female\\ai\\trooper_female",1,2,100,false,false,false,2}},{["label"]="SPAWN x10",["spawn"]={"FEMALE MARINE","objects\\characters\\marine_female\\marine_female","objects\\characters\\marine_female\\ai\\trooper_female",10,2,100,false,false,false,2}}},["MANUAL_SEARCH_F_111"]={{["label"]="SPAWN x1",["spawn"]={"FEMALE MARINE","objects\\characters\\marine_female\\marine_female","objects\\characters\\marine_female\\ai\\trooper_female",1,2,111,false,false,false,2}},{["label"]="SPAWN x10",["spawn"]={"FEMALE MARINE","objects\\characters\\marine_female\\marine_female","objects\\characters\\marine_female\\ai\\trooper_female",10,2,111,false,false,false,2}}},["MANUAL_SEARCH_F_112"]={{["label"]="SPAWN x1",["spawn"]={"FEMALE MARINE","objects\\characters\\marine_female\\marine_female","objects\\characters\\marine_female\\ai\\trooper_female",1,2,112,false,false,false,2}},{["label"]="SPAWN x10",["spawn"]={"FEMALE MARINE","objects\\characters\\marine_female\\marine_female","objects\\characters\\marine_female\\ai\\trooper_female",10,2,112,false,false,false,2}}},["MANUAL_SEARCH_F_113"]={{["label"]="SPAWN x1",["spawn"]={"FEMALE MARINE","objects\\characters\\marine_female\\marine_female","objects\\characters\\marine_female\\ai\\trooper_female",1,2,113,false,false,false,2}},{["label"]="SPAWN x10",["spawn"]={"FEMALE MARINE","objects\\characters\\marine_female\\marine_female","objects\\characters\\marine_female\\ai\\trooper_female",10,2,113,false,false,false,2}}},["MANUAL_SEARCH_F_101"]={{["label"]="SPAWN x1",["spawn"]={"FEMALE MARINE","objects\\characters\\marine_female\\marine_female","objects\\characters\\marine_female\\ai\\trooper_female",1,2,101,false,false,false,2}},{["label"]="SPAWN x10",["spawn"]={"FEMALE MARINE","objects\\characters\\marine_female\\marine_female","objects\\characters\\marine_female\\ai\\trooper_female",10,2,101,false,false,false,2}}},["MANUAL_SEARCH_F_109"]={{["label"]="SPAWN x1",["spawn"]={"FEMALE MARINE","objects\\characters\\marine_female\\marine_female","objects\\characters\\marine_female\\ai\\trooper_female",1,2,109,false,false,false,2}},{["label"]="SPAWN x10",["spawn"]={"FEMALE MARINE","objects\\characters\\marine_female\\marine_female","objects\\characters\\marine_female\\ai\\trooper_female",10,2,109,false,false,false,2}}},["MANUAL_SEARCH_F_106"]={{["label"]="SPAWN x1",["spawn"]={"FEMALE MARINE","objects\\characters\\marine_female\\marine_female","objects\\characters\\marine_female\\ai\\trooper_female",1,2,106,false,false,false,2}},{["label"]="SPAWN x10",["spawn"]={"FEMALE MARINE","objects\\characters\\marine_female\\marine_female","objects\\characters\\marine_female\\ai\\trooper_female",10,2,106,false,false,false,2}}},["MANUAL_SEARCH_F_108"]={{["label"]="SPAWN x1",["spawn"]={"FEMALE MARINE","objects\\characters\\marine_female\\marine_female","objects\\characters\\marine_female\\ai\\trooper_female",1,2,108,false,false,false,2}},{["label"]="SPAWN x10",["spawn"]={"FEMALE MARINE","objects\\characters\\marine_female\\marine_female","objects\\characters\\marine_female\\ai\\trooper_female",10,2,108,false,false,false,2}}},["MANUAL_SEARCH_F_104"]={{["label"]="SPAWN x1",["spawn"]={"FEMALE MARINE","objects\\characters\\marine_female\\marine_female","objects\\characters\\marine_female\\ai\\trooper_female",1,2,104,false,false,false,2}},{["label"]="SPAWN x10",["spawn"]={"FEMALE MARINE","objects\\characters\\marine_female\\marine_female","objects\\characters\\marine_female\\ai\\trooper_female",10,2,104,false,false,false,2}}},["MANUAL_SEARCH_F_105"]={{["label"]="SPAWN x1",["spawn"]={"FEMALE MARINE","objects\\characters\\marine_female\\marine_female","objects\\characters\\marine_female\\ai\\trooper_female",1,2,105,false,false,false,2}},{["label"]="SPAWN x10",["spawn"]={"FEMALE MARINE","objects\\characters\\marine_female\\marine_female","objects\\characters\\marine_female\\ai\\trooper_female",10,2,105,false,false,false,2}}},["MANUAL_SEARCH_F_114"]={{["label"]="SPAWN x1",["spawn"]={"FEMALE MARINE","objects\\characters\\marine_female\\marine_female","objects\\characters\\marine_female\\ai\\trooper_female",1,2,114,false,false,false,2}},{["label"]="SPAWN x10",["spawn"]={"FEMALE MARINE","objects\\characters\\marine_female\\marine_female","objects\\characters\\marine_female\\ai\\trooper_female",10,2,114,false,false,false,2}}},["MANUAL_SEARCH_F_115"]={{["label"]="SPAWN x1",["spawn"]={"FEMALE MARINE","objects\\characters\\marine_female\\marine_female","objects\\characters\\marine_female\\ai\\trooper_female",1,2,115,false,false,false,2}},{["label"]="SPAWN x10",["spawn"]={"FEMALE MARINE","objects\\characters\\marine_female\\marine_female","objects\\characters\\marine_female\\ai\\trooper_female",10,2,115,false,false,false,2}}},["MANUAL_SEARCH_F_102"]={{["label"]="SPAWN x1",["spawn"]={"FEMALE MARINE","objects\\characters\\marine_female\\marine_female","objects\\characters\\marine_female\\ai\\trooper_female",1,2,102,false,false,false,2}},{["label"]="SPAWN x10",["spawn"]={"FEMALE MARINE","objects\\characters\\marine_female\\marine_female","objects\\characters\\marine_female\\ai\\trooper_female",10,2,102,false,false,false,2}}},["MANUAL_SEARCH_F_103"]={{["label"]="SPAWN x1",["spawn"]={"FEMALE MARINE","objects\\characters\\marine_female\\marine_female","objects\\characters\\marine_female\\ai\\trooper_female",1,2,103,false,false,false,2}},{["label"]="SPAWN x10",["spawn"]={"FEMALE MARINE","objects\\characters\\marine_female\\marine_female","objects\\characters\\marine_female\\ai\\trooper_female",10,2,103,false,false,false,2}}},["MANUAL_SEARCH_F_110"]={{["label"]="SPAWN x1",["spawn"]={"FEMALE MARINE","objects\\characters\\marine_female\\marine_female","objects\\characters\\marine_female\\ai\\trooper_female",1,2,110,false,false,false,2}},{["label"]="SPAWN x10",["spawn"]={"FEMALE MARINE","objects\\characters\\marine_female\\marine_female","objects\\characters\\marine_female\\ai\\trooper_female",10,2,110,false,false,false,2}}}}
local aiPageTitles={["AI_0W109"]="NEEDLE RIFLE",["AI_40"]="GRUNT",["AI_SENTINELS"]="SPAWN SENTINELS",["BATTLE_AI"]="SPAWN AI",["AI_34_R7_W0"]="BONUS SPEC OPS - MIXED WEAPONS",["AI_34_R6_W2"]="BONUS MAJOR - RED PLASMA RIFLE",["AI_34_R5"]="ELITE - BONUS MINOR",["AI_FLOOD"]="SPAWN FLOOD",["AI_40W3"]="MIXED WEAPONS",["AI_34_R7"]="ELITE - BONUS SPEC OPS",["AI_15W6"]="MIXED WEAPONS",["AI_48"]="HUNTER",["AI_34_R4_W1"]="GENERAL - PLASMA RIFLE",["AI_34W104"]="PLASMA RIFLE",["AI_34_R5_W4"]="BONUS MINOR - ENERGY SWORD",["AI_34_R8_W0"]="BONUS ZEALOT - MIXED WEAPONS",["AI_10W104"]="PLASMA RIFLE",["AI_34W105"]="RED PLASMA RIFLE",["AI_5W106"]="NEEDLER",["AI_34_R0_W1"]="MINOR - PLASMA RIFLE",["AI_40W106"]="NEEDLER",["AI_34_R6_W3"]="BONUS MAJOR - NEEDLER",["AI_44W106"]="NEEDLER",["AI_34_R6"]="ELITE - BONUS MAJOR",["AI_0W6"]="MIXED WEAPONS",["AI_34_R3_W2"]="SPEC OPS - RED PLASMA RIFLE",["AI_34_R5_W0"]="BONUS MINOR - MIXED WEAPONS",["AI_44"]="JACKAL",["AI_34_R2_W3"]="ULTRA - NEEDLER",["AI_34_R4_W0"]="GENERAL - MIXED WEAPONS",["AI_34_R5_W2"]="BONUS MINOR - RED PLASMA RIFLE",["AI_34_R1_W3"]="OFFICER - NEEDLER",["AI_34_R8_W1"]="BONUS ZEALOT - PLASMA RIFLE",["AI_34_R1"]="ELITE - OFFICER",["AI_60W0"]="DEFAULT EQUIPMENT",["AI_51W0"]="DEFAULT EQUIPMENT",["AI_34_R3_W4"]="SPEC OPS - ENERGY SWORD",["AI_48W0"]="DEFAULT EQUIPMENT",["AI_34_R4_W2"]="GENERAL - RED PLASMA RIFLE",["AI_34_R1_W4"]="OFFICER - ENERGY SWORD",["AI_5"]="BRUTE CAPTAIN",["AI_34_R1_W0"]="OFFICER - MIXED WEAPONS",["AI_57W0"]="DEFAULT EQUIPMENT",["AI_62"]="SENTINEL MAJOR",["AI_0W106"]="NEEDLER",["AI_34_R5_W3"]="BONUS MINOR - NEEDLER",["AI_51"]="FLOOD HUMAN",["AI_MARINES"]="SPAWN MARINES",["AI_0"]="BRUTE",["AI_34_R4"]="ELITE - GENERAL",["AI_34_R8_W2"]="BONUS ZEALOT - RED PLASMA RIFLE",["AI_34_R4_W4"]="GENERAL - ENERGY SWORD",["AI_34_R6_W0"]="BONUS MAJOR - MIXED WEAPONS",["AI_53W0"]="DEFAULT EQUIPMENT",["AI_34_R2"]="ELITE - ULTRA",["AI_55W0"]="DEFAULT EQUIPMENT",["AI_34_R2_W0"]="ULTRA - MIXED WEAPONS",["AI_34_R1_W1"]="OFFICER - PLASMA RIFLE",["AI_34_R5_W1"]="BONUS MINOR - PLASMA RIFLE",["AI_10"]="BRUTE CHIEFTAIN - WEAPON",["AI_34_R0_W2"]="MINOR - RED PLASMA RIFLE",["AI_34_R3_W1"]="SPEC OPS - PLASMA RIFLE",["MARINE_FOLLOW"]="FOLLOW",["AI_34_R6_W4"]="BONUS MAJOR - ENERGY SWORD",["AI_34_MIXED"]="ELITE - MIXED RANKS",["AI_15W104"]="PLASMA RIFLE",["AI_5W109"]="NEEDLE RIFLE",["MARINE_FREE"]="FREE COMBAT",["AI_40W108"]="PLASMA PISTOL",["AI_5W104"]="PLASMA RIFLE",["AI_34_R8_W4"]="BONUS ZEALOT - ENERGY SWORD",["AI_55"]="FLOOD CARRIER",["AI_5W6"]="MIXED WEAPONS",["AI_44W108"]="PLASMA PISTOL",["AI_34_R2_W1"]="ULTRA - PLASMA RIFLE",["AI_34_R6_W1"]="BONUS MAJOR - PLASMA RIFLE",["AI_57"]="FLOOD TANK",["AI_34_R8_W3"]="BONUS ZEALOT - NEEDLER",["AI_10W109"]="NEEDLE RIFLE",["AI_34_R0_W4"]="MINOR - ENERGY SWORD",["AI_34_R1_W2"]="OFFICER - RED PLASMA RIFLE",["AI_62W0"]="DEFAULT EQUIPMENT",["AI_60"]="SENTINEL",["AI_34_R3"]="ELITE - SPEC OPS",["AI_BRUTES"]="BRUTES",["AI_34_R0_W3"]="MINOR - NEEDLER",["AI_15W106"]="NEEDLER",["AI_34_R7_W1"]="BONUS SPEC OPS - PLASMA RIFLE",["AI_34W2"]="MIXED WEAPONS",["AI_15"]="BRUTE CHIEFTAIN - ARMOR",["AI_34_R4_W3"]="GENERAL - NEEDLER",["AI_34_R2_W2"]="ULTRA - RED PLASMA RIFLE",["AI_34_R8"]="ELITE - BONUS ZEALOT",["AI_34_R7_W4"]="BONUS SPEC OPS - ENERGY SWORD",["AI_15W109"]="NEEDLE RIFLE",["AI_10W106"]="NEEDLER",["AI_10W6"]="MIXED WEAPONS",["AI_34_R2_W4"]="ULTRA - ENERGY SWORD",["AI_44W4"]="MIXED WEAPONS",["AI_34_R3_W3"]="SPEC OPS - NEEDLER",["AI_34W107"]="ENERGY SWORD",["AI_34W106"]="NEEDLER",["AI_0W104"]="PLASMA RIFLE",["AI_34_R7_W3"]="BONUS SPEC OPS - NEEDLER",["AI_34_R7_W2"]="BONUS SPEC OPS - RED PLASMA RIFLE",["AI_53"]="FLOOD ELITE",["AI_34"]="ELITE RANK",["AI_34_R0"]="ELITE - MINOR",["AI_COVENANT"]="SPAWN COVENANT",["AI_34_R0_W0"]="MINOR - MIXED WEAPONS",["AI_34_R3_W0"]="SPEC OPS - MIXED WEAPONS",["MANUAL_FOLLOW_M"]="MARINE",["MANUAL_FOLLOW_M_1"]="MARINE - MIXED WEAPONS",["MANUAL_FOLLOW_M_100"]="MARINE - ASSAULT RIFLE",["MANUAL_FOLLOW_M_111"]="MARINE - BATTLE RIFLE",["MANUAL_FOLLOW_M_112"]="MARINE - BEAM RIFLE",["MANUAL_FOLLOW_M_113"]="MARINE - FUEL ROD CANNON",["MANUAL_FOLLOW_M_101"]="MARINE - MAGNUM",["MANUAL_FOLLOW_M_109"]="MARINE - NEEDLE RIFLE",["MANUAL_FOLLOW_M_106"]="MARINE - NEEDLER",["MANUAL_FOLLOW_M_108"]="MARINE - PLASMA PISTOL",["MANUAL_FOLLOW_M_104"]="MARINE - PLASMA RIFLE",["MANUAL_FOLLOW_M_105"]="MARINE - PLASMA RIFLE RED",["MANUAL_FOLLOW_M_114"]="MARINE - ROCKET LAUNCHER",["MANUAL_FOLLOW_M_115"]="MARINE - SENTINEL BEAM",["MANUAL_FOLLOW_M_102"]="MARINE - SHOTGUN",["MANUAL_FOLLOW_M_103"]="MARINE - SNIPER RIFLE",["MANUAL_FOLLOW_M_110"]="MARINE - SMG",["MANUAL_FOLLOW_F"]="FEMALE MARINE",["MANUAL_FOLLOW_F_1"]="FEMALE MARINE - MIXED WEAPONS",["MANUAL_FOLLOW_F_100"]="FEMALE MARINE - ASSAULT RIFLE",["MANUAL_FOLLOW_F_111"]="FEMALE MARINE - BATTLE RIFLE",["MANUAL_FOLLOW_F_112"]="FEMALE MARINE - BEAM RIFLE",["MANUAL_FOLLOW_F_113"]="FEMALE MARINE - FUEL ROD CANNON",["MANUAL_FOLLOW_F_101"]="FEMALE MARINE - MAGNUM",["MANUAL_FOLLOW_F_109"]="FEMALE MARINE - NEEDLE RIFLE",["MANUAL_FOLLOW_F_106"]="FEMALE MARINE - NEEDLER",["MANUAL_FOLLOW_F_108"]="FEMALE MARINE - PLASMA PISTOL",["MANUAL_FOLLOW_F_104"]="FEMALE MARINE - PLASMA RIFLE",["MANUAL_FOLLOW_F_105"]="FEMALE MARINE - PLASMA RIFLE RED",["MANUAL_FOLLOW_F_114"]="FEMALE MARINE - ROCKET LAUNCHER",["MANUAL_FOLLOW_F_115"]="FEMALE MARINE - SENTINEL BEAM",["MANUAL_FOLLOW_F_102"]="FEMALE MARINE - SHOTGUN",["MANUAL_FOLLOW_F_103"]="FEMALE MARINE - SNIPER RIFLE",["MANUAL_FOLLOW_F_110"]="FEMALE MARINE - SMG",["MANUAL_FREE_M"]="MARINE",["MANUAL_FREE_M_1"]="MARINE - MIXED WEAPONS",["MANUAL_FREE_M_100"]="MARINE - ASSAULT RIFLE",["MANUAL_FREE_M_111"]="MARINE - BATTLE RIFLE",["MANUAL_FREE_M_112"]="MARINE - BEAM RIFLE",["MANUAL_FREE_M_113"]="MARINE - FUEL ROD CANNON",["MANUAL_FREE_M_101"]="MARINE - MAGNUM",["MANUAL_FREE_M_109"]="MARINE - NEEDLE RIFLE",["MANUAL_FREE_M_106"]="MARINE - NEEDLER",["MANUAL_FREE_M_108"]="MARINE - PLASMA PISTOL",["MANUAL_FREE_M_104"]="MARINE - PLASMA RIFLE",["MANUAL_FREE_M_105"]="MARINE - PLASMA RIFLE RED",["MANUAL_FREE_M_114"]="MARINE - ROCKET LAUNCHER",["MANUAL_FREE_M_115"]="MARINE - SENTINEL BEAM",["MANUAL_FREE_M_102"]="MARINE - SHOTGUN",["MANUAL_FREE_M_103"]="MARINE - SNIPER RIFLE",["MANUAL_FREE_M_110"]="MARINE - SMG",["MANUAL_FREE_F"]="FEMALE MARINE",["MANUAL_FREE_F_1"]="FEMALE MARINE - MIXED WEAPONS",["MANUAL_FREE_F_100"]="FEMALE MARINE - ASSAULT RIFLE",["MANUAL_FREE_F_111"]="FEMALE MARINE - BATTLE RIFLE",["MANUAL_FREE_F_112"]="FEMALE MARINE - BEAM RIFLE",["MANUAL_FREE_F_113"]="FEMALE MARINE - FUEL ROD CANNON",["MANUAL_FREE_F_101"]="FEMALE MARINE - MAGNUM",["MANUAL_FREE_F_109"]="FEMALE MARINE - NEEDLE RIFLE",["MANUAL_FREE_F_106"]="FEMALE MARINE - NEEDLER",["MANUAL_FREE_F_108"]="FEMALE MARINE - PLASMA PISTOL",["MANUAL_FREE_F_104"]="FEMALE MARINE - PLASMA RIFLE",["MANUAL_FREE_F_105"]="FEMALE MARINE - PLASMA RIFLE RED",["MANUAL_FREE_F_114"]="FEMALE MARINE - ROCKET LAUNCHER",["MANUAL_FREE_F_115"]="FEMALE MARINE - SENTINEL BEAM",["MANUAL_FREE_F_102"]="FEMALE MARINE - SHOTGUN",["MANUAL_FREE_F_103"]="FEMALE MARINE - SNIPER RIFLE",["MANUAL_FREE_F_110"]="FEMALE MARINE - SMG",["MARINE_SEARCH"]="JOIN LOCAL SQUAD ORDERS",["MANUAL_SEARCH_M"]="MARINE",["MANUAL_SEARCH_M_1"]="MARINE - MIXED WEAPONS",["MANUAL_SEARCH_M_100"]="MARINE - ASSAULT RIFLE",["MANUAL_SEARCH_M_111"]="MARINE - BATTLE RIFLE",["MANUAL_SEARCH_M_112"]="MARINE - BEAM RIFLE",["MANUAL_SEARCH_M_113"]="MARINE - FUEL ROD CANNON",["MANUAL_SEARCH_M_101"]="MARINE - MAGNUM",["MANUAL_SEARCH_M_109"]="MARINE - NEEDLE RIFLE",["MANUAL_SEARCH_M_106"]="MARINE - NEEDLER",["MANUAL_SEARCH_M_108"]="MARINE - PLASMA PISTOL",["MANUAL_SEARCH_M_104"]="MARINE - PLASMA RIFLE",["MANUAL_SEARCH_M_105"]="MARINE - PLASMA RIFLE RED",["MANUAL_SEARCH_M_114"]="MARINE - ROCKET LAUNCHER",["MANUAL_SEARCH_M_115"]="MARINE - SENTINEL BEAM",["MANUAL_SEARCH_M_102"]="MARINE - SHOTGUN",["MANUAL_SEARCH_M_103"]="MARINE - SNIPER RIFLE",["MANUAL_SEARCH_M_110"]="MARINE - SMG",["MANUAL_SEARCH_F"]="FEMALE MARINE",["MANUAL_SEARCH_F_1"]="FEMALE MARINE - MIXED WEAPONS",["MANUAL_SEARCH_F_100"]="FEMALE MARINE - ASSAULT RIFLE",["MANUAL_SEARCH_F_111"]="FEMALE MARINE - BATTLE RIFLE",["MANUAL_SEARCH_F_112"]="FEMALE MARINE - BEAM RIFLE",["MANUAL_SEARCH_F_113"]="FEMALE MARINE - FUEL ROD CANNON",["MANUAL_SEARCH_F_101"]="FEMALE MARINE - MAGNUM",["MANUAL_SEARCH_F_109"]="FEMALE MARINE - NEEDLE RIFLE",["MANUAL_SEARCH_F_106"]="FEMALE MARINE - NEEDLER",["MANUAL_SEARCH_F_108"]="FEMALE MARINE - PLASMA PISTOL",["MANUAL_SEARCH_F_104"]="FEMALE MARINE - PLASMA RIFLE",["MANUAL_SEARCH_F_105"]="FEMALE MARINE - PLASMA RIFLE RED",["MANUAL_SEARCH_F_114"]="FEMALE MARINE - ROCKET LAUNCHER",["MANUAL_SEARCH_F_115"]="FEMALE MARINE - SENTINEL BEAM",["MANUAL_SEARCH_F_102"]="FEMALE MARINE - SHOTGUN",["MANUAL_SEARCH_F_103"]="FEMALE MARINE - SNIPER RIFLE",["MANUAL_SEARCH_F_110"]="FEMALE MARINE - SMG"}

local function pageItems()
    if aiPages[state.page] then
        local choices={}
        for _,entry in ipairs(aiPages[state.page]) do
            local selected=entry
            choices[#choices+1]={label=entry.label,page=entry.page,close=false,
                action=entry.spawn and function() spawnActiveSquad(table.unpack(selected.spawn)) end or nil}
        end
        return choices
    end
    if state.page=='BATTLE_WEAPONS' or state.page=='BATTLE_VEHICLES' then
        local wanted=state.page=='BATTLE_WEAPONS' and 'weapon' or 'vehicle'
        local choices={}
        for _,item in ipairs(catalog) do
            if item.kind==wanted then local chosen=item;choices[#choices+1]={label=item.label,close=false,action=function() spawnBattleItem(chosen) end} end
        end
        return choices
    end
    return {
        {label=(escort.enabled and '[X] ' or '[ ] ')..'KEEP 3 MARINES',action=toggleEscort},
        {label=(johnsonEscort.enabled and '[X] ' or '[ ] ')..'KEEP SGT. JOHNSON',action=toggleJohnson},
        {label=(observerEnabled and '[X] ' or '[ ] ')..'BATTLE OBSERVER - AI IGNORES YOU',action=toggleObserver},
        {label='SPAWN AI',page='BATTLE_AI'},
        {label='SPAWN VEHICLES',page='BATTLE_VEHICLES'},
        {label='SPAWN WEAPONS',page='BATTLE_WEAPONS'},
        {label='PAUSE ALL AI - SET UP BATTLE',action=function() battleAIControl(0) end},
        {label='RESUME ALL AI - START BATTLE',action=function() battleAIControl(1) end},
    }
end

local VISIBLE = 11
local MAX_UI_LINES = 24

local function visibleItems()
    local all = pageItems()
    if state.page == 'ACCUEIL' then return all, 1, #all end
    local maxOffset = math.max(0, #all - VISIBLE)
    state.pageOffset = math.max(0, math.min(state.pageOffset, maxOffset))
    local shown = {}
    for index = state.pageOffset + 1, math.min(#all, state.pageOffset + VISIBLE) do shown[#shown + 1] = all[index] end
    return shown, state.pageOffset + 1, math.min(#all, state.pageOffset + VISIBLE), #all
end

local function removeWidgetSafe(widget)
    if not valid(widget) then return end
    pcall(function() widget:RemoveFromParent() end)
end

local function closeMenu(reason)
    local oldWidget = state.widget
    state.open = false
    state.uiGeneration = state.uiGeneration + 1
    state.redrawScheduled = false
    state.redrawQueued = false
    state.redrawBusy = false
    state.widget = nil
    state.statusLine = nil
    state.lineWidgets = {}
    state.lineSlots = {}
    state.lines = {}
    syncControllerMenuState()
    removeWidgetSafe(oldWidget)
    ExecuteInGameThreadWithDelay(80, function() removeWidgetSafe(oldWidget) end)
    print('[HaloSandbox] Menu ferme' .. (reason and (' (' .. tostring(reason) .. ')') or '') .. '.\n')
end

-- UEngine::LoadMap reconstruit le monde, les pointeurs UObjects et les tables
-- Blam de la mission. On invalide donc toutes les continuations Lua encore en
-- attente avant le chargement. La commande native deja partie peut finir, mais
-- son callback est ignore grace a lifecycleGeneration; les commandes qui
-- attendaient dans la file sont supprimees.
local function beginMissionTransition(reason)
    observerEnabled=false;observerDatum=nil;observerBusy=false
    escort.enabled=false;escort.epoch=escort.epoch+1;escort.busy=false;escort.context=nil;escort.slots={{},{},{}}
    johnsonEscort.enabled=false;johnsonEscort.epoch=johnsonEscort.epoch+1;johnsonEscort.busy=false;johnsonEscort.context=nil;johnsonEscort.slots={{},{},{}}
    testHistory('mission transition: ' .. tostring(reason))
    -- Coupe les gardes natifs avant que les datums de l'ancienne mission ne
    -- puissent etre reutilises par le moteur pendant le chargement.
    if type(nativeQuiesce) == 'function' then pcall(nativeQuiesce) end
    state.lifecycleGeneration = state.lifecycleGeneration + 1
    state.entryGeneration = state.entryGeneration + 1
    state.spawnGeneration = state.spawnGeneration + 1
    state.aiScenarioGeneration = state.aiScenarioGeneration + 1
    state.pelicanSeatPatchGeneration = state.pelicanSeatPatchGeneration + 1
    nativeQueue = {}

    state.lastSpawn = nil
    state.lastActiveAI = nil
    state.lastScenarioAI = nil
    state.deathlessAppliedDatum = nil
    state.bottomlessAppliedDatum = nil
    state.noclipAppliedDatum = nil
    state.followWatchActive = false
    state.followWatchGeneration = state.followWatchGeneration + 1
    state.followWatchEmptySamples = 0
    state.followWatchLastTracked = nil
    state.aircraftGunnerState = {}
    state.preloadedAssets = {}
    state.pelicanSeatPatchTag = nil
    state.pelicanBoostTag = nil
    state.preloadReady = false
    state.preloadInProgress = false
    state.preloadRetryScheduled = false
    state.preloadLoaded = 0
    state.preloadFailed = 0
    state.inputTimes = {}
    state.lastToggleAt = -1000.0
    state.coop = { active = false, sessionRunning = false, authority = nil, players = 0 }

    if state.open or valid(state.widget) then
        closeMenu(reason or 'changement de mission')
    else
        state.open = false
        state.widget = nil
        state.statusLine = nil
        state.lineWidgets = {}
        state.lineSlots = {}
        state.lines = {}
        syncControllerMenuState()
    end
    state.status = 'CHARGEMENT DE LA MISSION...'
    print(string.format('[HaloSandbox] Cycle mission %d: ancien etat invalide (%s), file native videe.\n',
        state.lifecycleGeneration, tostring(reason or 'transition')))
end

local function createPersistentMenu()
    local world, controller = getLocalWorld(), getLocalPlayerController()
    local source = FindFirstOf('WBP_FrameBorder_Effect_C')
    local widgetLibrary = StaticFindObject('/Script/UMG.Default__WidgetBlueprintLibrary')
    local verticalClass = StaticFindObject('/Script/UMG.VerticalBox')
    local borderClass = StaticFindObject('/Script/UMG.Border')
    local textClass = StaticFindObject('/Script/UMG.TextBlock')
    if not valid(world) or not valid(controller) or not valid(source) or not valid(widgetLibrary)
        or not valid(verticalClass) or not valid(borderClass) or not valid(textClass) then
        return false, 'classes UMG indisponibles'
    end

    local menu = widgetLibrary:Create(controller, source:GetClass(), controller)
    if not valid(menu) then menu = widgetLibrary:Create(world, source:GetClass(), nil) end
    if not valid(menu) or not valid(menu.Frame_Border) then return false, 'widget principal invalide' end

    local tree = menu.WidgetTree
    local list = StaticConstructObject(verticalClass, tree)
    local background = StaticConstructObject(borderClass, tree)
    if not valid(list) or not valid(background) then
        removeWidgetSafe(menu)
        return false, 'conteneur UMG invalide'
    end
    background:SetPadding({ Left = 38.0, Top = 30.0, Right = 38.0, Bottom = 30.0 })
    background:SetBrushColor({ R = 0.012, G = 0.055, B = 0.075, A = 0.94 })
    background:SetContent(list)
    menu.Frame_Border:SetPadding({ Left = 5.0, Top = 5.0, Right = 5.0, Bottom = 5.0 })
    menu.Frame_Border:SetBrushColor({ R = 0.08, G = 0.62, B = 0.72, A = 0.98 })
    menu.Frame_Border:SetContent(background)

    local lineWidgets, lineSlots = {}, {}
    for index = 1, MAX_UI_LINES do
        local line = StaticConstructObject(textClass, tree)
        if not valid(line) then
            removeWidgetSafe(menu)
            return false, 'ligne UMG ' .. tostring(index) .. ' invalide'
        end
        line:SetText(FText(''))
        line.Font.Size = 14
        line:SetShadowOffset({ X = 1.5, Y = 1.5 })
        line:SetShadowColorAndOpacity({ R = 0, G = 0, B = 0, A = 0.9 })
        local slot = list:AddChildToVerticalBox(line)
        lineWidgets[index], lineSlots[index] = line, slot
    end

    local viewport = { X = 1920, Y = 1080 }
    pcall(function() viewport = widgetLibrary:GetViewportSize(world) end)
    menu:SetDesiredSizeInViewport({ X = 700.0, Y = 900.0 })
    menu:SetPositionInViewport({ X = math.max(35.0, (viewport.X or 1920) - 745.0), Y = 70.0 }, false)
    menu:SetAlignmentInViewport({ X = 0.0, Y = 0.0 })
    menu:AddToViewport(7500)

    state.widget = menu
    state.lineWidgets = lineWidgets
    state.lineSlots = lineSlots
    return true
end

local function menuLineSpecs()
    local specs = {}
    local function add(text, size, selected, top, status)
        specs[#specs + 1] = {
            text = tostring(text or ''), size = size or 14,
            selected = selected == true, top = top or 3, status = status == true,
        }
    end

    add('Starky Spartan Spawner', 31, true, 4)
    local coopLabel = 'LOCAL SESSION'
    if state.coop.active then
        if state.coop.netMode == 2 or (state.coop.netMode == nil and state.coop.authority == true) then
            coopLabel = 'CO-OP HOST'
        elseif state.coop.netMode == 3 or state.coop.authority == false then
            coopLabel = 'CO-OP CLIENT'
        else
            coopLabel = 'CO-OP ROLE UNKNOWN'
        end
    end
    add((({BATTLE_BRUTES='BRUTES',BATTLE_FLOOD='FLOOD',BATTLE_SENTINELS='SENTINELS',BATTLE_AI='AI GROUPS',BATTLE_WEAPONS='WEAPONS',BATTLE_VEHICLES='VEHICLES'})[state.page] or aiPageTitles[state.page] or UI_PAGE_NAMES[state.page] or state.page) .. '  //  ' .. coopLabel, 16, false, 0)
    add('', 7, false, 0)

    local shown, first, last, total = visibleItems()
    if state.page ~= 'ACCUEIL' then
        add(state.selection == 1 and '>  BACK' or '   BACK', 21, state.selection == 1, 3)
    end
    local base = state.page == 'ACCUEIL' and 0 or 1
    for index, entry in ipairs(shown) do
        local selected = state.selection == index + base
        add((selected and '>  ' or '   ') .. uiText(entry.label), 22, selected, 4)
    end
    add('', 6, false, 0)
    if total and total > VISIBLE then add(string.format('%d-%d / %d', first, last, total), 15, false, 1) end
    add(uiText(state.status), 16, true, 2, true)
    add('ARROWS: NAVIGATE   ENTER: SELECT   BACKSPACE: BACK', 14, false, 2)
    add('F4 / B: CLOSE   F2: EMERGENCY CLOSE', 14, false, 1)



    return specs
end

local function redrawInternal()
    if not state.open then return end
    if not valid(state.widget) then
        local created, createError = createPersistentMenu()
        if not created then
            print('[HaloSandbox] UI refusee: ' .. tostring(createError) .. '\n')
            closeMenu('creation UMG impossible')
            return
        end
    end

    local specs = menuLineSpecs()
    state.statusLine = nil
    for index = 1, MAX_UI_LINES do
        local line, slot = state.lineWidgets[index], state.lineSlots[index]
        if not valid(line) then
            closeMenu('ligne UMG perdue')
            return
        end
        local spec = specs[index]
        if spec then
            line:SetText(FText(spec.text))
            line.Font.Size = spec.size
            line.ColorAndOpacity.SpecifiedColor = spec.selected
                and { R = 0.10, G = 0.92, B = 1.0, A = 1.0 }
                or { R = 0.76, G = 0.91, B = 0.94, A = 1.0 }
            if valid(slot) then
                slot:SetPadding({ Left = spec.selected and 8.0 or 14.0, Top = spec.top, Right = 6.0, Bottom = 3.0 })
            end
            if spec.status then state.statusLine = line end
        else
            line:SetText(FText(''))
            line.Font.Size = 1
            line.ColorAndOpacity.SpecifiedColor = { R = 0, G = 0, B = 0, A = 0 }
            if valid(slot) then slot:SetPadding({ Left = 0.0, Top = 0.0, Right = 0.0, Bottom = 0.0 }) end
        end
    end
end

redraw = function()
    if not state.open then return end
    if state.redrawBusy then state.redrawQueued = true return end
    if state.redrawScheduled then return end
    state.redrawScheduled = true
    local requestedGeneration = state.uiGeneration
    ExecuteInGameThreadWithDelay(25, function()
        state.redrawScheduled = false
        if not state.open or requestedGeneration ~= state.uiGeneration then return end
        state.redrawBusy = true
        local ok, err = xpcall(redrawInternal, debug.traceback)
        state.redrawBusy = false
        if not ok then
            print('[HaloSandbox] ERREUR UI: ' .. tostring(err) .. '\n')
            closeMenu('exception UI')
            return
        end
        if state.redrawQueued and state.open then
            state.redrawQueued = false
            redraw()
        end
    end)
end

local function openMenu()
    -- Le menu est local a chaque PC et reste volontairement libre pour tous
    -- les joueurs de la coop. Aucune regle de couronne/hote ne le bloque.
    local controller = getLocalPlayerController()
    if not valid(controller) or not valid(controller.Pawn) then
        print('[HaloSandbox] Menu refuse: controleur ou pawn invalide.\n')
        state.status = 'OUVRE LE MENU UNE FOIS DANS UNE MISSION'
        return
    end
    print('[HaloSandbox] ACCESS menu coop libre accepte.\n')
    state.uiGeneration = state.uiGeneration + 1
    state.open = true
    print('[HaloSandbox] Menu ouvert par entree locale.\n')
    local allowed, coop, reason = hostFeaturesAllowed()
    if coop and coop.active and not allowed then
        state.status = reason or 'MODE CLIENT: SPAWNS DESACTIVES'
    elseif coop and coop.active then
        state.status = 'MODE HOTE AUTORITAIRE - SPAWNS ACTIFS'
    end
    syncControllerMenuState()
    state.navStack = {}
    state.page = 'ACCUEIL'
    state.selection = 1
    state.pageOffset = 0
    redraw()
end

local function toggleMenu()
    local now = os.clock()
    if now - state.lastToggleAt < 0.25 then return end
    state.lastToggleAt = now
    if state.open then
        print('[HaloSandbox] F4/RB ferme le menu.\n')
        closeMenu('toggle')
    else
        openMenu()
    end
end

local function moveSelection(delta)
    if not state.open then return end
    local shown = visibleItems()
    local count = #shown + (state.page == 'ACCUEIL' and 0 or 1)
    state.selection = state.selection + delta
    if state.selection < 1 then
        if state.page ~= 'ACCUEIL' and state.pageOffset > 0 then state.pageOffset = state.pageOffset - 1; state.selection = 2
        else state.selection = count end
    elseif state.selection > count then
        local total = #pageItems()
        if state.page ~= 'ACCUEIL' and state.pageOffset + #shown < total then state.pageOffset = state.pageOffset + 1; state.selection = count
        else state.selection = 1 end
    end
    redraw()
end

local function back()
    if not state.open then return end
    if state.page == 'ACCUEIL' then closeMenu() return end
    local previous=table.remove(state.navStack or {})
    state.page, state.selection, state.pageOffset = previous and previous.page or 'ACCUEIL', previous and previous.selection or 1, previous and previous.offset or 0
    redraw()
end

local function activate()
    if not state.open then return end
    if state.page ~= 'ACCUEIL' and state.selection == 1 then back() return end
    local shown = visibleItems()
    local index = state.selection - (state.page == 'ACCUEIL' and 0 or 1)
    local entry = shown[index]
    if not entry then return end
    if entry.page then
        state.navStack=state.navStack or {}
        state.navStack[#state.navStack+1]={page=state.page,selection=state.selection,offset=state.pageOffset}
        state.page, state.selection, state.pageOffset = entry.page, 1, 0
        redraw()
    elseif entry.item then
        closeMenu()
        spawnItem(entry.item)
    elseif entry.action then
        if entry.close then closeMenu() end
        local ok, err = xpcall(entry.action, debug.traceback)
        if not ok then
            print('[HaloSandbox] ERREUR ACTION MENU: ' .. tostring(err) .. '\n')
            setStatus('ACTION ANNULEE - VOIR UE4SS.LOG')
            closeMenu()
            return
        end
        if state.open then redraw() end
    end
end

local function safeInput(name, callback, debounceSeconds)
    return function()
        local now = os.clock()
        local delay = tonumber(debounceSeconds) or 0.08
        local previous = state.inputTimes[name] or -1000.0
        if now - previous < delay then return end
        state.inputTimes[name] = now
        local ok, err = xpcall(callback, debug.traceback)
        if not ok then
            print('[HaloSandbox] ERREUR ENTREE ' .. tostring(name) .. ': ' .. tostring(err) .. '\n')
            closeMenu('exception entree')
        end
    end
end

local function bindKey(key, name, callback, debounceSeconds)
    local ok, err = pcall(RegisterKeyBind, key, safeInput(name, callback, debounceSeconds))
    if not ok then print('[HaloSandbox] Touche ' .. tostring(name) .. ' indisponible: ' .. tostring(err) .. '\n') end
end

bindKey(Key.F4, 'F4', function()
    print('[HaloSandbox] F4 recu par UE4SS.\n')
    toggleMenu()
end)
bindKey(Key.F2, 'F2-SECOURS', function()
    print('[HaloSandbox] F2 fermeture secours.\n')
    closeMenu()
end)
bindKey(Key.UP_ARROW, 'HAUT', function() moveSelection(-1) end, 0.10)
bindKey(Key.DOWN_ARROW, 'BAS', function() moveSelection(1) end, 0.10)
bindKey(Key.RETURN, 'ENTREE', activate, 0.18)
bindKey(Key.F12, 'MANETTE-A', activate, 0.18)
bindKey(Key.BACKSPACE, 'RETOUR', back, 0.15)
-- Ne pas enregistrer ECHAP : UE4SS consomme la touche meme si le rappel ne
-- fait rien lorsque notre menu est ferme. Le jeu doit donc conserver ECHAP
-- pour ouvrir son propre menu pause et permettre les redemarrages de mission.
-- Diagnostic strictement en lecture seule : aucun deplacement, aucune ecriture
-- dans le scenario ou le checkpoint. Il permet de comparer le meme acteur
-- avant/apres que le joueur s'eloigne normalement.

syncControllerMenuState()
if type(controllerStart) == 'function' then
    local ok, err = pcall(controllerStart)
    if ok then print('[HaloSandbox] Controle XInput pret.\n')
    else print('[HaloSandbox] XInput indisponible: ' .. tostring(err) .. '\n') end
else
    print('[HaloSandbox] XInput indisponible: ' .. tostring(controllerLoadError) .. '\n')
end

local function preloadCoreAssets()
    do return end
    -- En coop, les tags ajoutes dynamiquement doivent etre charges dans le meme
    -- ordre sur tous les PC. Sinon un datum recu du serveur peut designer autre
    -- chose chez le client. On prepare seulement les vrais vehicules pilotables
    -- du catalogue (jamais les 431 assets), un par un pour eviter les gels.
    local generation = state.lifecycleGeneration
    if state.preloadInProgress or state.preloadReady then return end
    -- Le chargeur Blam n'est valide qu'une fois la simulation de campagne
    -- initialisee. Depuis le menu principal il peut ne pas avoir de table de
    -- tags courante : attendre un joueur Halo reel avant toute commande native.
    if not playerContext(false) then
        state.preloadReady = false
        if not state.preloadRetryScheduled then
            state.preloadRetryScheduled = true
            ExecuteInGameThreadWithDelay(1000, function()
                if generation ~= state.lifecycleGeneration then return end
                state.preloadRetryScheduled = false
                preloadCoreAssets()
            end)
        end
        return
    end
    state.preloadInProgress = true
    state.preloadRetryScheduled = false
    local vehicles = {}
    state.preloadReady = false
    state.preloadLoaded, state.preloadFailed = 0, 0
    for _, item in ipairs(catalog) do
        if item.kind == 'vehicle' and not item.attachment and item.preferred == true
            and not item.path:find('tuning_fork', 1, true) then
            table.insert(vehicles, item)
        end
    end

    local index = 1
    local function prepareNext()
        if generation ~= state.lifecycleGeneration then return end
        local item = vehicles[index]
        if not item then
            state.preloadInProgress = false
            state.preloadReady = true
            print(string.format('[HaloSandbox] Catalogue vehicules coop resident: %d charge(s), %d refuse(s), ordre identique.\n',
                state.preloadLoaded, state.preloadFailed))
            patchPelicanSeatsLocal(nil, true)
            return
        end
        index = index + 1
        ExecuteInGameThread(function()
            if generation ~= state.lifecycleGeneration then return end
            local okAsset, asset = pcall(LoadAsset, item.asset)
            if not okAsset or not valid(asset) then
                state.preloadFailed = state.preloadFailed + 1
                print(string.format('[HaloSandbox] Prechargement vehicule refuse (asset): %s\n', item.label))
                ExecuteInGameThreadWithDelay(60, prepareNext)
                return
            end
            state.preloadedAssets[item.asset] = asset
            runNative('findtag ' .. item.group .. ' ' .. item.path, function(found, findError)
                if generation ~= state.lifecycleGeneration then return end
                local datum = found and tonumber(found:match('datum=([%-%d]+)'))
                if datum and not findError then
                    state.preloadLoaded = state.preloadLoaded + 1
                    ExecuteInGameThreadWithDelay(60, prepareNext)
                    return
                end
                runNative('loadtag ' .. item.group .. ' ' .. item.path, function(loaded, loadError)
                    if generation ~= state.lifecycleGeneration then return end
                    local loadedDatum = loaded and tonumber(loaded:match('datum=([%-%d]+)'))
                    if loadedDatum and not loadError then
                        state.preloadLoaded = state.preloadLoaded + 1
                        print(string.format('[HaloSandbox] Tag vehicule coop ajoute: %s datum=%d\n',
                            item.label, loadedDatum))
                    else
                        state.preloadFailed = state.preloadFailed + 1
                        print(string.format('[HaloSandbox] Tag vehicule coop refuse: %s detail=%s\n',
                            item.label, tostring(loaded or loadError or found or findError)))
                    end
                    ExecuteInGameThreadWithDelay(60, prepareNext)
                end)
            end)
        end)
    end
    prepareNext()
end

ExecuteInGameThreadWithDelay(1200, preloadCoreAssets)

-- Hooks de cycle de vie fournis directement par UE4SS. Ils couvrent aussi le
-- bouton "Redemarrer la mission", qui recharge la carte sans redemarrer le
-- processus ni le script Lua.
local loadMapHooksOk, loadMapHooksError = pcall(function()
    RegisterLoadMapPreHook(function()
        beginMissionTransition('chargement de carte')
    end)
    RegisterLoadMapPostHook(function()
        local generation = state.lifecycleGeneration
        ExecuteInGameThreadWithDelay(1200, function()
            if generation ~= state.lifecycleGeneration then return end
            preloadCoreAssets()
        end)
        ExecuteInGameThreadWithDelay(2200, function()
            if generation ~= state.lifecycleGeneration then return end
            local context = playerContext(false)
            if context and state.preloadReady then patchPelicanSeatsLocal(nil, true) end
        end)
        print(string.format('[HaloSandbox] Cycle mission %d: nouvelle carte chargee.\n', generation))
    end)
end)
if loadMapHooksOk then
    print('[HaloSandbox] Hooks LoadMap de securite actifs.\n')
else
    print('[HaloSandbox] Hooks LoadMap indisponibles: ' .. tostring(loadMapHooksError) .. '\n')
end

-- Halo Campaign Evolved peut remplacer la simulation Blam dans un monde UE
-- persistant. Dans ce cas UEngine::LoadMap ne s'execute pas, mais le GameState
-- de la nouvelle mission est initialise. Ce second hook couvre ce chemin ainsi
-- que l'option "Redemarrer la mission" du menu pause.
local initGameStateHookOk, initGameStateHookError = pcall(function()
    RegisterInitGameStatePostHook(function()
        beginMissionTransition('initialisation GameState')
        local generation = state.lifecycleGeneration
        ExecuteInGameThreadWithDelay(1200, function()
            if generation ~= state.lifecycleGeneration then return end
            preloadCoreAssets()
        end)
        print(string.format('[HaloSandbox] Cycle mission %d: GameState initialise.\n', generation))
    end)
end)
if initGameStateHookOk then
    print('[HaloSandbox] Hook InitGameState de secours actif.\n')
else
    print('[HaloSandbox] Hook InitGameState indisponible: ' .. tostring(initGameStateHookError) .. '\n')
end

-- Réapplique périodiquement le correctif local. Les tags peuvent être
-- rechargés lors d'un changement de mission/checkpoint, ce qui annule les
-- modifications mémoire sur un client sans redémarrer UE4SS.
local function maintainPelicanSeatPatch()
    do return end
    local context = playerContext(false)
    if context and state.preloadReady then patchPelicanSeatsLocal(nil, true) end
    ExecuteInGameThreadWithDelay(12000, maintainPelicanSeatPatch)
end
ExecuteInGameThreadWithDelay(2200, maintainPelicanSeatPatch)

-- La maintenance lourde des pouvoirs vit dans la DLL et n'ecrit que lorsqu'un
-- compteur change. Cette impulsion Lua ne rattache les options qu'apres un
-- nouveau datum joueur; elle ne tourne plus quatre fois par seconde.
local function maintainPlayerPowers()
    do return end
    if not nativeBusy and (state.bottomless or state.deathless or state.noclip) then
        local allowed = hostFeaturesAllowed()
        if allowed then
            local context = playerContext(false)
            if context then
                if state.noclip and state.noclipAppliedDatum ~= context.datum then
                    runNative(string.format('noclip %d 1 %d', context.datum,
                        state.deathless and 1 or 0), function(_, noclipError)
                        if not noclipError then
                            state.noclipAppliedDatum = context.datum
                            print(string.format('[HaloSandbox] Noclip reapplique au datum joueur %d.\n', context.datum))
                        end
                    end)
                else
                    local bottomlessMode = state.bottomless
                        and state.bottomlessAppliedDatum ~= context.datum and 1 or -1
                    if state.deathless or bottomlessMode ~= -1 then
                        local proofMode = (state.deathless or state.noclip) and 1 or -1
                        runNative(string.format('player_powers %d %d %d', context.datum,
                            proofMode, bottomlessMode), function(_, powersError)
                            if powersError then
                                if tostring(powersError):find('unit_missing', 1, true) then
                                    state.deathlessAppliedDatum = nil
                                    state.bottomlessAppliedDatum = nil
                                end
                                return
                            end
                            state.deathlessAppliedDatum = state.deathless and context.datum or nil
                            if bottomlessMode == 1 then
                                state.bottomlessAppliedDatum = context.datum
                            end
                        end)
                    end
                end
            end
        end
    end
    ExecuteInGameThreadWithDelay(1500, maintainPlayerPowers)
end
ExecuteInGameThreadWithDelay(2750, maintainPlayerPowers)

-- Le pont direct entretient uniquement les limites de zone du nouveau module.
-- Il ne passe ni par le fichier de commandes ni par le worker reserve a 250 ms.
local function maintainNoclipBridge()
    do return end
    if state.noclip and not state.open and not nativeBusy then
        if type(nativeNoclipBridge) == 'function' then
            local ok, bridgeError = pcall(nativeNoclipBridge)
            if not ok and not state.noclipBridgeErrorLogged then
                state.noclipBridgeErrorLogged = true
                print('[HaloSandbox] Pont noclip direct refuse: ' .. tostring(bridgeError) .. '\n')
            elseif ok then
                state.noclipBridgeErrorLogged = false
            end
        elseif not state.noclipBridgeErrorLogged then
            state.noclipBridgeErrorLogged = true
            print('[HaloSandbox] Pont noclip direct indisponible: ' .. tostring(nativeNoclipBridgeError) .. '\n')
        end
    end
    ExecuteInGameThreadWithDelay(500, maintainNoclipBridge)
end
ExecuteInGameThreadWithDelay(3250, maintainNoclipBridge)

-- Les allies crees par le mod appartiennent bien a la fireteam du joueur,
-- mais certains BSP etroits ne fournissent aucun chemin de navigation entre
-- deux secteurs. Le garde natif n'agit qu'apres quatre mesures immobiles a
-- plus de 12 m, ignore les passagers de vehicule et rapproche alors l'allie.
local function maintainSpawnedAIFollow()
    do return end
    if state.followWatchActive and not nativeBusy then
        local context = playerContext(false)
        if context then
            local watchGeneration = state.followWatchGeneration
            runNative('ai_follow_watch', function(result, err)
                if watchGeneration ~= state.followWatchGeneration then return end
                if not err and result then
                    local tracked = tonumber(tostring(result):match('tracked=(%d+)'))
                    local invalidated = tonumber(tostring(result):match('invalidated=(%d+)')) or 0
                    if tracked and tracked > 0 then
                        state.followWatchEmptySamples = 0
                    elseif tracked == 0 then
                        state.followWatchEmptySamples = state.followWatchEmptySamples + 1
                        if state.followWatchEmptySamples >= 6 then state.followWatchActive = false end
                    end
                    if tracked ~= state.followWatchLastTracked then
                        print(string.format('[HaloSandbox] Suivi IA actif: allies=%s invalides=%d.\n',
                            tostring(tracked or '?'), invalidated))
                        state.followWatchLastTracked = tracked
                    end
                    local exitRequested = tonumber(tostring(result):match('exit_requested=(%d+)')) or 0
                    local exitCompleted = tonumber(tostring(result):match('exit_completed=(%d+)')) or 0
                    if exitRequested > 0 or exitCompleted > 0 then
                        print(string.format('[HaloSandbox] Vehicule quitte: sorties IA demandees=%d terminees=%d.\n',
                            exitRequested, exitCompleted))
                    end
                    local recovered = tonumber(tostring(result):match('recovered=(%d+)')) or 0
                    if recovered > 0 then
                        print(string.format('[HaloSandbox] Navigation IA: %d allie(s) degage(s) d un obstacle.\n', recovered))
                        setStatus(string.format('IA DEGAGEE D UN OBSTACLE: %d', recovered))
                    end
                end
            end)
        end
    end
    ExecuteInGameThreadWithDelay(1500, maintainSpawnedAIFollow)
end
ExecuteInGameThreadWithDelay(3000, maintainSpawnedAIFollow)

runNative('probe', function(result, err)
    if err or not result or not result:find('protocol=70', 1, true) then
        setStatus('BUILD HALO NON COMPATIBLE - MOD BLOQUE')
    else
        setStatus('PRET - F4 OUVRE / FERME LE MENU')

    end
end)

print('[HaloSandbox] ' .. VERSION .. ' charge. Experimental AI test; runtime validation pending.\n')
