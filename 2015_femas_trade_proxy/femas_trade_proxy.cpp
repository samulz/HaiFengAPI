// femas_trade_proxy.cpp : 定义 DLL 应用程序的导出函数。
//

#include "femas_trade_proxy.h"
#include "../2015_include_lib/HFTrade.h"

CfemasTrade *spi;
CUstpFtdcTraderApi *api;
map<string, InstrumentField> _id_instrument;
map<long, string> _id_sysid; //orderid&sysid
long _session = -1;
HANDLE hThread;//启动时查询用
char _userID[16];

DllExport void WINAPI CreateApi()
{
	api = CUstpFtdcTraderApi::CreateFtdcTraderApi("./log/");

}
DllExport int WINAPI ReqConnect(char *pFront)
{
	spi = new CfemasTrade();
	api->RegisterSpi(spi);
	api->RegisterFront(pFront);
	api->SubscribePrivateTopic(USTP_TERT_QUICK);	//私有流用quick
	api->SubscribePublicTopic(USTP_TERT_RESTART);	//公有流用public
	api->SubscribeUserTopic(USTP_TERT_QUICK);
	api->Init();
	return 0;
}
DllExport int WINAPI ReqUserLogin(char* pInvestor, char* pPwd, char* pBroker)
{
	strcpy_s(_broker, sizeof(_broker), pBroker);
	strcpy_s(_userID, sizeof(_userID), pInvestor);

	CUstpFtdcReqUserLoginField f;
	memset(&f, 0, sizeof(CUstpFtdcReqUserLoginField));
	strcpy_s(f.BrokerID, _broker);
	strcpy_s(f.UserID, _userID);
	strcpy_s(f.Password, sizeof(f.Password), pPwd);
	strcpy_s(f.UserProductInfo, "@Haifeng");
	return api->ReqUserLogin(&f, ++req);
}
DllExport void WINAPI ReqUserLogout()
{
	_session = 0;
	api->RegisterSpi(NULL);
	//api->Release();  //主程序会停止此处
	api = NULL;
}
DllExport const char* WINAPI GetTradingDay()
{
	if (strlen(_TradingDay) == 0)
	{
		strcpy_s(_TradingDay, api->GetTradingDay());
	}
	return _TradingDay;
}
DllExport int WINAPI ReqQryOrder()
{
	CUstpFtdcQryOrderField f;
	memset(&f, 0, sizeof(CUstpFtdcQryOrderField));
	strcpy_s(f.BrokerID, _broker);
	strcpy_s(f.InvestorID, _investor);
	strcpy_s(f.UserID, sizeof(f.UserID), _userID);
	return api->ReqQryOrder(&f, ++req);
}
DllExport int WINAPI ReqQryTrade()
{
	CUstpFtdcQryTradeField f;
	memset(&f, 0, sizeof(CUstpFtdcQryTradeField));
	strcpy_s(f.BrokerID, _broker);
	strcpy_s(f.InvestorID, _investor);
	strcpy_s(f.UserID, sizeof(f.UserID), _userID);
	return api->ReqQryTrade(&f, ++req);
}
DllExport int WINAPI ReqQryPosition()
{
	CUstpFtdcQryInvestorPositionField f;
	memset(&f, 0, sizeof(CUstpFtdcQryInvestorPositionField));
	strcpy_s(f.BrokerID, _broker);
	strcpy_s(f.InvestorID, _investor);
	strcpy_s(f.UserID, sizeof(f.UserID), _userID);
	return api->ReqQryInvestorPosition(&f, ++req);
}
DllExport int WINAPI ReqQryAccount()
{
	CUstpFtdcQryInvestorAccountField f;
	memset(&f, 0, sizeof(CUstpFtdcQryInvestorAccountField));
	strcpy_s(f.BrokerID, _broker);
	strcpy_s(f.InvestorID, _investor);
	strcpy_s(f.UserID, sizeof(f.UserID), _userID);
	return api->ReqQryInvestorAccount(&f, ++req);
}

DllExport int WINAPI ReqOrderInsert(char *pInstrument, DirectionType pDirection, OffsetType pOffset, double pPrice, int pVolume, HedgeType pHedge, OrderType pType)
{
	CUstpFtdcInputOrderField f;
	memset(&f, 0, sizeof(CUstpFtdcInputOrderField));

	strcpy_s(f.InstrumentID, sizeof(f.InstrumentID), pInstrument);
	strcpy_s(f.BrokerID, sizeof(f.BrokerID), _broker);
	switch (pHedge)
	{
	case  Speculation:
		f.HedgeFlag = USTP_FTDC_CHF_Speculation;
		break;
	case  Arbitrage:
		f.HedgeFlag = USTP_FTDC_CHF_Arbitrage;
		break;
	case  Hedge:
		f.HedgeFlag = USTP_FTDC_CHF_Hedge;
		break;
	}
	switch (pDirection)
	{
	case Buy:
		f.Direction = USTP_FTDC_D_Buy;
		break;
	default:
		f.Direction = USTP_FTDC_D_Sell;
		break;
	}
	switch (pOffset)
	{
	case Open:
		f.OffsetFlag = USTP_FTDC_OF_Open;
		break;
	case CloseToday:
		f.OffsetFlag = USTP_FTDC_OF_CloseToday;
		break;
	case  Close:
		f.OffsetFlag = USTP_FTDC_OF_Close;
		break;
	}
	f.Volume = pVolume;
	strcpy_s(f.InvestorID, sizeof(f.InvestorID), _investor);
	strcpy_s(f.UserID, sizeof(f.UserID), _userID);
	f.IsAutoSuspend = 0;
	f.ForceCloseReason = USTP_FTDC_FCR_NotForceClose;
	f.MinVolume = 1;

	f.OrderPriceType = USTP_FTDC_OPT_LimitPrice;
	f.VolumeCondition = USTP_FTDC_VC_AV;
	f.TimeCondition = USTP_FTDC_TC_IOC;
	f.MinVolume = 1;
	f.LimitPrice = pPrice;

	switch (pType)
	{
	case  Limit:
		f.TimeCondition = USTP_FTDC_TC_GFD;
		break;
	case  Market:
		f.OrderPriceType = USTP_FTDC_OPT_AnyPrice;
		f.LimitPrice = 0;
		break;
	case  FAK:
		break;
	case FOK:
		f.VolumeCondition = USTP_FTDC_VC_CV; //全部数量
		break;
	}


	//以当日的当前时间（精确到毫秒）作为标识
	struct tm tBegin;
	tBegin.tm_year = 2014;
	tBegin.tm_mon = 8;
	tBegin.tm_mday = 1;
	tBegin.tm_hour = 0;
	tBegin.tm_min = 0;
	tBegin.tm_sec = 0;
	time_t t = mktime(&tBegin);

	SYSTEMTIME sys;
	GetLocalTime(&sys);
	time_t tt;
	time(&tt);	//秒数

	sprintf_s(f.UserOrderLocalID, "%d%3d", difftime(tt, t), sys.wMilliseconds);

	_ltoa_s(_session, f.UserCustom, 10);
	return api->ReqOrderInsert(&f, ++req);
}
DllExport int WINAPI ReqOrderAction(long pOrderId)
{
	if (_id_order.find(pOrderId) == _id_order.end()) //不存在
	{
		if (_OnRtnError)
		{
			((DefOnRtnError)_OnRtnError)(-1, "OrderActionError:no orderid.");
		}
		return -1;
	}
	if (_id_sysid.find(pOrderId) == _id_sysid.end())
	{
		if (_OnRtnError)
		{
			((DefOnRtnError)_OnRtnError)(-1, "OrderActionError:no sysid.");
		}
		return -1;
	}

	OrderField of = _id_order[pOrderId];// *iter->second;// ._id_order.find(pOrderId);
	if (_id_instrument.find(of.InstrumentID) == _id_instrument.end())
	{
		if (_OnRtnError)
		{
			((DefOnRtnError)_OnRtnError)(-1, "OrderActionError:no instrumentid.");
		}
		return -1;
	}
	CUstpFtdcOrderActionField f;
	memset(&f, 0, sizeof(CUstpFtdcOrderActionField));
	f.ActionFlag = USTP_FTDC_AF_Delete;
	strcpy_s(f.BrokerID, sizeof(f.BrokerID), _broker);
	strcpy_s(f.InvestorID, sizeof(f.InvestorID), _investor);
	strcpy_s(f.UserID, sizeof(f.UserID), _userID);
	strcpy_s(f.ExchangeID, _id_instrument[of.InstrumentID].ExchangeID);
	//UserOrderActionLocalID
	strcpy_s(f.OrderSysID, sizeof(f.OrderSysID), _id_sysid[pOrderId].c_str());

	return api->ReqOrderAction(&f, ++req);
}

void CfemasTrade::OnRspQryUserInvestor(CUstpFtdcRspUserInvestorField *pRspUserInvestor, CUstpFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	strcpy_s(_investor, sizeof(_investor), pRspUserInvestor->InvestorID);
	Sleep(1100);
	CUstpFtdcQryInstrumentField f;
	memset(&f, 0, sizeof(CUstpFtdcQryInstrumentField));
	api->ReqQryInstrument(&f, ++req);
}

void CfemasTrade::OnFrontConnected()
{
	if (_OnFrontConnected)
	{
		((DefOnFrontConnected)_OnFrontConnected)();
	}
}

void CfemasTrade::OnFrontDisconnected(int nReason)
{
	if (_OnRspUserLogout)
	{
		((DefOnRspUserLogout)_OnRspUserLogout)(nReason);
	}
}

void CfemasTrade::OnRspUserLogin(CUstpFtdcRspUserLoginField *pRspUserLogin, CUstpFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if (_OnRspUserLogin)
	{
		SYSTEMTIME sys;
		GetLocalTime(&sys);
		_session = sys.wHour * 100 * 100 * 1000 + sys.wMinute * 100 * 1000 + sys.wSecond * 1000 + sys.wMilliseconds;
		//sprintf_s(_session, "%2d%2d%2d%3d", sys.wHour, sys.wMinute, sys.wSecond, sys.wMilliseconds);
		//_session =  pRspUserLogin->;
		/*CUstpFtdcSettlementInfoConfirmField f;
		memset(&f, 0, sizeof(CUstpFtdcSettlementInfoConfirmField));
		strcpy_s(f.BrokerID, sizeof(f.BrokerID), _broker);
		strcpy_s(f.InvestorID, sizeof(f.InvestorID), _investor);
		api->ReqSettlementInfoConfirm(&f, ++req);*/

		//查投资者
		CUstpFtdcQryUserInvestorField f0;
		strcpy_s(f0.BrokerID, _broker);
		strcpy_s(f0.UserID, _userID);
		api->ReqQryUserInvestor(&f0, ++req);
		((DefOnRspUserLogin)_OnRspUserLogin)(pRspInfo == NULL ? 0 : pRspInfo->ErrorID);
	}
}

void CfemasTrade::OnRspUserLogout(CUstpFtdcRspUserLogoutField *pRspUserLogout, CUstpFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if (_OnRspUserLogout)
	{
		((DefOnRspUserLogout)_OnRspUserLogout)(pRspInfo->ErrorID);
	}
}

//void CfemasTrade::onrsps (CUstpFtdcSettlementInfoConfirmField *pSettlementInfoConfirm, CUstpFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
//{
//	CUstpFtdcQryInstrumentField f;
//	memset(&f, 0, sizeof(CUstpFtdcQryInstrumentField));
//	api->ReqQryInstrument(&f, ++req);
//}
void QryOnLaunch()
{
	Sleep(1200);
	if (_session == 0)
		return;
	ReqQryAccount();

	Sleep(1200);
	if (_session == 0)
		return;
	ReqQryPosition();

	Sleep(1200);
	if (_session == 0)
		return;
	ReqQryOrder();

	Sleep(1200);
	if (_session == 0)
		return;
	ReqQryTrade();
	while (_session != 0)
	{
		ReqQryAccount();
		Sleep(1200);
	}
}
void CfemasTrade::OnRspQryInstrument(CUstpFtdcRspInstrumentField *pInstrument, CUstpFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	InstrumentField f;
	memset(&f, 0, sizeof(InstrumentField));
	if (pInstrument)
	{
		strcpy_s(f.InstrumentID, sizeof(f.InstrumentID), pInstrument->InstrumentID);
		strcpy_s(f.ExchangeID, sizeof(f.ExchangeID), pInstrument->ExchangeID);
		f.PriceTick = pInstrument->PriceTick;
		f.VolumeMultiple = pInstrument->VolumeMultiple;
		strcpy_s(f.ProductID, sizeof(f.ProductID), pInstrument->ProductID);
		_id_instrument[string(f.InstrumentID)] = f;
	}
	if (_OnRspQryInstrument)
	{
		((DefOnRspQryInstrument)_OnRspQryInstrument)(&f, bIsLast);
	}
	if (bIsLast)
	{
		hThread = CreateThread(
			NULL,                                   // SD  
			0,                                  // initial stack size  
			(LPTHREAD_START_ROUTINE)QryOnLaunch,    // thread function  
			NULL,                                    // thread argument  
			0,                                   // creation option  
			NULL//threadID                               // thread identifier  
			);
	}
}

void CfemasTrade::OnRspQryOrder(CUstpFtdcOrderField *pOrder, CUstpFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	OrderField f;
	memset(&f, 0, sizeof(OrderField));
	if (pOrder)
	{
		//f.AvgPrice = pOrder
		f.Direction = pOrder->Direction == USTP_FTDC_D_Buy ? Buy : Sell;
		switch (pOrder->HedgeFlag)
		{
		case  USTP_FTDC_CHF_Speculation:
			f.Hedge = Speculation;
			break;
		case  USTP_FTDC_CHF_Arbitrage:
			f.Hedge = Arbitrage;
			break;
		case  USTP_FTDC_CHF_Hedge:
			f.Hedge = Hedge;
			break;
		}
		switch (pOrder->Direction)
		{
		case USTP_FTDC_D_Buy:
			f.Direction = Buy;
			break;
		default:
			f.Direction = Sell;
			break;
		}
		switch (pOrder->HedgeFlag)
		{
		case USTP_FTDC_OF_Open:
			f.Offset = Open;
			break;
		case USTP_FTDC_OF_CloseToday:
			f.Offset = CloseToday;
			break;
		case  USTP_FTDC_OF_Close:
			f.Offset = Close;
			break;
		}
		strcpy_s(f.InsertTime, sizeof(f.InsertTime), pOrder->InsertTime);
		strcpy_s(f.InstrumentID, sizeof(f.InstrumentID), pOrder->InstrumentID);
		//strcpy_s(f.TradeTime, sizeof(f.TradeTime), pOrder->UpdateTime);
		f.IsLocal = atol(pOrder->UserCustom) == _session;	// pOrder->SessionID == _session;
		f.LimitPrice = pOrder->LimitPrice;
		//即时返回委托编号,由上层处理
		//if (strlen(pOrder->OrderRef) == 9) //以hhmmssfff格式的标识
		//	f.OrderId = atoi(pOrder->OrderRef);
		//else
		f.OrderId = atol(pOrder->OrderLocalID);
		switch (pOrder->OrderStatus)
		{
		case USTP_FTDC_OS_Canceled:
			f.Status = Canceled;
			break;
		case USTP_FTDC_OS_AllTraded:
			f.Status = Filled;
			break;
		case USTP_FTDC_OS_PartTradedQueueing:
			f.Status = Partial;
			break;
		default:
			f.Status = Normal;
			break;
		}
		f.Volume = pOrder->Volume;// pOrder->VolumeTotalOriginal;
		//f.VolumeLeft =pOrder->VolumeTotal;	//order响应中为实际剩余
		f.VolumeLeft = f.Volume;	//需要计算均价用
		_id_order[f.OrderId] = f;
	}
	if (_OnRspQryOrder)
	{
		((DefOnRspQryOrder)_OnRspQryOrder)(&f, bIsLast);
	}
}

void CfemasTrade::OnRspQryTrade(CUstpFtdcTradeField *pTrade, CUstpFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	TradeField f;
	memset(&f, 0, sizeof(TradeField));
	if (pTrade)
	{//精确到毫秒的时间差
		long id = atol(pTrade->UserOrderLocalID);//->OrderLocalID);

		if (_id_order.find(id) != _id_order.end())
		{
			OrderField f = _id_order[id];
			f.AvgPrice = ((f.AvgPrice*(f.Volume - f.VolumeLeft)) + pTrade->TradePrice *pTrade->TradeVolume) / (f.Volume - f.VolumeLeft + pTrade->TradeVolume);
			strcpy_s(f.TradeTime, 16, pTrade->TradeTime);
			_id_order[id] = f;
			//f.TradeVolume = pTrade->Volume;
			//f.VolumeLeft -= f.TradeVolume;
			//((DefOnRtnOrder)_OnRtnOrder)(&f);
		}

		f.OrderID = id;

		switch (pTrade->HedgeFlag)
		{
		case  USTP_FTDC_CHF_Speculation:
			f.Hedge = Speculation;
			break;
		case  USTP_FTDC_CHF_Arbitrage:
			f.Hedge = Arbitrage;
			break;
		case  USTP_FTDC_CHF_Hedge:
			f.Hedge = Hedge;
			break;
		}
		switch (pTrade->Direction)
		{
		case USTP_FTDC_D_Buy:
			f.Direction = Buy;
			break;
		default:
			f.Direction = Sell;
			break;
		}
		switch (pTrade->OffsetFlag)
		{
		case USTP_FTDC_OF_Open:
			f.Offset = Open;
			break;
		case USTP_FTDC_OF_CloseToday:
			f.Offset = CloseToday;
			break;
		case  USTP_FTDC_OF_Close:
			f.Offset = Close;
			break;
		}
		strcpy_s(f.ExchangeID, sizeof(f.ExchangeID), pTrade->ExchangeID);
		strcpy_s(f.InstrumentID, sizeof(f.InstrumentID), pTrade->InstrumentID);
		f.OrderID = atol(pTrade->UserOrderLocalID);//  OrderLocalID);
		f.Price = pTrade->TradePrice;
		strcpy_s(f.TradeID, sizeof(f.TradeID), pTrade->TradeID);
		strcpy_s(f.TradeTime, sizeof(f.TradeTime), pTrade->TradeTime);
		strcpy_s(f.TradingDay, sizeof(f.TradingDay), pTrade->TradingDay);
		f.Volume = pTrade->TradeVolume;
	}
	if (_OnRspQryTrade)
	{
		((DefOnRspQryTrade)_OnRspQryTrade)(&f, bIsLast);
	}
}

void CfemasTrade::OnRspQryInvestorPosition(CUstpFtdcRspInvestorPositionField *pInvestorPosition, CUstpFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if (_OnRspQryPosition)
	{
		PositionField f;
		memset(&f, 0, sizeof(PositionField));
		if (pInvestorPosition)
		{
			switch (pInvestorPosition->HedgeFlag)
			{
			case  USTP_FTDC_CHF_Speculation:
				f.Hedge = Speculation;
				break;
			case  USTP_FTDC_CHF_Arbitrage:
				f.Hedge = Arbitrage;
				break;
			case  USTP_FTDC_CHF_Hedge:
				f.Hedge = Hedge;
				break;
			}
			switch (pInvestorPosition->Direction)
			{
			case USTP_FTDC_D_Buy:
				f.Direction = Buy;
				break;
			default:
				f.Direction = Sell;
				break;
			}

			strcpy_s(f.InstrumentID, sizeof(f.InstrumentID), pInvestorPosition->InstrumentID);
			//f.Margin = pInvestorPosition->UseMargin;
			f.Price = pInvestorPosition->Position == 0 ? 0 : pInvestorPosition->PositionCost / pInvestorPosition->Position;// (pInvestorPosition->PositionCost / _id_instrument[pInvestorPosition->InstrumentID].VolumeMultiple / pInvestorPosition->Position);
			f.Position = pInvestorPosition->Position;
			f.YdPosition = pInvestorPosition->YdPosition;
			f.TdPosition = f.Position - f.YdPosition;
		}
		((DefOnRspQryPosition)_OnRspQryPosition)(&f, bIsLast);
	}
}

void CfemasTrade::OnRspQryInvestorAccount(CUstpFtdcRspInvestorAccountField *pTradingAccount, CUstpFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if (_OnRspQryTradingAccount)
	{
		TradingAccount f;
		memset(&f, 0, sizeof(TradingAccount));
		f.Available = pTradingAccount->Available;
		f.CloseProfit = pTradingAccount->CloseProfit;
		f.Commission = pTradingAccount->Fee;//->Commission;
		f.CurrMargin = pTradingAccount->Margin;//->CurrMargin;
		f.FrozenCash = pTradingAccount->FrozenFee + pTradingAccount->FrozenMargin;//->FrozenCash;
		f.PositionProfit = pTradingAccount->PositionProfit;
		f.PreBalance = pTradingAccount->PreBalance;
		f.Fund = f.PreBalance + f.CloseProfit + f.PositionProfit + pTradingAccount->Deposit - pTradingAccount->Withdraw;
		((DefOnRspQryTradingAccount)_OnRspQryTradingAccount)(&f);
	}
}

void CfemasTrade::OnRtnInstrumentStatus(CUstpFtdcInstrumentStatusField *pInstrumentStatus)
{
	if (_OnRtnExchangeStatus)
	{
		ExchangeStatusType status = BeforeTrading;
		switch (pInstrumentStatus->InstrumentStatus)
		{
			/*case  USTP_FTDC_IS_AuctionBalance:
			case  USTP_FTDC_IS_AuctionMatch:
			case USTP_FTDC_IS_BeforeTrading:*/
		case USTP_FTDC_IS_Continous:
			status = Trading;
			break;
		case USTP_FTDC_IS_Closed:
			status = Closed;
			break;
		case USTP_FTDC_IS_NoTrading:
			status = NoTrading;
			break;
		}
		((DefOnRtnExchangeStatus)_OnRtnExchangeStatus)(pInstrumentStatus->InstrumentID, status);
	}
}

void CfemasTrade::OnRspError(CUstpFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if (_OnRtnError && pRspInfo)
	{
		((DefOnRtnError)_OnRtnError)(pRspInfo->ErrorID, pRspInfo->ErrorMsg);
	}
}

//void CfemasTrade::OnRtnTradingNotice(CUstpFtdcTradingNoticeInfoField *pTradingNoticeInfo)
//{
//	if (_OnRtnNotice)
//	{
//		((DefOnRtnNotice)_OnRtnNotice)(pTradingNoticeInfo->FieldContent);
//	}
//}


void CfemasTrade::OnRtnOrder(CUstpFtdcOrderField *pOrder)
{
	long id = atol(pOrder->OrderLocalID);
	OrderField f;
	memset(&f, 0, sizeof(OrderField));

	switch (pOrder->HedgeFlag)
	{
	case  USTP_FTDC_CHF_Speculation:
		f.Hedge = Speculation;
		break;
	case  USTP_FTDC_CHF_Arbitrage:
		f.Hedge = Arbitrage;
		break;
	case  USTP_FTDC_CHF_Hedge:
		f.Hedge = Hedge;
		break;
	}
	switch (pOrder->Direction)
	{
	case USTP_FTDC_D_Buy:
		f.Direction = Buy;
		break;
	default:
		f.Direction = Sell;
		break;
	}
	switch (pOrder->OffsetFlag)
	{
	case USTP_FTDC_OF_Open:
		f.Offset = Open;
		break;
	case USTP_FTDC_OF_CloseToday:
		f.Offset = CloseToday;
		break;
	case  USTP_FTDC_OF_Close:
		f.Offset = Close;
		break;
	}
	strcpy_s(f.InsertTime, sizeof(f.InsertTime), pOrder->InsertTime);
	strcpy_s(f.InstrumentID, sizeof(f.InstrumentID), pOrder->InstrumentID);
	strcpy_s(f.TradeTime, sizeof(f.TradeTime), pOrder->InsertTime);
	f.IsLocal = atol(pOrder->UserCustom) == _session;
	f.LimitPrice = pOrder->LimitPrice;
	f.OrderId = id;
	switch (pOrder->OrderStatus)
	{
	case USTP_FTDC_OS_Canceled:
		f.Status = Canceled;
		break;
	case USTP_FTDC_OS_AllTraded:
		f.Status = Filled;
		break;
	case USTP_FTDC_OS_PartTradedQueueing:
		f.Status = Partial;
		break;
	default:
		f.Status = Normal;
		break;
	}
	f.Volume = pOrder->Volume;
	//f->VolumeLeft = pOrder->VolumeTotal; //由ontrade处理
	if (pOrder->OrderSysID && strlen(pOrder->OrderSysID) > 0)
	{
		_id_sysid[id] = string(pOrder->OrderSysID);  //撤单用
	}

	bool first = _id_order.find(id) == _id_order.end();
	if (first)//首次处理
	{
		f.VolumeLeft = f.Volume;// pOrder->VolumeTotal;
	}
	_id_order[id] = f;
	if (_OnRtnOrder && first)
	{
		((DefOnRtnOrder)_OnRtnOrder)(&f);
	}
	if (f.Status == Canceled)
	{
		if (_OnRtnCancel)
			((DefOnRtnCancel)_OnRtnCancel)(&f);
		/*if (_OnRtnError && strstr(pOrder->StatusMsg, "被拒绝") != NULL)
		{
		char msg[512];
		sprintf_s(msg, "OrderInsertError(id:%d):%s", f.OrderId, pOrder->StatusMsg);
		((DefOnRtnError)_OnRtnError)(-1, msg);
		}*/
	}
}

void CfemasTrade::OnRtnTrade(CUstpFtdcTradeField *pTrade)
{
	long id = atol(pTrade->UserOrderLocalID);
	if (_id_order.find(id) != _id_order.end())
	{
		OrderField f = _id_order[id];

		strcpy_s(f.TradeTime, 16, pTrade->TradeTime);
		f.AvgPrice = ((f.AvgPrice*(f.Volume - f.VolumeLeft)) + pTrade->TradePrice*pTrade->TradeVolume) / (f.Volume - f.VolumeLeft + pTrade->TradeVolume);
		f.TradeVolume = pTrade->TradeVolume;
		f.VolumeLeft -= f.TradeVolume;
		if (f.VolumeLeft == 0)
			f.Status = Filled;
		else
			f.Status = Partial;
		_id_order[id] = f;
		if (_OnRtnOrder)
		{
			((DefOnRtnOrder)_OnRtnOrder)(&f);
		}
	}
	if (_OnRtnTrade)
	{
		TradeField f;
		memset(&f, 0, sizeof(TradeField));
		f.OrderID = id;

		switch (pTrade->HedgeFlag)
		{
		case  USTP_FTDC_CHF_Speculation:
			f.Hedge = Speculation;
			break;
		case  USTP_FTDC_CHF_Arbitrage:
			f.Hedge = Arbitrage;
			break;
		case  USTP_FTDC_CHF_Hedge:
			f.Hedge = Hedge;
			break;
		}
		switch (pTrade->Direction)
		{
		case USTP_FTDC_D_Buy:
			f.Direction = Buy;
			break;
		default:
			f.Direction = Sell;
			break;
		}
		switch (pTrade->OffsetFlag)
		{
		case USTP_FTDC_OF_Open:
			f.Offset = Open;
			break;
		case USTP_FTDC_OF_CloseToday:
			f.Offset = CloseToday;
			break;
		case  USTP_FTDC_OF_Close:
			f.Offset = Close;
			break;
		}
		strcpy_s(f.ExchangeID, sizeof(f.ExchangeID), pTrade->ExchangeID);
		strcpy_s(f.InstrumentID, sizeof(f.InstrumentID), pTrade->InstrumentID);
		f.OrderID = atol(pTrade->UserOrderLocalID);
		f.Price = pTrade->TradePrice;
		strcpy_s(f.TradeID, sizeof(f.TradeID), pTrade->TradeID);
		strcpy_s(f.TradeTime, sizeof(f.TradeTime), pTrade->TradeTime);
		strcpy_s(f.TradingDay, sizeof(f.TradingDay), pTrade->TradingDay);
		f.Volume = pTrade->TradeVolume;
		((DefOnRtnTrade)_OnRtnTrade)(&f);
	}
}

void CfemasTrade::OnRspOrderInsert(CUstpFtdcInputOrderField *pInputOrder, CUstpFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if (_OnRtnError)
	{
		char msg[512];
		sprintf_s(msg, "%s:%s", "OrderInsertError:", pRspInfo->ErrorMsg);
		((DefOnRtnError)_OnRtnError)(pRspInfo == NULL ? -1 : pRspInfo->ErrorID, msg);
	}
}

void CfemasTrade::OnErrRtnOrderInsert(CUstpFtdcInputOrderField *pInputOrder, CUstpFtdcRspInfoField *pRspInfo)
{
	if (_OnRtnError)
	{
		char msg[512];
		sprintf_s(msg, "%s:%s", "OrderInsertError:", pRspInfo->ErrorMsg);
		((DefOnRtnError)_OnRtnError)(pRspInfo == NULL ? -1 : pRspInfo->ErrorID, msg);
	}
}

void CfemasTrade::OnRspOrderAction(CUstpFtdcOrderActionField *pInputOrderAction, CUstpFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if (_OnRtnError)
	{
		char msg[512];
		sprintf_s(msg, "%s:%s", "OrderActionError:", pRspInfo->ErrorMsg);
		((DefOnRtnError)_OnRtnError)(pRspInfo == NULL ? -1 : pRspInfo->ErrorID, msg);
	}
}

void CfemasTrade::OnErrRtnOrderAction(CUstpFtdcOrderActionField *pOrderAction, CUstpFtdcRspInfoField *pRspInfo)
{
	if (_OnRtnError)
	{
		char msg[512];
		sprintf_s(msg, "%s:%s", "OrderActionError:", pRspInfo->ErrorMsg);
		((DefOnRtnError)_OnRtnError)(pRspInfo == NULL ? -1 : pRspInfo->ErrorID, msg);
	}
}



