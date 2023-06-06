find . -iname "*.cpp" -print -exec sh -c 'mv {} "$(dirname {})/ronin_$(basename {})"' \;
