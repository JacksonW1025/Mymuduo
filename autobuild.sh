#! /bin/bash

set -e # 如果命令执行失败，则退出

echo "开始安装mymuduo库"
# 如果build目录不存在，则创建build目录
if [ ! -d "build" ]; then
    mkdir build
fi

rm -rf `pwd`/build/*
cd `pwd`/build && cmake .. && make -j4

cd ..

# 把头文件拷贝到 /usr/include/mymuduo 目录下, so库拷贝到 /usr/lib 目录下 PATH
if [ ! -d /usr/include/mymuduo ]; then
    mkdir -p /usr/include/mymuduo
fi

for header in `ls *.h`
do
    cp $header /usr/include/mymuduo/
done

cp `pwd`/lib/libmymuduo.so /usr/lib/

ldconfig # 更新动态链接库缓存

echo "mymuduo 库安装成功"