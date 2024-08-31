FROM mysql:8

ENV MYSQL_ROOT_PASSWORD=123456

ADD ./init.sql /docker-entrypoint-initdb.d/

EXPOSE 3306