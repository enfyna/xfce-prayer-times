<p align="center">
  <img src="https://github.com/grassmunk/Chicago95/blob/bdf5cf36a16102aaac297f3de887c601c2b1146f/Icons/Chicago95-tux/actions/scalable/call-start.svg" width=100>
</p>
<h1 align="center">
  xfce-prayer-times
</h1>
<p align="center">
    <img src=https://img.shields.io/badge/XFCE-%232284F2.svg?style=for-the-badge&logo=xfce&logoColor=white>
    <img src=https://img.shields.io/badge/GTK-7FA719.svg?style=for-the-badge&logo=GTK&logoColor=white>
    <img src=https://img.shields.io/badge/c-%2300599C.svg?style=for-the-badge&logo=c&logoColor=white>
</p>
<br><br>
<p align="center">
  Islamic prayer times panel plugin for XFCE.
</p>
<br>


# Installation

1. Clone the repo.

        git clone https://github.com/enfyna/xfce-prayer-times
        cd xfce-prayer-times

2. Build the project.
> [!NOTE]
> If you have missing dependencies and cant find them open an issue.

        make


3. Install system-wide.
   
    To make XFCE see the plugin you need to symlink the build files to these directories:

        sudo ln -s `dir`/xfce-prayer-times/prayer-times-plugin.desktop /usr/share/xfce4/panel/plugins/prayer-times-plugin.desktop 
        sudo ln -s `dir`/xfce-prayer-times/build/libprayer-times-plugin.so /usr/lib/xfce4/panel/plugins/libprayer-times-plugin.so 

    `dir` is the directory you cloned the repo in. For example:

       sudo ln -s /home/user/xfce-prayer-times/prayer-times-plugin.desktop /usr/share/xfce4/panel/plugins/prayer-times-plugin.desktop
       sudo ln -s /home/user/xfce-prayer-times/build/libprayer-times-plugin.so /usr/lib/xfce4/panel/plugins/libprayer-times-plugin.so 

4. Restart xfce4-panel.

       xfce4-panel -r

5. Add the plugin to your panel.

    1. Go to **Settings>Panel>Items**.
    2. Click the **+ Add** button.
    3. Double click **Prayer Times**.

# Usage

> [!NOTE]
> Prayer times are calculated with the formula taken from here https://radhifadlillah.com/blog/2020-09-06-calculating-prayer-times/

> [!CAUTION]
> Calculated prayer times probably will not be exact. In some locations the local scholar usually will adjust the schedule forward or backward for a few minutes.

<p align="center">
    <img src="https://github.com/user-attachments/assets/f067e60d-905a-4774-83ce-f5e736926702" width=40%>
</p>

<p align="center">
    <img src="https://github.com/user-attachments/assets/067c3cf8-f476-4c45-b1db-dfe806954d9a" width=20%>
    <img src="https://github.com/user-attachments/assets/a927a47f-3e77-4848-8a65-6638390ae17a" width=20%>
</p>

Plugin label will show the time left to the next calculated prayer time. If the plugin checkbox is not checked the plugin will send a notification each notification interval. After the checkbox is checked notifications will stop until the next prayer time. 

> [!TIP]
> You can set the notification interval from the properties dialog. 

### Properties

<p align="center">
  <img src="https://github.com/user-attachments/assets/48463344-73cb-40f2-9c89-99a4003c2f9b" width=40%>
  <img src="https://github.com/user-attachments/assets/72885bf7-636e-4e14-b928-9b394b91cc31" width=40%>
</p>


Prayer times are calculated with the formula taken from this [page](https://radhifadlillah.com/blog/2020-09-06-calculating-prayer-times/). You can read the requirements section there to learn how to setup the parameters. 

# Translate

Translation files are stored in the `panel-po` directory.

### Adding a new language

To add a new language run the following command:

    make po-init LANG=<language-code>

This will generate a `xpt.po` file under the `panel-po/<language-code>` directory.
In this file you can add your translations by providing the translated text in the `msgstr` fields corresponding to the `msgid` entries.

### Testing Translations

To test your translations, change the LANG environment variable to your target language and restart the XFCE panel.
For example to test the english translations use:

    LANG=en_US.utf8
    make && xfce4-panel -r

# Develop

### Clangd

generate compile_commands.json:

    bear -- make


### Debugging

    PANEL_DEBUG=xfce-prayer-times
    xfce4-panel -q && make && xfce4-panel 
