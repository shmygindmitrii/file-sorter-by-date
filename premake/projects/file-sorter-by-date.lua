project("file-sorter-by-date")
    kind("ConsoleApp")
    language "C++"
    cppdialect "C++17"
    targetname("tests")
    files({
        "../../source/file-sorter-by-date/src/sorter.cpp"
    })
    includedirs({
        "../../source/file-sorter-by-date/inc/" 
    })
    location("../../build/file-sorter-by-date")
    targetdir("../../build/file-sorter-by-date/bin")
    objdir("../../build/file-sorter-by-date/obj")
    exceptionhandling("On")
    warnings "Extra"
    flags { 
        "FatalCompileWarnings" 
    }


