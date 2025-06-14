#!/bin/bash

# server
systemctl stop RED_API_SERVER
rm -rf /root/RED_Safe_apiserver
cp /root/RED-Safe/build/api-server/RED_Safe_apiserver /root
systemctl start RED_API_SERVER

# web
rm -rf /var/www/elder-alert-system/
cp -r /root/RED-Safe/elder-alert-system /var/www/elder-alert-system

# nginx
systemctl stop nginx
rm -rf /etc/nginx/nginx.conf
cp /root/RED-Safe/config/nginx.conf /etc/nginx/
systemctl start nginx