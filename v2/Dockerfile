FROM server:v1.0

ADD ./server/ /home/server/

RUN cd /home/server/build && cmake .. && make && mkdir /home/Image

RUN mkdir /home/Image/test1 /home/Image/test2

WORKDIR /home/server/bin

EXPOSE 8080

CMD ./server