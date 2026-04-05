# readme 

## 经典三模块
1. RAW内存
2. 核心单线线程
3. IO多路复用


## 数据结构

Ziplist 与 Listpack

- SDS(Simple Dynamic String) Redis 自研字符串结构，支持O(1) 获取长度，二进制安全，并采用空间预分配策略减少内存重分配
- SkipList(跳表) 用于实现ZSet，支持对数级的查询效率，是实现排行榜等功能的核心
- Listpack 的革命，早期的Ziplist 在某些极端情况下会触发“连锁更新"(Cascading Update)，导致性能剧烈抖动。Redis7.0 引入的Listpack 采用了更精巧的经凑布局，在保持内存高校利用的同时，彻底规避了连锁更新风险


## 持久化策略
> 作为内存数据库，Redis必须解决宕机后数据恢复问题。它提供了两种互补的持久化机制，并在最新版本中进行了深度优化。

1. RDB(Redis Database): 通过定时器生成数据快照(snapshot)的方式将内存数据保存到磁盘中。它适合全量备份，恢复速度快，但存在数据丢失窗口
2. AOD(Append Only File) 通过记录每条写命令来保证数据的实时性
3. Redis 7.0 的 Multi-Part AOF 这是AOF 机制一次重大升级。传统的AOF 在重写期间会产生巨大IO 压力。Mutil-Part AOF 将AOF 分为Base(基础)文件和Incremental(增量)文件，显著降低了重写开销，提升了系统的稳定性。


## 高可用架构：从哨兵到集群

> 为了应对大规模开发和海量数据,redis 提供了多种分布式部署方案，满足同业务场景下的可用性要求。

- 主从复制
- 哨兵模式
- 集群模式




