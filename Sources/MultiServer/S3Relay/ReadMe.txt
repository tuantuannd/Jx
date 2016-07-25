========================================================================
       WIN32 APPLICATION : S3Relay
========================================================================



预编译宏：
_WORKMODE_SINGLETHREAD / _WORKMODE_MULTITHREAD / _WORKMODE_MULTITHREAD2 : 工作模式（单线程/多线程/回调多线程），必须定义其中之一
_TESTING : 如果此宏被定义，dTRACE和DEBUGDO等自定义Debug指令也会在Release版中有效（assert等系统指令不包括在内）






本机作服务器（Heaven）：
CNetServer: 启动服务
CNetConnect: 收到的连接
	各种样式的服务器都从以上两个类派生，以增加控制

本机作客户（Rainbow）：
CNetCenter: 客户管理
CNetClient: 客户连接
	各种样式的客户都从以上两个类派生，以增加控制



CRootCenter / CRootClient: 与RelayServer连接相关，都仅有一个实例
CGatewayCenter / CGatewayClient: 与Gateway连接相关，都仅有一个实例
CDBCenter / CDBClient: 与DBRole连接相关，都仅有一个实例

CRelayCenter / CRelayClient: 与其他同级Relay连接，CRelayCenter有一个实例，CRelayClient与同级Relay相关
CRelayServer / CRelayConnect: 其他同级Relay与本机连接，CRelayServer有一个实例，CRelayConnect与连接对应

每个GameServer与S3Relay之间都有3个连接：Host、Chat、Tong，分别用作主连接、聊天、和频道
CHostServer / CHostConnect: GameServer与本机连接，CHostServer有一个实例，CHostConnect与连接对应
CChatServer / CChatConnect: GameServer与本机连接，CChatServer有一个实例，CChatConnect与连接对应
CTongServer / CTongConnect: GameServer与本机连接，CTongServer有一个实例，CTongConnect与连接对应




配置文件（INI格式）：

relay_config.ini : 连接配置文件

	[root]						//作为客户，S3RelayServer服务器相关参数
	address = 192.168.20.63		//内网IP地址
	freebuffer = 15				//（网络底层参数）
	buffersize = 1048576		//（网络底层参数）
	retryinterval = 12000		//重连间隔，0则永不重联


	[gateway]					//作为客户，Bishop服务器相关参数，参见[root]
	address = 192.168.20.63
	freebuffer = 15
	buffersize = 1048576
	retryinterval = 12000

	[dbrole]					//作为客户，Goddess服务器相关参数，参见[root]
	address = 192.168.20.63
	freebuffer = 15
	buffersize = 1048576
	retryinterval = 12000

	[relay]						//作为服务器，为其他S3Relay开启的服务的相关参数
	playercnt = 10				//（网络底层参数）
	precision = 1				//（网络底层参数）
	freebuffer = 15				//（网络底层参数）
	buffersize = 1048576		//（网络底层参数）

	[host]						//作为服务器，为GameServer的Host连接开启的服务的相关参数，参见[relay]
	playercnt = 10
	precision = 1
	freebuffer = 15
	buffersize = 1048576

	[chat]						//作为服务器，为GameServer的Chat连接开启的服务的相关参数，参见[relay]
	playercnt = 10
	precision = 1
	freebuffer = 15
	buffersize = 1048576

	[tong]						//作为服务器，为GameServer的Tong连接开启的服务的相关参数，参见[relay]
	playercnt = 10
	precision = 1
	freebuffer = 15
	buffersize = 1048576


relay_channcfg.ini : 频道基本配置文件

	[system]					//系统参数
	nameGM = GM					//GM频道名
	charEsc = \					//（内部参数）
	charSplt = :				//（内部参数）
	defCost = 0					//默认花费

	[team]						//队伍频道
	escSpec = T					//（内部参数）
	minID = 0					//队伍最小ID
	maxID = -1					//队伍最大ID
	cost = 0					//队伍聊天花费

	[faction]					//门派频道
	escSpec = F					//（内部参数）
	minID = 0					//队伍最小ID
	maxID = -1					//队伍最大ID
	cost = 1					//门派聊天花费

	[tong]						//帮会频道
	escSpec = O					//（内部参数）
	minID = 0					//队伍最小ID
	maxID = -1					//队伍最大ID
	cost = 1					//帮会聊天花费

	[screen]					//同屏聊天
	escSpec = S					//（内部参数）
	cost = 0					//同屏聊天花费

	[broadcast]					//城市频道
	escSpec = B					//（内部参数）
	cost = 2					//城市聊天花费


relay_channel.ini : Stock频道列表

	[世界]						//频道名称
	cost = 2					//频道花费


relay_friendcfg.ini : 好友参数配置文件

	[database]					//好友数据库参数
	path = dbfriend				//数据库文件夹
	file = friend				//数据库名称
	updateinterval = 300000		//Update数据库间隔，0则不会自动Update
	storeplayerperudtdb = 8		//（内部参数）
	loadplayerperudtdb = 2		//（内部参数）
