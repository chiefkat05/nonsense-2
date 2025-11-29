while sleep 1 ; do find . -name '*.c' -o -name '*.h' | entr -s 'sh ./build.sh &' ; done
