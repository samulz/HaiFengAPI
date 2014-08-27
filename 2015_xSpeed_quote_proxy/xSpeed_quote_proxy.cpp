// xSpeed_quote_proxy.cpp : 定义 DLL 应用程序的导出函数。
//


#define DllExport __declspec(dllexport)
#define WINAPI      __stdcall
#define WIN32_LEAN_AND_MEAN             //  从 Windows 头文件中排除极少使用的信息

#include "../2015_include_lib/HFQuote.h"
#include "xSpeed_quote_proxy.h"
#include <stdio.h>


DFITCMdApi *api;
//DllExport CxSpeed_quote_proxy* spi;

//构造api
DllExport void WINAPI CreateApi()
{
	/*_FrontConnected = NULL;
	_RspUserLogin = NULL;
	_RspUserLogout = NULL;
	_RspError = NULL;
	_RtnDepthMarketData = NULL;*/
	api = DFITCMdApi::CreateDFITCMdApi();
}

///注册前置机网络地址
DllExport int WINAPI ReqConnect(char *pFront)
{
	CxSpeed_quote_proxy* spi = new CxSpeed_quote_proxy();
	return api->Init(pFront, (DFITCMdSpi*)spi);
}

///用户登录请求
DllExport int WINAPI ReqUserLogin(char* pInvestor, char* pPwd, char* pBroker)
{
	DFITCTradingDayField fDate;
	memset(&fDate, 0, sizeof(DFITCTradingDayField));
	fDate.lRequestID = ++req;
	api->ReqTradingDay(&fDate);

	DFITCUserLoginField f;
	memset(&f, 0, sizeof(DFITCUserLoginField));
	strcpy_s(f.accountID, sizeof(f.accountID), pInvestor);
	strcpy_s(f.passwd, sizeof(f.passwd), pPwd);
	return api->ReqUserLogin(&f);
}

///登出请求
DllExport void WINAPI ReqUserLogout()
{
	api->Release();
}

///获取当前交易日
///@retrun 获取到的交易日
///@remark 只有登录成功后,才能得到正确的交易日
DllExport const char* WINAPI GetTradingDay()
{
	return _TradingDay;
}

///订阅行情。
///@param ppInstrumentID 合约ID  
///@param nCount 要订阅/退订行情的合约个数
///@remark 
DllExport int WINAPI ReqSubMarketData(char *pInstrumentID)
{
	char* iis[] = { pInstrumentID };
	return api->SubscribeMarketData(iis, 1, ++req);
}

///退订行情。
///@param ppInstrumentID 合约ID  
///@param nCount 要订阅/退订行情的合约个数
///@remark 
DllExport int WINAPI ReqUnSubMarketData(char *pInstrumentID)
{
	char* iis[] = { pInstrumentID };
	return api->UnSubscribeMarketData(iis, 1, ++req);
}

///////////////////////// 以上为通用封装部分 ////////////////////

CxSpeed_quote_proxy::CxSpeed_quote_proxy(void)
{
	/*_FrontConnected = NULL;
	_RspUserLogin = NULL;
	_RspUserLogout = NULL;
	_RspError = NULL;
	_RtnDepthMarketData = NULL;*/
}

void CxSpeed_quote_proxy::OnFrontConnected()
{
	if (_OnFrontConnected != NULL)
	{
		((DefOnFrontConnected)_OnFrontConnected)();
	}
}

void CxSpeed_quote_proxy::OnFrontDisconnected(int nReason)
{
	if (_OnRspUserLogout)
	{
		((DefOnRspUserLogout)_OnRspUserLogout)(nReason);
	}
}
void CxSpeed_quote_proxy::OnRspError(struct DFITCErrorRtnField *pRspInfo)
{
	if (_OnRtnError)
	{
		pRspInfo = repareInfo(pRspInfo);	//处理 NULL的情况
		((DefOnRtnError)_OnRtnError)(pRspInfo->nErrorID, pRspInfo->errorMsg);
	}
}
void CxSpeed_quote_proxy::OnRspTradingDay(struct DFITCTradingDayRtnField * pTradingDayRtnData)
{
	strcpy_s(_TradingDay, strlen(pTradingDayRtnData->date) + 1, pTradingDayRtnData->date);
}
void CxSpeed_quote_proxy::OnRspUserLogin(struct DFITCUserLoginInfoRtnField * pRspUserLogin, struct DFITCErrorRtnField * pRspInfo)
{
	if (_OnRspUserLogin)
	{
		pRspInfo = repareInfo(pRspInfo);	//处理 NULL的情况
		((DefOnRspUserLogin)_OnRspUserLogin)(pRspInfo->nErrorID);
	}
}
void CxSpeed_quote_proxy::OnRspUserLogout(struct DFITCUserLogoutInfoRtnField * pRspUsrLogout, struct DFITCErrorRtnField * pRspInfo)
{
	/*if (_RspUserLogout)
	{
	((RspUserLogout)_RspUserLogout)(pRspInfo->nErrorID);
	}*/
}
void CxSpeed_quote_proxy::OnMarketData(struct DFITCDepthMarketDataField * pMarketDataField)
{
	if (_OnRtnDepthMarketData)
	{
		MarketData f;
		memset(&f, 0, sizeof(MarketData));
		f.AskPrice1 = pMarketDataField->AskPrice1;
		f.AskVolume1 = pMarketDataField->AskVolume1;
		f.AveragePrice = pMarketDataField->AveragePrice;
		f.BidPrice1 = pMarketDataField->BidPrice1;
		f.BidVolume1 = pMarketDataField->BidVolume1;
		strcpy_s(f.InstrumentID, sizeof(f.InstrumentID), pMarketDataField->instrumentID);
		f.LastPrice = pMarketDataField->lastPrice;
		f.LowerLimitPrice = pMarketDataField->lowerLimitPrice;
		f.OpenInterest = pMarketDataField->openInterest;
		sprintf_s(f.UpdateTime, "%s %s", pMarketDataField->tradingDay, pMarketDataField->UpdateTime);
		f.UpdateMillisec = pMarketDataField->UpdateMillisec;
		f.UpperLimitPrice = pMarketDataField->upperLimitPrice;
		f.Volume = pMarketDataField->Volume;
		((DefOnRtnDepthMarketData)_OnRtnDepthMarketData)(&f);
	}
}


