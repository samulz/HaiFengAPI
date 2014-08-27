#define DllExport __declspec(dllexport)
#define WINAPI      __stdcall
#define WIN32_LEAN_AND_MEAN             //  从 Windows 头文件中排除极少使用的信息

#include <windows.h>
#include <time.h>
#include <map>

// exchange[8], xxxtime[16], id[32], msg[128]
///////// 函数封装 ////////
#pragma region enum
enum OrderType : int
{
	/// <summary>
	/// 限价
	/// </summary>
	Limit,
	/// <summary>
	/// 市价
	/// </summary>
	Market,
	/// <summary>
	/// 部成即撤
	/// </summary>
	FAK,
	/// <summary>
	/// 全成全撤
	/// </summary>
	FOK
};

/// <summary>
/// 买卖方向
/// </summary>
enum DirectionType : int
{
	/// <summary>
	/// 买
	/// </summary>
	Buy,

	/// <summary>
	/// 卖
	/// </summary>
	Sell
};

/// <summary>
/// 开平
/// </summary>
enum OffsetType : int
{
	/// <summary>
	/// 
	/// </summary>
	Open,
	/// <summary>
	/// 
	/// </summary>
	Close,
	/// <summary>
	/// 平今
	/// </summary>
	CloseToday,
	/// <summary>
	/// 期权行权
	/// </summary>
	Excute,
};

enum OrderStatus : int
{
	/// <summary>
	/// 委托
	/// </summary>
	Normal,
	/// <summary>
	/// 部成
	/// </summary>
	Partial,
	/// <summary>
	/// 全成
	/// </summary>
	Filled,
	/// <summary>
	/// 撤单
	/// </summary>
	Canceled,
};

/// <summary>
/// 交易所状态
/// </summary>
enum ExchangeStatusType : int
{
	/// <summary>
	/// 开盘前
	/// </summary>
	BeforeTrading,
	/// <summary>
	/// 非交易
	/// </summary>
	NoTrading,
	/// <summary>
	/// 交易
	/// </summary>
	Trading,
	/// <summary>
	/// 收盘
	/// </summary>
	Closed,
};

/// <summary>
/// 投机套保标志
/// </summary>
enum HedgeType : int
{
	/// <summary>
	/// 投机
	/// </summary>
	Speculation,

	/// <summary>
	/// 套利
	/// </summary>
	Arbitrage,

	/// <summary>
	/// 套保
	/// </summary>
	Hedge,
};
#pragma endregion enum

#pragma region structs
/// <summary>
/// 合约信息
/// </summary>
struct InstrumentField
{
	/// <summary>
	/// 合约代码
	/// </summary>
	char InstrumentID[32];

	/// <summary>
	/// 产品代码
	/// </summary>
	char ProductID[32];

	/// <summary>
	/// 交易所代码
	/// </summary>
	char ExchangeID[8];

	/// <summary>
	/// 合约数量乘数
	/// </summary>
	int VolumeMultiple;

	/// <summary>
	/// 最小变动价位
	/// </summary>
	double PriceTick;
};

/// <summary>
/// 持仓
/// </summary>
struct PositionField
{
	/// <summary>
	/// 合约代码
	/// </summary>
	char InstrumentID[32];

	/// <summary>
	/// 买卖
	/// </summary>
	DirectionType Direction;

	/// <summary>
	/// 持仓均价
	/// </summary>
	double Price;

	/// <summary>
	/// 总持仓量
	/// </summary>
	int Position;

	/// <summary>
	/// 昨仓
	/// </summary>
	int YdPosition;

	/// <summary>
	/// 今仓
	/// </summary>
	int TdPosition;

	/// <summary>
	/// 占用保证金
	/// </summary>
	//double Margin;

	/// <summary>
	/// 投机套保标志
	/// </summary>
	HedgeType Hedge;
};

/// <summary>
/// 帐户权益
/// </summary>
struct TradingAccount
{
	/// <summary>
	/// 上次结算准备金
	/// </summary>
	double PreBalance;

	/// <summary>
	/// 持仓盈亏
	/// </summary>
	double PositionProfit;

	/// <summary>
	/// 平仓盈亏
	/// </summary>
	double CloseProfit;

	/// <summary>
	/// 手续费
	/// </summary>
	double Commission;

	/// <summary>
	/// 当前保证金总额
	/// </summary>
	double CurrMargin;

	/// <summary>
	/// 冻结的资金
	/// </summary>
	double FrozenCash;

	/// <summary>
	/// 可用资金
	/// </summary>
	double Available;

	/// <summary>
	/// 动态权益
	/// </summary>
	double Fund;
};

/// <summary>
/// 报单
/// </summary>
struct OrderField
{
	/// <summary>
	/// 报单标识
	/// </summary>
	long OrderId;

	/// <summary>
	/// 合约
	/// </summary>
	char InstrumentID[32];

	/// <summary>
	/// 买卖
	/// </summary>
	DirectionType Direction;

	/// <summary>
	/// 开平
	/// </summary>
	OffsetType Offset;

	/// <summary>
	/// 报价
	/// </summary>
	double LimitPrice;

	/// <summary>
	/// 成交均价
	/// </summary>
	double AvgPrice;

	/// <summary>
	/// 委托时间(交易所)
	/// </summary>
	char InsertTime[16];

	/// <summary>
	/// 最后成交时间
	/// </summary>
	char TradeTime[16];

	/// <summary>
	/// 本次成交量,trade更新
	/// </summary>
	int TradeVolume;

	/// <summary>
	/// 报单数量
	/// </summary>
	int Volume;

	/// <summary>
	/// 未成交,trade更新
	/// </summary>
	int VolumeLeft;

	/// <summary>
	/// 投保
	/// </summary>
	HedgeType Hedge;

	/// <summary>
	/// 是否被撤单
	/// </summary>
	OrderStatus Status;

	/// <summary>
	/// 是否自身报单
	/// </summary>
	bool IsLocal;
};

/// <summary>
/// 成交
/// </summary>
struct TradeField
{
	/// <summary>
	/// 成交编号
	/// </summary>
	char TradeID[32];

	/// <summary>
	/// 合约代码
	/// </summary>
	char InstrumentID[32];

	/// <summary>
	/// 交易所代码
	/// </summary>
	char ExchangeID[8];

	/// <summary>
	/// 买卖方向
	/// </summary>
	DirectionType Direction;

	/// <summary>
	/// 开平标志
	/// </summary>
	OffsetType Offset;

	/// <summary>
	/// 投机套保标志
	/// </summary>
	HedgeType Hedge;

	/// <summary>
	/// 价格
	/// </summary>
	double Price;

	/// <summary>
	/// 数量
	/// </summary>
	int Volume;

	/// <summary>
	/// 成交时间
	/// </summary>
	char TradeTime[16];

	/// <summary>
	/// 交易日
	/// </summary>
	char TradingDay[16];

	/// <summary>
	/// 对应的委托标识
	/// </summary>
	long OrderID;
};
#pragma endregion structs

#pragma region typedef
///当客户端与交易后台建立起通信连接时（还未登录前），该方法被调用。
typedef int (WINAPI *DefOnFrontConnected)();								void* _OnFrontConnected;

///登录请求响应
typedef int (WINAPI *DefOnRspUserLogin)(int pErrId);		void* _OnRspUserLogin;

///登出请求响应
typedef int (WINAPI *DefOnRspUserLogout)(int pReason);						void* _OnRspUserLogout;

///错误应答
typedef int (WINAPI *DefOnRtnError)(int pErrId, const char* pMsg);			void* _OnRtnError;

//交易信息
typedef int (WINAPI *DefOnRtnNotice)(const char* pMsg);						void* _OnRtnNotice;

//交易所状态信息
typedef int (WINAPI *DefOnRtnExchangeStatus)(const char* pExchangeID, ExchangeStatusType pStatus);	void* _OnRtnExchangeStatus;

//返回合约,登录后自动调用
typedef int (WINAPI *DefOnRspQryInstrument)(InstrumentField* pInstrument, bool pLast);	void* _OnRspQryInstrument;

//返回合约,登录后自动调用
typedef int (WINAPI *DefOnRspQryOrder)(OrderField* pOrder, bool pLast);	void* _OnRspQryOrder;

//返回合约,登录后自动调用
typedef int (WINAPI *DefOnRspQryTrade)(TradeField* pTrade, bool pLast);	void* _OnRspQryTrade;

//返回合约,登录后自动调用
typedef int (WINAPI *DefOnRspQryPosition)(PositionField* pPosition, bool pLast);	void* _OnRspQryPosition;

//返回合约,登录后自动调用
typedef int (WINAPI *DefOnRspQryTradingAccount)(TradingAccount* pAccount);	void* _OnRspQryTradingAccount;

//报单响应
typedef int(WINAPI *DefOnRtnOrder)(OrderField*);							void* _OnRtnOrder;

//报单成交响应
typedef int (WINAPI *DefOnRtnTrade)(TradeField*);							void* _OnRtnTrade;

//报单撤单响应
typedef int (WINAPI *DefOnRtnCancel)(OrderField*);							void* _OnRtnCancel;
#pragma endregion typedef


//注册响应函数
DllExport void WINAPI RegOnFrontConnected(void* onFunction){ _OnFrontConnected = onFunction; }
DllExport void WINAPI RegOnRspUserLogin(void* onFunction){ _OnRspUserLogin = onFunction; }
DllExport void WINAPI RegOnRspUserLogout(void* onFunction){ _OnRspUserLogout = onFunction; }
DllExport void WINAPI RegOnRtnError(void* onFunction){ _OnRtnError = onFunction; }
DllExport void WINAPI RegOnRtnNotice(void* onFunction){ _OnRtnNotice = onFunction; }
DllExport void WINAPI RegOnRtnExchangeStatus(void* onFunction){ _OnRtnExchangeStatus = onFunction; }
DllExport void WINAPI RegOnRspQryInstrument(void* onFunction){ _OnRspQryInstrument = onFunction; }
DllExport void WINAPI RegOnRspQryOrder(void* onFunction){ _OnRspQryOrder = onFunction; }
DllExport void WINAPI RegOnRspQryTrade(void* onFunction){ _OnRspQryTrade = onFunction; }
DllExport void WINAPI RegOnRspQryPosition(void* onFunction){ _OnRspQryPosition = onFunction; }
DllExport void WINAPI RegOnRspQryTradingAccount(void* onFunction){ _OnRspQryTradingAccount = onFunction; }
DllExport void WINAPI RegOnRtnOrder(void* onFunction){ _OnRtnOrder = onFunction; }
DllExport void WINAPI RegOnRtnTrade(void* onFunction){ _OnRtnTrade = onFunction; }
DllExport void WINAPI RegOnRtnCancel(void* onFunction){ _OnRtnCancel = onFunction; }
////////////////////////////
DllExport void WINAPI CreateApi();
DllExport int WINAPI ReqConnect(char *pFront);
DllExport int WINAPI ReqUserLogin(char* pInvestor, char* pPwd, char* pBroker);
DllExport void WINAPI ReqUserLogout();
DllExport const char* WINAPI GetTradingDay();

DllExport int WINAPI ReqQryOrder();
DllExport int WINAPI ReqQryTrade();
DllExport int WINAPI ReqQryPosition();
DllExport int WINAPI ReqQryAccount();

DllExport int WINAPI ReqOrderInsert(char *pInstrument, DirectionType pDirection, OffsetType pOffset, double pPrice, int pVolume, HedgeType pHedge, OrderType pType);
DllExport int WINAPI ReqOrderAction(long pOrderId);

using namespace std;
int req = 0;
char _TradingDay[16];
char _investor[16];
char _broker[16];
map<long, OrderField> _id_order;

/*
登录后完成获取tradingday
登录后:qryaccount, qryposition, qryorder, qrytrade
*/

