generate compile_commands.json with:

    bear -- make

to build run:

    make && xfce4-panel -r

to debug:

    PANEL_DEBUG=xfce-prayer-times
    xfce4-panel -q && make && xfce4-panel 

to make xfce see the plugin you need to symlink the build files to these directories:

    sudo ln -s <dir>/xfce-prayer-times/prayer-times-plugin.desktop /usr/share/xfce4/panel/plugins/prayer-times-plugin.desktop 
    sudo ln -s <dir>/xfce-prayer-times/build/libprayer-times-plugin.so /usr/lib/xfce4/panel/plugins/libprayer-times-plugin.so 

<dir> is the directory you cloned the repo in
