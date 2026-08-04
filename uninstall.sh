sudo rm /usr/lib/fcitx5/unixkey.so
sudo rm /usr/share/fcitx5/addon/unixkey.conf
sudo rm /usr/share/fcitx5/inputmethod/unixkey.conf

read -p "Remove the config file as well? [y/n]: " -n 1 -r
echo
if [[ $REPLY =~ ^[Yy]$ ]]
then
  rm ~/.config/unixkey.json
fi
