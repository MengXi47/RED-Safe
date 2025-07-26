#!/bin/bash

# user service
systemctl stop RED_SERVER_USER_SERVICE
rm -rf /root/server/user-service
mkdir /root/server/user-service
cp /root/RED-Safe/build/bin/RED_Safe_user_service /root/server/user-service
systemctl start RED_SERVER_USER_SERVICE

# edge service
systemctl stop RED_SERVER_EDGE_SERVICE
rm -rf /root/server/edge-service
mkdir /root/server/edge-service
cp /root/RED-Safe/build/bin/RED_Safe_edge_service /root/server/edge-service
systemctl start RED_SERVER_EDGE_SERVICE

# ios service
systemctl stop RED_SERVER_IOS_SERVICE
rm -rf /root/server/ios-service
mkdir /root/server/ios-service
cp /root/RED-Safe/build/bin/RED_Safe_ios_service /root/server/ios-service
systemctl start RED_SERVER_IOS_SERVICE

# web
rm -rf /var/www/elder-alert-system/
cp -r /root/RED-Safe/elder-alert-system /var/www/elder-alert-system

# nginx
systemctl stop nginx
rm -rf /etc/nginx/nginx.conf
cp /root/RED-Safe/server/nginx.conf /etc/nginx/
systemctl start nginx