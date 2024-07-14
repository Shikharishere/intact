#!/bin/bash

echo -n "Run intact after installation? Yes or No: "
read run
echo "setting up intact"
make intact
sudo mv intact /usr/local/bin
sudo chmod +x /usr/local/bin/intact
if [ "$run" = "Yes" ]; then
    intact
else
    echo "Intact is installed. Run 'intact' to start"
fi
