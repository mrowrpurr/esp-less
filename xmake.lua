set_languages("c++23")

add_rules("mode.debug", "mode.release")

add_repositories("SkyrimScripting https://github.com/SkyrimScripting/Packages.git")

add_requires("skyrim-commonlib", "simpleini")

target("no-esp")
  add_files("src/plugin.cpp")
  add_includedirs("include")
  add_packages("skyrim-commonlib", "simpleini")
  add_rules("@skyrim-commonlib/plugin", {
      name = "no-esp",
      author = "Mrowr Purr",
      email = "mrowr.purr@gmail.com",
      mods_folder = os.getenv("SKYRIM_SCRIPTING_MOD_FOLDERS")
  })
