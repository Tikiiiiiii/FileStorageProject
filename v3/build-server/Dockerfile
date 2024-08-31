FROM ubuntu:latest
# 服务端文件
ADD ./server/ /home/server/

# 安装依赖工具及库
RUN apt-get update && \
    apt-get install -y \
    cmake \
    g++ \
    libevent-dev \
    libjsoncpp-dev \
    libmysqlcppconn-dev \
    libcrypto++-dev \
    libspdlog-dev

# 清楚包，减轻镜像大小
RUN apt-get clean && rm -rf /var/lib/apt/lists/*

# 切换目录
WORKDIR /home/server/build

# 编译
RUN cmake .. && make

# 创建图片文件夹
RUN mkdir /home/Image
RUN mkdir /home/Image/test1 /home/Image/test2

# 工作目录
WORKDIR /home/server/bin

# 暴露端口
EXPOSE 8081

CMD ./server