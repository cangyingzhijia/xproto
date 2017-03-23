# xproto Introduction

xproto was a long time ago to write a small library, and then to solve the problem of bad data protobuf format printing.

protobuf used in server storage, rpc sequence and so, because of its high performance, ease of use, data scalability, and other characteristics and therefore used very widely, 
but in the development and testing process protobuf is not easy to print binary data show, while many scene rpc using protobuf, and display layers using json, xml, etc., 
which use more web format, if the flexibility to switch between data and protobuf json, xml and other formats would be more convenient.

Based on this consideration, think if you really can achieve these free conversion between text and protobuf so in development is helpful, 
therefore protobuf depth study, we found that can achieve these functions through protobuf reflection mechanism. 
So I try to use two days spare time to realize this idea, also appeared xproto this small library. 
Sneaky later reference it in their daily work, we have been used up to now, I have worked in teams to get a lot of use.

# xproto介绍
xproto是很久之前就写的一个小类库了，那时候是为了解决protobuf格式的数据不好打印的问题。

protobuf用在服务端存储、rpc序列化等，由于其高性能、使用方便、数据可扩展性等特性因此用的很广泛,<br>
但是在开发测试过程中protobuf是二进制数据不容易打印显示，同时在很多场景下rpc使用的是protobuf，而显示层使用的是json、xml等,<br>
这些web使用较多的格式，如果能在json、xml等格式的数据与protobuf之间灵活转换那就方便多了。

基于这种考虑，觉得如果真能实现这些文本格式与protobuf之间的自由转换那么在开发中是很有帮助的，<br>
因此深入研究了protobuf后，发现通过protobuf reflection机制完全可以实现这些功能。<br>
所以就尝试着使用两天业余的时间实现了这个想法,也就出现了xproto这个小类库。<br>
后来又偷偷摸摸的把它引用在日常的工作中，一直使用到现在，在我工作过的团队得到大量的使用。
