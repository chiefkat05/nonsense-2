clear

echo "$(tput setaf 3)
	building application for windows

[----------------------------------------------------------------]
"
echo "$(tput setaf 1)killing existing processes:"
pkill -f -e -c game.exe

echo "
"

set -e

echo "
[----------------------------------------------------------------]
$(tput setaf 4)

compiling and linking:
"
x86_64-w64-mingw32-gcc ./*.c -o ./windows/game.exe -O3 -Wall -lglfw3 -lopengl32 -lgdi32 -lglew32
echo "
[----------------------------------------------------------------]
$(tput setaf 2)
	success"

echo "
	building complete, running executable
[----------------------------------------------------------------]
$(tput setaf 7)"

wine ./windows/game.exe
