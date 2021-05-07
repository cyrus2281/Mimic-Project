#!/bin/bash
read -p "Please enter the comment" msg;
git add .
git commit -m "$msg"
git push
echo "Respository Updated"
sleep 2
