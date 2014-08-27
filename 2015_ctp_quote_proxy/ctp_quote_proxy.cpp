// ctp_quote_proxy.cpp : 定义 DLL 应用程序的导出函数。
//

#include "ctp_quote_proxy.h"
#include "../2015_include_lib/HFQuote.h"
#include <stdio.h>

CThostFtdcMdApi *api;
CThostFtdcMdSpi *spi;

//构造api
DllExport void WINAPI CreateApi()
{
	api = CThostFtdcMdApi::CreateFtdcMdApi("./log/");
	spi = new CctpQuote();
}

///注册前置机网络地址
DllExport int WINAPI ReqConnect(char *pFront)
{
	api->RegisterSpi(spi);
	api->RegisterFront(pFront);
	api->Init();
	return 0;
}

///用户登录请求
DllExport int WINAPI ReqUserLogin(char* pInvestor, char* pPwd, char* pBroker)
{
	CThostFtdcReqUserLoginField f;
	memset(&f, 0, sizeof(CThostFtdcReqUserLoginField));
	strcpy_s(f.BrokerID, sizeof(f.BrokerID), pBroker);
	strcpy_s(f.UserID, sizeof(f.UserID), pInvestor);
	strcpy_s(f.Password, sizeof(f.Password), pPwd);
	return api->ReqUserLogin(&f, ++req);
}

///登出请求
DllExport void WINAPI ReqUserLogout()
{
	api->RegisterSpi(NULL);
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
	char *insts[] = { pInstrumentID };
	return api->SubscribeMarketData(insts, 1);
}

///退订行情。
///@param ppInstrumentID 合约ID  
///@param nCount 要订阅/退订行情的合约个数
///@remark 
DllExport int WINAPI ReqUnSubMarketData(char *pInstrumentID)
{
	char *insts[] = { pInstrumentID };
	return api->UnSubscribeMarketData(insts, 1);
}

void CctpQuote::OnFrontConnected()
{
	if (_OnFrontConnected)
	{
		((DefOnFrontConnected)_OnFrontConnected)();
	}
}

void CctpQuote::OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	int eId = -1;
	if (pRspInfo)
		eId = pRspInfo->ErrorID;
	if (eId == 0)
	{
		strcpy_s(_TradingDay, sizeof(_TradingDay), api->GetTradingDay());
	}
	if (_OnRspUserLogin)
	{
		((DefOnRspUserLogin)_OnRspUserLogin)(eId);
	}
}

void CctpQuote::OnRspError(CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if (_OnRtnError && pRspInfo)
	{
		((DefOnRtnError)_OnRtnError)(pRspInfo->ErrorID, pRspInfo->ErrorMsg);
	}
}

void CctpQuote::OnFrontDisconnected(int nReason)
{
	if (_OnRspUserLogout)
	{
		((DefOnRspUserLogout)_OnRspUserLogout)(nReason);
	}
}

void CctpQuote::OnRtnDepthMarketData(CThostFtdcDepthMarketDataField *pDepthMarketData)
{
	if (_OnRtnDepthMarketData)
	{
		MarketData f;
		memset(&f, 0, sizeof(MarketData));
		f.AskPrice1 = pDepthMarketData->AskPrice1;
		f.AskVolume1 = pDepthMarketData->AskVolume1;
		f.AveragePrice = pDepthMarketData->AveragePrice;
		f.BidPrice1 = pDepthMarketData->BidPrice1;
		f.BidVolume1 = pDepthMarketData->BidVolume1;
		strcpy_s(f.InstrumentID, sizeof(f.InstrumentID), pDepthMarketData->InstrumentID);
		f.LastPrice = pDepthMarketData->LastPrice;
		f.LowerLimitPrice = pDepthMarketData->LowerLimitPrice;
		f.OpenInterest = pDepthMarketData->OpenInterest;
		f.UpdateMillisec = pDepthMarketData->UpdateMillisec;
		sprintf_s(f.UpdateTime, "%s %s", pDepthMarketData->ActionDay, pDepthMarketData->UpdateTime);// "%4d%2d%2d%s", day / 10000, day % 10000 / 100, day % 100, time.erase(':'));
		f.UpperLimitPrice = pDepthMarketData->UpperLimitPrice;
		f.Volume = pDepthMarketData->Volume;
		((DefOnRtnDepthMarketData)_OnRtnDepthMarketData)(&f);
	}
}

