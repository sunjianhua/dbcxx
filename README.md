# 快速处理静态数据

在游戏中不会产生变化的基础数据为静态数据，比如技能属性，物品属性等，在RPG或模拟经营类游戏，用到静态数据的地方尤其多，使用也非常频繁，因为物品、道具、技能、任务等的基础数据都是静态数据，因此怎么高效的处理静态数据也值得研究一下。

> 静态数据对游戏而言就是只读数据，所以不用考虑数据保存的问题，静态数据在游戏运行期间不会有增删改的需求，所以不用考虑数据地址变动问题，根据这两个特性，快速的处理静态数据主要考虑数据加载和数据的查找都要快速和高效。

##数据加载
数据加载和数据的保存方式是相关的，个人接触过的静态数据保存有三种模式：保存到数据库，保存到文本，保存到自定义格式。

###保存到数据库
这种在服务器端比较常见，把静态数据保存在MYSQL或SQL SERVER等数据库中，服务器启动从数据库加载，如果静态数据比较多，可以看到服务器加载个几分钟静态数据，这样处理的好处是静态数据和动态数据（比如人物的经验值、金钱、等级等）统一处理。在客户端也有这样处理的，但在客户端用MYSQL或SQL SERVER不经济，所以像SQLite这种内存数据库就有了用武之地。

用数据库保存静态数据，在程序启动的时候从数据库读数据，数据库会在内存中维护一份用到的数据，程序再把读到的数据赋值给相关的数据结构，在特定时间会在内存维护两份数据，会有一次把一份数据赋值给另一份数据的操作，如果静态数据比较多，这个简单的操作也是比较费时的，在服务器端这个耗时操作可以接受，毕竟服务器内存大，程序启动一次，如果不崩溃可以一直运行，但是在客户端，如果按这种方式处理静态数据就有点不经济，所以不考虑。

###保存到文本
把静态数据保存成xml格式或者json格式，这个除了描述信息方便，用起来比较不经济，所以多是在游戏配置信息部分见到用。另一种见用到的保存方式为：CSV格式，这个因为可以用EXCEL作为数据编辑器，比较方便策划。

保存静态数据到文本，在程序启动加载静态数据的时候，文本解析部分要解析文本，会在内存中维护一份用到的数据，程序再把读到的数据赋值给相关的数据结构，在特定时间会在内存维护两份数据，会有一次把一份数据赋值给另一份数据的操作，如果静态数据比较多，这些操作也是比较费时的。所以不考虑。

###保存到自定义格式
自定义保存格式，见到的基本是按数据结构，直接把数据写到文件中，这样处理好处是，加载的时候，在内存按数据结构和数据数量分配好内存，然后读取磁盘上的文件直接到分配好的内存，然后就可以直接使用了，没有从其它数据源的读取再解析，没有从其它数据源的再赋值，所以效率非常高。

但是这种方式因为字节对齐的问题，如果跨平台，在使用的时候是有些限制的，并且要额外提供专用数据编辑器，会多做些工作，但是高效这一优点应该是值得多做些工作的。

##数据查找
数据查找见到的多是hash_map和二分查找，但是再快也是要找一下的，像静态数据在程序运行过程中又不会增删改，所以一步到位是最理想了，要做到一步到位，就只有数据的索引(ID)等同于数据在数组的下标，这样在内存中查找数据就非常快了。而这样做又非常简单，只要和策划沟通好，编辑静态数据的时候，数据索引(ID)从0逐渐递增就可以了，在程序内存中，根据数据索引(ID)和数组下标的对应关系，直接分配到对应数组区间，这样查找的时候，直接是数组下标定位数据，非常快。

#DBC格式
dbc格式，是魔兽世界静态数据的保存格式，对它的解析可以看arcemu这个开源服务器的dbc部分。个人觉得在魔兽世界实际处理dbc数据的时候，应该比arcemu处理的更高效，魔兽世界的dbc文件，数据索引(ID)从0逐渐递增，数据基本是按数据结构保存，为了避免跨平台因字节长度或字节对齐造成的问题，在数据类型上以4字节为主，比如int,unsigned int,float,避免使用char、short、long？？？

因为字符串长度不确定，如果数据结构中有字符串字段，直接被一起保存到文件，会在加载时，因为字符串的长度不确定，要动态的计算的地方太多，dbc格式对此的解决方式就比较高，把字符串和其它数据分开存储，把字符串都放到一起，形成字符串区域。数据结构中的字符串字段，在文件中保存的是所用字符串在字符串区域的偏移地址。这个处理方式非常赞，
