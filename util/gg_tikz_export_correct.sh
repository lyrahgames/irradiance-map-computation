#!/bin/bash

sed -i 's/\[10pt\]{article}/{standalone}/g' $1
sed -i '5a\\\usetikzlibrary{arrows.meta}' $1
sed -i 's/\$\$/\$/g' $1
sed -i 's/>=triangle 45/>={Stealth}/g' $1