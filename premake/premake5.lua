workspace("file-sorter-by-date")
    configurations { "Debug", "Release" }
    architecture "x64"
    defines { "WIN32", "_CONSOLE" }
    language "C++"
    location("../build/")
    startproject "file-sorter-by-date"

filter "configurations:Debug"
    defines { "_DEBUG" }
    optimize "Off"
    symbols "On"

filter "configurations:Release"
    defines { "NDEBUG" }
    optimize "Full"

include("projects/file-sorter-by-date")


