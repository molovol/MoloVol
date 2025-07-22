on run argv
    -- Use the argument directly as volume name (e.g. "MoloVol")
    set volumeName to item 1 of argv

    tell application "Finder"
        tell disk volumeName
            open
            set current view of container window to icon view
            set toolbar visible of container window to false
            set statusbar visible of container window to false
            set the bounds of container window to {100, 100, 550, 400}

            set viewOptions to the icon view options of container window
            set arrangement of viewOptions to not arranged
            set icon size of viewOptions to 80
            set text size of viewOptions to 14
            set background picture of viewOptions to file ".background:background.png"

            set position of item ".fseventsd" to {400, 400}
            set position of item ".background" to {500, 400}
            set position of item "Applications" to {350,130}
            set position of item "MoloVol.app" to {103, 130}

            close
            open
            delay 2
        end tell
    end tell
end run

