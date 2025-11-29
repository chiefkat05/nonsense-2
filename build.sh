clear

echo "$(tput setaf 3)
	building application

[----------------------------------------------------------------]
"
echo "$(tput setaf 1)killing existing processes:"
pkill -f -e -c game

echo "
"

set -e

echo "
[----------------------------------------------------------------]
$(tput setaf 4)

compiling and linking:
"
gcc ./*.c -o game -O3 -Wall `pkg-config --cflags --libs glfw3 glew` -lm -lpng
echo "
[----------------------------------------------------------------]
$(tput setaf 2)
	success"

echo "
	building complete, running executable
[----------------------------------------------------------------]
$(tput setaf 7)"

./game
