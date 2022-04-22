# 1.0.0

- [x] Bind to references by 0x0 reference form ID (once per script)
- [x] Test that scripts are still attached after a save game
- [x] Support finding forms from other mods
- [x] Bind by calling Papyrus function to Bind a script to an Object
- [x] Remove '*'
- [x] Skyrim SE + VR + AE
- [x] Release on Nexus

# 1.1.0

- [x] On first location load (aka coc) IF NOT A NEW GAME then load ALL THE THINGS
- [x] Support `;` and `#` and `//` comments in the AutoBinding files (strip them out, incl on the same line `Script 0x123 # foo`
- [x] Oh shit. Whoops. Only 1 script can register itself for a Base Form!
- [x] Fill properties via `ScriptName:SomeProperty 0x123 [plugin.esp]`
- [x] Autobind based on Keyword
- [x] Autobind based on FormList
- [x] Document the Papyrus functions
- [x] Record a new demo!
- [x] Fix: when the script doesn't exist, it blows up
- [x] Double check: BindScript.ToForm and .ToEditorID don't blow up if None/not found or non-existent script!
- [x] Papyrus function to get a Form by an Editor ID
- [x] Release Update on Nexus

# 1.1.1

- [x] Ignore files that don't end in .txt
- [x] Can provide a *partial* Editor ID (`*foo`, `foo*`, `*foo*`, `*foo*bar*`, `/^foo.+bar$/`)
- [x] Update log for script bound to include the hex form ID

# 1.2.0

Project Stuff:
- [x] CMake cleaned up and working for all Skyrim versions WITHOUT a local vcpkg! Or with just one!
- [x] Compiled OK in VSCode for all 3 Skyrim Versions
- [x] .bat files for compiling any version

Features:
- [x] Process every reference in the game on game load
  - [x] Capture Main Menu visit or leave
- [x] Papyrus function which will process every form in the game for script attachment
- [x] Papyrus function which will process every object near the player or an object or a point in space
- [x] .ini config for searching nearby on a regular basis

Config:
- [x] Add .ini file support
  - [x] .ini - Can toggle console logs on and off
  - [x] .ini - Can disable on load object reference search

Logs:
- [x] Add a .log file specifically for no-esp (rewrite with each game rerun)

Other:
- [x] Make test suite run plz k thx
- [x] Parse the examples added, plz k thx, like make them go!
- [x] Hook it up to Skyrim
- [x] Properties can be set for all types!
  - [x] String
  - [x] Int
  - [x] Float
  - [x] Bool
  - [x] Single Form
    - [x] Using Editor ID
    - [x] Using Form ID
    - [x] Using Form ID and plugin name
  - [x] Array of String
  - [x] Array of Int
  - [x] Array of Float
  - [x] Array of Bool
  - [x] Array of forms
    - [x] Using multiple Editor IDs
    - [x] Using Editor ID matcher (gets all matches)
    - [x] Using Form ID
    - [x] Using Form ID and plugin name
- [x] Can provide the name of a form type, e.g. `BOOK`
  - [x] Can parse form types given human readable names
  - [x] Can attach script to everything of a given form type
- [x] Can provide form type AND editor ID matcher
- [x] Can provide Form NAME matcher
- [x] Can provide Form NAME matcher with form type

## Soon:
- [ ] Document configurable folder output for .dll and scripts
- [ ] Refactor System into files that make sense!

## Later:
- [ ] Skyrim LE
