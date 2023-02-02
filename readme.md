利用某安全公司驱动程序实现DLL注入（进程刚启动时），稳定性还可以

****

用法：

安装  

qianxin_loaddll.exe -i [32位dll全路径] [64位dll全路径]

卸载

qianxin_loaddll.exe -u

****

注意事项  

被注入的dll都要导出一个叫ijtdummy的函数。