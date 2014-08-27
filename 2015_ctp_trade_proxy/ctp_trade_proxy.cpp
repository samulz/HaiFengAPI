// ctp_trade_proxy.cpp : 定义 DLL 应用程序的导出函数。
//

#include "ctp_trade_proxy.h"
#include "../2015_include_lib/HFTrade.h"

CctpTrade *spi;
CThostFtdcTraderApi *api;
map<string, InstrumentField> _id_instrument;
map<long, string> _id_sysid; //orderid&sysid
int _session = -1;
HANDLE hThread;//启动时查询用

DllExport void WINAPI CreateApi()
{
	api = CThostFtdcTraderApi::CreateFtdcTraderApi("./log/");

}
DllExport int WINAPI ReqConnect(char *pFront)
{
	spi = new CctpTrade();
	api->RegisterFront(pFront);
	api->RegisterSpi(spi);
	api->SubscribePrivateTopic(THOST_TERT_QUICK);	//私有流用quick
	api->SubscribePublicTopic(THOST_TERT_RESTART);	//公有流用public
	api->Init();
	return 0;
}
DllExport int WINAPI ReqUserLogin(char* pInvestor, char* pPwd, char* pBroker)
{
	CThostFtdcReqUserLoginField f;
	memset(&f, 0, sizeof(CThostFtdcReqUserLoginField));
	strcpy_s(f.BrokerID, sizeof(f.BrokerID), pBroker);
	strcpy_s(f.UserID, sizeof(f.UserID), pInvestor);
	strcpy_s(f.Password, sizeof(f.Password), pPwd);
	strcpy_s(f.UserProductInfo, "@Haifeng");
	strcpy_s(_broker, f.BrokerID);
	strcpy_s(_investor, f.UserID);
	return api->ReqUserLogin(&f, ++req);
}
DllExport void WINAPI ReqUserLogout()
{
	_session = 0;
	api->RegisterSpi(NULL);
	api->Release();
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
	CThostFtdcQryOrderField f;
	memset(&f, 0, sizeof(CThostFtdcQryOrderField));
	strcpy_s(f.BrokerID, _broker);
	strcpy_s(f.InvestorID, _investor);
	return api->ReqQryOrder(&f, ++req);
}
DllExport int WINAPI ReqQryTrade()
{
	CThostFtdcQryTradeField f;
	memset(&f, 0, sizeof(CThostFtdcQryTradeField));
	strcpy_s(f.BrokerID, _broker);
	strcpy_s(f.InvestorID, _investor);
	return api->ReqQryTrade(&f, ++req);
}
DllExport int WINAPI ReqQryPosition()
{
	CThostFtdcQryInvestorPositionField f;
	memset(&f, 0, sizeof(CThostFtdcQryInvestorPositionField));
	strcpy_s(f.BrokerID, _broker);
	strcpy_s(f.InvestorID, _investor);
	return api->ReqQryInvestorPosition(&f, ++req);
}
DllExport int WINAPI ReqQryAccount()
{
	CThostFtdcQryTradingAccountField f;
	memset(&f, 0, sizeof(CThostFtdcQryTradingAccountField));
	strcpy_s(f.BrokerID, _broker);
	strcpy_s(f.InvestorID, _investor);
	return api->ReqQryTradingAccount(&f, ++req);
}

DllExport int WINAPI ReqOrderInsert(char *pInstrument, DirectionType pDirection, OffsetType pOffset, double pPrice, int pVolume, HedgeType pHedge, OrderType pType)
{
	CThostFtdcInputOrderField f;
	memset(&f, 0, sizeof(CThostFtdcInputOrderField));

	strcpy_s(f.InstrumentID, sizeof(f.InstrumentID), pInstrument);
	strcpy_s(f.BrokerID, sizeof(f.BrokerID), _broker);
	switch (pHedge)
	{
	case  Speculation:
		f.CombHedgeFlag[0] = THOST_FTDC_HF_Speculation;
		break;
	case  Arbitrage:
		f.CombHedgeFlag[0] = THOST_FTDC_HF_Arbitrage;
		break;
	case  Hedge:
		f.CombHedgeFlag[0] = THOST_FTDC_HF_Hedge;
		break;
	}
	switch (pDirection)
	{
	case Buy:
		f.Direction = THOST_FTDC_D_Buy;
		break;
	default:
		f.Direction = THOST_FTDC_D_Sell;
		break;
	}
	switch (pOffset)
	{
	case Open:
		f.CombOffsetFlag[0] = THOST_FTDC_OF_Open;
		break;
	case CloseToday:
		f.CombOffsetFlag[0] = THOST_FTDC_OF_CloseToday;
		break;
	case  Close:
		f.CombOffsetFlag[0] = THOST_FTDC_OF_Close;
		break;
	}
	f.VolumeTotalOriginal = pVolume;
	strcpy_s(f.InvestorID, sizeof(f.InvestorID), _investor);
	f.IsAutoSuspend = 0;

	f.ContingentCondition = THOST_FTDC_CC_Immediately;
	f.ForceCloseReason = THOST_FTDC_FCC_NotForceClose;
	f.IsSwapOrder = 0;
	f.UserForceClose = 0;

	f.OrderPriceType = THOST_FTDC_OPT_LimitPrice;
	f.VolumeCondition = THOST_FTDC_VC_AV;
	f.TimeCondition = THOST_FTDC_TC_IOC;
	f.MinVolume = 1;
	f.LimitPrice = pPrice;

	switch (pType)
	{
	case  Limit:
		f.TimeCondition = THOST_FTDC_TC_GFD;
		break;
	case  Market:
		f.OrderPriceType = THOST_FTDC_OPT_AnyPrice;
		f.LimitPrice = 0;
		break;
	case  FAK:
		break;
	case FOK:
		f.VolumeCondition = THOST_FTDC_VC_CV; //全部数量
		break;
	}

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
	CThostFtdcInputOrderActionField f;
	memset(&f, 0, sizeof(CThostFtdcInputOrderActionField));
	f.ActionFlag = THOST_FTDC_AF_Delete;
	strcpy_s(f.BrokerID, sizeof(f.BrokerID), _broker);
	strcpy_s(f.InvestorID, sizeof(f.InvestorID), _investor);
	strcpy_s(f.ExchangeID, _id_instrument[of.InstrumentID].ExchangeID);
	strcpy_s(f.OrderSysID, sizeof(f.OrderSysID), _id_sysid[pOrderId].c_str());
	return api->ReqOrderAction(&f, ++req);
}

void CctpTrade::OnFrontConnected()
{
	if (_OnFrontConnected)
	{
		((DefOnFrontConnected)_OnFrontConnected)();
	}
}

void CctpTrade::OnFrontDisconnected(int nReason)
{
	if (_OnRspUserLogout)
	{
		((DefOnRspUserLogout)_OnRspUserLogout)(nReason);
	}
}

void CctpTrade::OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if (_OnRspUserLogin)
	{
		_session = pRspUserLogin->SessionID;
		CThostFtdcSettlementInfoConfirmField f;
		memset(&f, 0, sizeof(CThostFtdcSettlementInfoConfirmField));
		strcpy_s(f.BrokerID, sizeof(f.BrokerID), _broker);
		strcpy_s(f.InvestorID, sizeof(f.InvestorID), _investor);
		api->ReqSettlementInfoConfirm(&f, ++req);
		((DefOnRspUserLogin)_OnRspUserLogin)(pRspInfo == NULL ? 0 : pRspInfo->ErrorID);
	}
}

void CctpTrade::OnRspSettlementInfoConfirm(CThostFtdcSettlementInfoConfirmField *pSettlementInfoConfirm, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	CThostFtdcQryInstrumentField f;
	memset(&f, 0, sizeof(CThostFtdcQryInstrumentField));
	api->ReqQryInstrument(&f, ++req);
}
void QryOnLaunch()
{
	Sleep(1200);
	if(_session == 0)
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
void CctpTrade::OnRspQryInstrument(CThostFtdcInstrumentField *pInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
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

void CctpTrade::OnRspQryOrder(CThostFtdcOrderField *pOrder, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	OrderField f;
	memset(&f, 0, sizeof(OrderField));
	if (pOrder)
	{
		//f.AvgPrice = pOrder
		f.Direction = pOrder->Direction == THOST_FTDC_D_Buy ? Buy : Sell;
		switch (pOrder->CombHedgeFlag[0])
		{
		case  THOST_FTDC_HF_Speculation:
			f.Hedge = Speculation;
			break;
		case  THOST_FTDC_HF_Arbitrage:
			f.Hedge = Arbitrage;
			break;
		case  THOST_FTDC_HF_Hedge:
			f.Hedge = Hedge;
			break;
		}
		switch (pOrder->Direction)
		{
		case THOST_FTDC_D_Buy:
			f.Direction = Buy;
			break;
		default:
			f.Direction = Sell;
			break;
		}
		switch (pOrder->CombOffsetFlag[0])
		{
		case THOST_FTDC_OF_Open:
			f.Offset = Open;
			break;
		case THOST_FTDC_OF_CloseToday:
			f.Offset = CloseToday;
			break;
		case  THOST_FTDC_OF_Close:
			f.Offset = Close;
			break;
		}
		strcpy_s(f.InsertTime, sizeof(f.InsertTime), pOrder->InsertTime);
		strcpy_s(f.InstrumentID, sizeof(f.InstrumentID), pOrder->InstrumentID);
		//strcpy_s(f.TradeTime, sizeof(f.TradeTime), pOrder->UpdateTime);
		f.IsLocal = pOrder->SessionID == _session;
		f.LimitPrice = pOrder->LimitPrice;
		//即时返回委托编号,由上层处理
		//if (strlen(pOrder->OrderRef) == 9) //以hhmmssfff格式的标识
		//	f.OrderId = atoi(pOrder->OrderRef);
		//else
		f.OrderId = atoi(pOrder->OrderLocalID);
		switch (pOrder->OrderStatus)
		{
		case THOST_FTDC_OST_Canceled:
			f.Status = Canceled;
			break;
		case THOST_FTDC_OST_AllTraded:
			f.Status = Filled;
			break;
		case THOST_FTDC_OST_PartTradedQueueing:
			f.Status = Partial;
			break;
		default:
			f.Status = Normal;
			break;
		}
		f.Volume = pOrder->VolumeTotalOriginal;
		//f.VolumeLeft =pOrder->VolumeTotal;	//order响应中为实际剩余
		f.VolumeLeft = f.Volume;	//需要计算均价用
		_id_order[f.OrderId] = f;
	}
	if (_OnRspQryOrder)
	{
		((DefOnRspQryOrder)_OnRspQryOrder)(&f, bIsLast);
	}
}

void CctpTrade::OnRspQryTrade(CThostFtdcTradeField *pTrade, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	TradeField f;
	memset(&f, 0, sizeof(TradeField));
	if (pTrade)
	{
		long id = atol(pTrade->OrderLocalID);

		if (_id_order.find(id) != _id_order.end())
		{
			OrderField f = _id_order[id];
			f.AvgPrice = ((f.AvgPrice*(f.Volume - f.VolumeLeft)) + pTrade->Price*pTrade->Volume) / (f.Volume - f.VolumeLeft + pTrade->Volume);
			strcpy_s(f.TradeTime, 16, pTrade->TradeTime);
			_id_order[id] = f;
			//f.TradeVolume = pTrade->Volume;
			//f.VolumeLeft -= f.TradeVolume;
			//((DefOnRtnOrder)_OnRtnOrder)(&f);
		}

		f.OrderID = id;

		switch (pTrade->HedgeFlag)
		{
		case  THOST_FTDC_HF_Speculation:
			f.Hedge = Speculation;
			break;
		case  THOST_FTDC_HF_Arbitrage:
			f.Hedge = Arbitrage;
			break;
		case  THOST_FTDC_HF_Hedge:
			f.Hedge = Hedge;
			break;
		}
		switch (pTrade->Direction)
		{
		case THOST_FTDC_D_Buy:
			f.Direction = Buy;
			break;
		default:
			f.Direction = Sell;
			break;
		}
		switch (pTrade->OffsetFlag)
		{
		case THOST_FTDC_OF_Open:
			f.Offset = Open;
			break;
		case THOST_FTDC_OF_CloseToday:
			f.Offset = CloseToday;
			break;
		case  THOST_FTDC_OF_Close:
			f.Offset = Close;
			break;
		}
		strcpy_s(f.ExchangeID, sizeof(f.ExchangeID), pTrade->ExchangeID);
		strcpy_s(f.InstrumentID, sizeof(f.InstrumentID), pTrade->InstrumentID);
		f.OrderID = atol(pTrade->OrderLocalID);
		f.Price = pTrade->Price;
		strcpy_s(f.TradeID, sizeof(f.TradeID), pTrade->TradeID);
		strcpy_s(f.TradeTime, sizeof(f.TradeTime), pTrade->TradeTime);
		strcpy_s(f.TradingDay, sizeof(f.TradingDay), pTrade->TradingDay);
		f.Volume = pTrade->Volume;
	}
	if (_OnRspQryTrade)
	{
		((DefOnRspQryTrade)_OnRspQryTrade)(&f, bIsLast);
	}
}

void CctpTrade::OnRspQryInvestorPosition(CThostFtdcInvestorPositionField *pInvestorPosition, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if (_OnRspQryPosition)
	{
		PositionField f;
		memset(&f, 0, sizeof(PositionField));
		if (pInvestorPosition)
		{
			switch (pInvestorPosition->HedgeFlag)
			{
			case  THOST_FTDC_HF_Speculation:
				f.Hedge = Speculation;
				break;
			case  THOST_FTDC_HF_Arbitrage:
				f.Hedge = Arbitrage;
				break;
			case  THOST_FTDC_HF_Hedge:
				f.Hedge = Hedge;
				break;
			}
			switch (pInvestorPosition->PosiDirection)
			{
			case THOST_FTDC_PD_Long:
				f.Direction = Buy;
				break;
			default:
				f.Direction = Sell;
				break;
			}

			strcpy_s(f.InstrumentID, sizeof(f.InstrumentID), pInvestorPosition->InstrumentID);
			//f.Margin = pInvestorPosition->UseMargin;
			f.Price = pInvestorPosition->Position == 0 ? 0 : (pInvestorPosition->PositionCost / _id_instrument[pInvestorPosition->InstrumentID].VolumeMultiple / pInvestorPosition->Position);
			f.Position = pInvestorPosition->Position;
			f.TdPosition = pInvestorPosition->TodayPosition;
			f.YdPosition = f.Position - f.TdPosition; //pInvestorPosition->YdPosition; 平仓后不知如何计算
		}
		((DefOnRspQryPosition)_OnRspQryPosition)(&f, bIsLast);
	}
}

void CctpTrade::OnRspQryTradingAccount(CThostFtdcTradingAccountField *pTradingAccount, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if (_OnRspQryTradingAccount)
	{
		TradingAccount f;
		memset(&f, 0, sizeof(TradingAccount));
		f.Available = pTradingAccount->Available;
		f.CloseProfit = pTradingAccount->CloseProfit;
		f.Commission = pTradingAccount->Commission;
		f.CurrMargin = pTradingAccount->CurrMargin;
		f.FrozenCash = pTradingAccount->FrozenCash;
		f.PositionProfit = pTradingAccount->PositionProfit;
		f.PreBalance = pTradingAccount->PreBalance;
		f.Fund = f.PreBalance + f.CloseProfit + f.PositionProfit + pTradingAccount->Deposit - pTradingAccount->Withdraw;
		((DefOnRspQryTradingAccount)_OnRspQryTradingAccount)(&f);
	}
}

void CctpTrade::OnRtnInstrumentStatus(CThostFtdcInstrumentStatusField *pInstrumentStatus)
{
	if (_OnRtnExchangeStatus)
	{
		ExchangeStatusType status = BeforeTrading;
		switch (pInstrumentStatus->InstrumentStatus)
		{
			/*case  THOST_FTDC_IS_AuctionBalance:
				case  THOST_FTDC_IS_AuctionMatch:
				case THOST_FTDC_IS_BeforeTrading:*/
		case THOST_FTDC_IS_Continous:
			status = Trading;
			break;
		case THOST_FTDC_IS_Closed:
			status = Closed;
			break;
		case THOST_FTDC_IS_NoTrading:
			status = NoTrading;
			break;
		}
		((DefOnRtnExchangeStatus)_OnRtnExchangeStatus)(pInstrumentStatus->InstrumentID, status);
	}
}

void CctpTrade::OnRspError(CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if (_OnRtnError && pRspInfo)
	{
		((DefOnRtnError)_OnRtnError)(pRspInfo->ErrorID, pRspInfo->ErrorMsg);
	}
}

void CctpTrade::OnRtnTradingNotice(CThostFtdcTradingNoticeInfoField *pTradingNoticeInfo)
{
	if (_OnRtnNotice)
	{
		((DefOnRtnNotice)_OnRtnNotice)(pTradingNoticeInfo->FieldContent);
	}
}


void CctpTrade::OnRtnOrder(CThostFtdcOrderField *pOrder)
{
	long id = atol(pOrder->OrderLocalID);
	OrderField f;
	bool first = _id_order.find(id) == _id_order.end();
	if (first)
	{
		memset(&f, 0, sizeof(OrderField));

		switch (pOrder->CombHedgeFlag[0])
		{
		case  THOST_FTDC_HF_Speculation:
			f.Hedge = Speculation;
			break;
		case  THOST_FTDC_HF_Arbitrage:
			f.Hedge = Arbitrage;
			break;
		case  THOST_FTDC_HF_Hedge:
			f.Hedge = Hedge;
			break;
		}
		switch (pOrder->Direction)
		{
		case THOST_FTDC_D_Buy:
			f.Direction = Buy;
			break;
		default:
			f.Direction = Sell;
			break;
		}
		switch (pOrder->CombOffsetFlag[0])
		{
		case THOST_FTDC_OF_Open:
			f.Offset = Open;
			break;
		case THOST_FTDC_OF_CloseToday:
			f.Offset = CloseToday;
			break;
		case  THOST_FTDC_OF_Close:
			f.Offset = Close;
			break;
		}
		strcpy_s(f.InsertTime, sizeof(f.InsertTime), pOrder->InsertTime);
		strcpy_s(f.InstrumentID, sizeof(f.InstrumentID), pOrder->InstrumentID);
		//strcpy_s(f.TradeTime, sizeof(f.TradeTime), pOrder->UpdateTime);
		f.IsLocal = pOrder->SessionID == _session;
		f.LimitPrice = pOrder->LimitPrice;
		f.OrderId = id;
		f.Volume = pOrder->VolumeTotalOriginal;
		f.VolumeLeft = f.Volume;// pOrder->VolumeTotal;
		//f->VolumeLeft = pOrder->VolumeTotal; //由ontrade处理
		f.Status = Normal;
		if (_OnRtnOrder)
		{
			((DefOnRtnOrder)_OnRtnOrder)(&f);
		}
	}
	else
		f = _id_order[id];

	switch (pOrder->OrderStatus)
	{
	case THOST_FTDC_OST_Canceled:
		f.Status = Canceled;
		break;
	case THOST_FTDC_OST_AllTraded:
		f.Status = Filled;
		break;
	case THOST_FTDC_OST_PartTradedQueueing:
		f.Status = Partial;
		break;
	default:
		f.Status = Normal;
		break;
	}
	if (pOrder->OrderSysID && strlen(pOrder->OrderSysID) > 0)
	{
		_id_sysid[id] = string(pOrder->OrderSysID);  //撤单用
	}

	_id_order[id] = f; //数据更新

	if (f.Status == Canceled)
	{
		if (_OnRtnCancel)
			((DefOnRtnCancel)_OnRtnCancel)(&f);
		if (_OnRtnError && strstr(pOrder->StatusMsg, "被拒绝") != NULL)
		{
			char msg[512];
			sprintf_s(msg, "OrderInsertError(id:%d):%s", f.OrderId, pOrder->StatusMsg);
			((DefOnRtnError)_OnRtnError)(-1, msg);
		}
	}
}

void CctpTrade::OnRtnTrade(CThostFtdcTradeField *pTrade)
{
	long id = atol(pTrade->OrderLocalID);
	if (_id_order.find(id) != _id_order.end())
	{
		OrderField f = _id_order[id];

		strcpy_s(f.TradeTime, 16, pTrade->TradeTime);
		f.AvgPrice = ((f.AvgPrice*(f.Volume - f.VolumeLeft)) + pTrade->Price*pTrade->Volume) / (f.Volume - f.VolumeLeft + pTrade->Volume);
		f.TradeVolume = pTrade->Volume;
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
		case  THOST_FTDC_HF_Speculation:
			f.Hedge = Speculation;
			break;
		case  THOST_FTDC_HF_Arbitrage:
			f.Hedge = Arbitrage;
			break;
		case  THOST_FTDC_HF_Hedge:
			f.Hedge = Hedge;
			break;
		}
		switch (pTrade->Direction)
		{
		case THOST_FTDC_D_Buy:
			f.Direction = Buy;
			break;
		default:
			f.Direction = Sell;
			break;
		}
		switch (pTrade->OffsetFlag)
		{
		case THOST_FTDC_OF_Open:
			f.Offset = Open;
			break;
		case THOST_FTDC_OF_CloseToday:
			f.Offset = CloseToday;
			break;
		case  THOST_FTDC_OF_Close:
			f.Offset = Close;
			break;
		}
		strcpy_s(f.ExchangeID, sizeof(f.ExchangeID), pTrade->ExchangeID);
		strcpy_s(f.InstrumentID, sizeof(f.InstrumentID), pTrade->InstrumentID);
		f.OrderID = atol(pTrade->OrderLocalID);
		f.Price = pTrade->Price;
		strcpy_s(f.TradeID, sizeof(f.TradeID), pTrade->TradeID);
		strcpy_s(f.TradeTime, sizeof(f.TradeTime), pTrade->TradeTime);
		strcpy_s(f.TradingDay, sizeof(f.TradingDay), pTrade->TradingDay);
		f.Volume = pTrade->Volume;
		((DefOnRtnTrade)_OnRtnTrade)(&f);
	}
}

void CctpTrade::OnRspOrderInsert(CThostFtdcInputOrderField *pInputOrder, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if (_OnRtnError)
	{
		char msg[512];
		sprintf_s(msg, "%s:%s", "OrderInsertError:", pRspInfo->ErrorMsg);
		((DefOnRtnError)_OnRtnError)(pRspInfo == NULL ? -1 : pRspInfo->ErrorID, msg);
	}
}

void CctpTrade::OnErrRtnOrderInsert(CThostFtdcInputOrderField *pInputOrder, CThostFtdcRspInfoField *pRspInfo)
{

}

void CctpTrade::OnRspOrderAction(CThostFtdcInputOrderActionField *pInputOrderAction, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if (_OnRtnError)
	{
		char msg[512];
		sprintf_s(msg, "%s:%s", "OrderActionError:", pRspInfo->ErrorMsg);
		((DefOnRtnError)_OnRtnError)(pRspInfo == NULL ? -1 : pRspInfo->ErrorID, msg);
	}
}

void CctpTrade::OnErrRtnOrderAction(CThostFtdcOrderActionField *pOrderAction, CThostFtdcRspInfoField *pRspInfo)
{
	if (_OnRtnError)
	{
		char msg[512];
		sprintf_s(msg, "%s:%s", "OrderActionError:", pRspInfo->ErrorMsg);
		((DefOnRtnError)_OnRtnError)(pRspInfo == NULL ? -1 : pRspInfo->ErrorID, msg);
	}
}

