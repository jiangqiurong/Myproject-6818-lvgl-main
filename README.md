# Myproject-6818-lvgl-main
基于gec6818开发板做的第二阶段项目，项目为：基于Linux的UU网盘文件传输系统，分客户端和服务器，客户端功能：登录，注册，注销，文件上传、下载，开发板目录查看，天气显示，视频播放，音频播放；服务器功能：接收客户端发送的请求，对登录，注册，注销，文件上传、下载，天气数据的操作

lvgl版本：8.2

客户端文件:lv_port_linux_frame_buffer-release-v8.2

服务器文件:register_login
 
项目运行

先运行服务器后运行客户端

服务器：执行make，也可以make clean后make,如果make clean后make出现bug可自行修复，修复不了代码，可重新再下载一份代码，重新开始

客户端：

1.修改图片的路径以及连接服务器的ip，vscode可使用Ctrl+f快速替换路径

2.把liblvgl.so挂载到开发板的/lib目录下:cp /mnt/nfs/myproject-6818-lvgl-main/lv_port_linux_frame_buffer-release-v8.2/liblvgl.so /lib/

！！！注：/mnt/nfs/myproject-6818-lvgl-main/lv_port_linux_frame_buffer-release-v8.2/为我的共享路径，每个人的共享路径可能不同

3.把mplayer挂载到开发板的/usr/bin/上  :cp /mnt/nfs/myproject-6818-lvgl-main/lv_port_linux_frame_buffer-release-v8.2/mplayer /usr/bin/

4.音频.mp3要挂载到开发板的/IOT/mp3/上  :cp /mnt/nfs/myproject-6818-lvgl-main/lv_port_linux_frame_buffer-release-v8.2/my_lvgl_test/res/music/bolizhiqing.mp3 /IOT/mp3/

5.视频.avi要挂载到开发板的/IOT/video/上:cp /mnt/nfs/myproject-6818-lvgl-main/lv_port_linux_frame_buffer-release-v8.2/my_lvgl_test/res/1.avi /IOT/video/

5.执行make，也可以make clean后make,如果make clean后make出现bug可自行修复，修复不了代码，可重新再下载一份代码，重新配置
