workspace "slider"
  location "build"
	configurations { "debug", "release" }

project "slider"
	kind "ConsoleApp"
	language "C"
	location "build"
	targetdir "bin/%{cfg.buildcfg}"
	systemversion "latest"
	cdialect "C99"
	debugargs { "-d", "%{wks.location}/../demo.txt", "%{wks.location}/../demo.pdf" }

	files { "lib/*.h", "lib/*.c", "*.h", "*.c" }

	filter "configurations:debug"
		defines { "DEBUG" }
		symbols "On"

	filter "configurations:release"
		defines { "NDEBUG" }
		optimize "On"
