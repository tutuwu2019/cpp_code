# 关于 rdtsc 计时 
> 注意采用的 rdtsc 并不是标准时间，而是跟cpu 的频率挂钩，更具体的来说是适配x86架构cpu
>> cat /proc/cpuinfo | grep -i rdtsc    #此命令 查看是否支持 rdtsc 

查看linux cpu频率
>lscpu | grep MHz
>CPU MHz:             2304.006  
>cat /proc/cpuinfo | grep "MHz"  查看所有内核频率
