#pragma once
#define DllExport __declspec(dllexport)
#define WINAPI      __stdcall
#define WIN32_LEAN_AND_MEAN             //  从 Windows 头文件中排除极少使用的信息

#include <windows.h>

///深度行情
struct MarketData
{
	///合约代码
	char	InstrumentID[32];
	///最新价
	double	LastPrice;
	///申买价一
	double	BidPrice1;
	///申买量一
	int	BidVolume1;
	///申卖价一
	double	AskPrice1;
	///申卖量一
	int	AskVolume1;
	///当日均价
	double	AveragePrice;
	///数量
	int	Volume;
	///持仓量
	double	OpenInterest;
	///最后修改时间:yyyyMMdd HH:mm:ss
	char	UpdateTime[32];
	///最后修改毫秒
	int	UpdateMillisec;
	///涨停板价
	double	UpperLimitPrice;
	///跌停板价
	double	LowerLimitPrice;
};


typedef int (WINAPI *DefOnFrontConnected)();		void* _OnFrontConnected;
typedef int (WINAPI *DefOnRspUserLogin)(int pErrId);	void* _OnRspUserLogin;
typedef int (WINAPI *DefOnRspUserLogout)(int pReason);	void* _OnRspUserLogout;
typedef int (WINAPI *DefOnRtnError)(int pErrId, const char* pMsg);	void* _OnRtnError;
typedef int (WINAPI *DefOnRtnDepthMarketData)(MarketData *pMarketData);	void* _OnRtnDepthMarketData;

//注册响应函数
DllExport void WINAPI RegOnFrontConnected(void* onFunction){ _OnFrontConnected = onFunction; }
DllExport void WINAPI RegOnRspUserLogin(void* onFunction){ _OnRspUserLogin = onFunction; }
DllExport void WINAPI RegOnRspUserLogout(void* onFunction){ _OnRspUserLogout = onFunction; }
DllExport void WINAPI RegOnRtnDepthMarketData(void* onFunction){ _OnRtnDepthMarketData = onFunction; }
DllExport void WINAPI RegOnRtnError(void* onFunction){ _OnRtnError = onFunction; }

int req = 0;
char _TradingDay[16];

//构造api
DllExport void WINAPI CreateApi();

///注册前置机网络地址
DllExport int WINAPI ReqConnect(char *pFront);

///用户登录请求
DllExport int WINAPI ReqUserLogin(char* pInvestor, char* pPwd, char* pBroker);

///登出请求
DllExport void WINAPI ReqUserLogout();

///获取当前交易日
///@retrun 获取到的交易日
///@remark 只有登录成功后,才能得到正确的交易日
DllExport const char* WINAPI GetTradingDay();

///订阅行情。
///@param ppInstrumentID 合约ID  
///@param nCount 要订阅/退订行情的合约个数
///@remark 
DllExport int WINAPI ReqSubMarketData(char *pInstrumentID);

///退订行情。
///@param ppInstrumentID 合约ID  
///@param nCount 要订阅/退订行情的合约个数
///@remark 
DllExport int WINAPI ReqUnSubMarketData(char *pInstrumentID);


