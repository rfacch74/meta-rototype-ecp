#!/bin/bash

while :
do
  read -t 5 -p "Do you want to program ECP board? (Y/N): "
  if [ $? -gt 128 ]; then
    echo "N"
    break
  fi

  case $REPLY in
  [yY]*)
    echo "Y"
    break
    ;;
  [nN]*)
    echo "N"
    break
    ;;
  *) echo "Please enter Y or N"
     ;;
  esac
done


