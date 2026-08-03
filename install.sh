read -p "This installer will install files that are part of an fcitx5 addon into /usr/share/fcitx5/, /usr/lib/fcitx5/ and ~/.config/unixkey.json"
git clone https://github.com/Schlafhase/UnixKey
cd UnixKey
cmake -S . -B build -DCMAKE_INSTALL_PREFIX=/usr -DCMAKE_BUILD_TYPE=Release
cmake --build build
sudo cmake --install build
cp src/unixkey.json ~/.config/unixkey.json
cd ..
rm -rf UnixKey
