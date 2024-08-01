use server;

drop table if exists images_table cascade;
drop table if exists user_table cascade;

create table if not exists user_table(
    id int primary key auto_increment,
    username varchar(32) unique,
    password varchar(64)
);

insert into user_table (username, password) 
        values ('test1', 'E10ADC3949BA59ABBE56E057F20F883E'),  -- 123456
                ('test2','E35CF7B66449DF565F93C607D5A81D09');  -- 456789
                
create table if not exists images_table(
    id int primary key auto_increment,
    username varchar(32) not null,
    imgname varchar(128) not null,
    imgpath varchar(256), 
    foreign key (username) references user_table(username)
);
 


select * from user_table;
select * from images_table;