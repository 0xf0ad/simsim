-to build this projct on linux run ./buils.sh
-to build for windows run the cmake script (edit it if the minGW toolchain isnt installed)

( make sure  to clone with --recurse-submodules flag )

dependencies:
	- included with this repo:
		- glad
		- stb_image
	- included as git submodules:
		- ImGui
		- ImPlot
	- should be installed to build:
		- cln
		- GiNaC
		- glfw3

known bugs:
	removing the ground component trigers UB
		work around: remove every link to any ground once a ground component is removec
			nested bug: phantom links
