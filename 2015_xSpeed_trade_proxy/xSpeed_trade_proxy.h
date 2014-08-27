

#include "../2015_include_lib/DFITCTraderApi.h"

using namespace DFITCXSPEEDAPI;

class CxSpeed_trade_proxy : DFITCTraderSpi
{
private:
	DFITCErrorRtnField rif;
	DFITCErrorRtnField* repareInfo(DFITCErrorRtnField *pRspInfo)
	{
		if (pRspInfo == NULL)
		{
			memset(&rif, 0, sizeof(DFITCErrorRtnField));
			rif.nErrorID = 0;
			strcpy_s(rif.errorMsg, "no error");
			return &rif;
		}
		else
			return pRspInfo;
	}

public:
	CxSpeed_trade_proxy(void);

	/* 网络连接正常响应:当客户端与交易后台需建立起通信连接时（还未登录前），客户端API会自动检测与前置机之间的连接，
	* 当网络可用，将自动建立连接，并调用该方法通知客户端， 客户端可以在实现该方法时，重新使用资金账号进行登录。
	*（该方法是在Api和前置机建立连接后被调用，该调用仅仅是说明tcp连接已经建立成功。用户需要自行登录才能进行后续的业务操作。
	*  登录失败则此方法不会被调用。）
	*/
	virtual void OnFrontConnected();

	/**
	* 网络连接不正常响应：当客户端与交易后台通信连接断开时，该方法被调用。当发生这个情况后，API会自动重新连接，客户端可不做处理。
	* @param  nReason:错误原因。
	*        0x1001 网络读失败
	*        0x1002 网络写失败
	*        0x2001 接收心跳超时
	*        0x2002 发送心跳失败
	*        0x2003 收到错误报文
	*/
	virtual void OnFrontDisconnected(int nReason);

	/**
	* 登陆请求响应:当用户发出登录请求后，前置机返回响应时此方法会被调用，通知用户登录是否成功。
	* @param pUserLoginInfoRtn:用户登录信息结构地址。
	* @param pErrorInfo:若请求失败，返回错误信息地址，该结构含有错误信息。
	*/
	virtual void OnRspUserLogin(struct DFITCUserLoginInfoRtnField * pUserLoginInfoRtn, struct DFITCErrorRtnField * pErrorInfo);

	/**
	* 登出请求响应:当用户发出退出请求后，前置机返回响应此方法会被调用，通知用户退出状态。
	* @param pUserLogoutInfoRtn:返回用户退出信息结构地址。
	* @param pErrorInfo:若请求失败，返回错误信息地址。
	*/
	virtual void OnRspUserLogout(struct DFITCUserLogoutInfoRtnField * pUserLogoutInfoRtn, struct DFITCErrorRtnField * pErrorInfo);

	/**
	* 期货委托报单响应:当用户录入报单后，前置返回响应时该方法会被调用。
	* @param pOrderRtn:返回用户下单信息结构地址。
	* @param pErrorInfo:若请求失败，返回错误信息地址。
	*/
	virtual void OnRspInsertOrder(struct DFITCOrderRspDataRtnField * pOrderRtn, struct DFITCErrorRtnField * pErrorInfo);

	/**
	* 期货委托撤单响应:当用户撤单后，前置返回响应是该方法会被调用。
	* @param pOrderCanceledRtn:返回撤单响应信息结构地址。
	* @param pErrorInfo:若请求失败，返回错误信息地址。
	*/
	virtual void OnRspCancelOrder(struct DFITCOrderRspDataRtnField * pOrderCanceledRtn, struct DFITCErrorRtnField * pErrorInfo){};

	/**
	* 错误回报
	* @param pErrorInfo:错误信息的结构地址。
	*/
	virtual void OnRtnErrorMsg(struct DFITCErrorRtnField * pErrorInfo);

	/**
	* 成交回报:当委托成功交易后次方法会被调用。
	* @param pRtnMatchData:指向成交回报的结构的指针。
	*/
	virtual void OnRtnMatchedInfo(struct DFITCMatchRtnField * pRtnMatchData);

	/**
	* 委托回报:下单委托成功后，此方法会被调用。
	* @param pRtnOrderData:指向委托回报地址的指针。
	*/
	virtual void OnRtnOrder(struct DFITCOrderRtnField * pRtnOrderData);

	/**
	* 撤单回报:当撤单成功后该方法会被调用。
	* @param pCancelOrderData:指向撤单回报结构的地址，该结构体包含被撤单合约的相关信息。
	*/
	virtual void OnRtnCancelOrder(struct DFITCOrderCanceledRtnField * pCancelOrderData);

	/**
	* 查询当日委托响应:当用户发出委托查询后，该方法会被调用。
	* @param pRtnOrderData:指向委托回报结构的地址。
	* @param bIsLast:表明是否是最后一条响应信息（0 -否   1 -是）。
	*/
	virtual void OnRspQryOrderInfo(struct DFITCOrderCommRtnField * pRtnOrderData, struct DFITCErrorRtnField * pErrorInfo, bool bIsLast);

	/**
	* 查询当日成交响应:当用户发出成交查询后该方法会被调用。
	* @param pRtnMatchData:指向成交回报结构的地址。
	* @param bIsLast:表明是否是最后一条响应信息（0 -否   1 -是）。
	*/
	virtual void OnRspQryMatchInfo(struct DFITCMatchedRtnField * pRtnMatchData, struct DFITCErrorRtnField * pErrorInfo, bool bIsLast);

	/**
	* 持仓查询响应:当用户发出持仓查询指令后，前置返回响应时该方法会被调用。
	* @param pPositionInfoRtn:返回持仓信息结构的地址。
	* @param pErrorInfo:错误信息结构，如果持仓查询发生错误，则返回错误信息。
	* @param bIsLast:表明是否是最后一条响应信息（0 -否   1 -是）。
	*/
	virtual void OnRspQryPosition(struct DFITCPositionInfoRtnField * pPositionInfoRtn, struct DFITCErrorRtnField * pErrorInfo, bool bIsLast);

	/**
	* 客户资金查询响应:当用户发出资金查询指令后，前置返回响应时该方法会被调用。
	* @param pCapitalInfoRtn:返回资金信息结构的地址。
	* @param pErrorInfo:错误信息结构，如果客户资金查询发生错误，则返回错误信息。
	*/
	virtual void OnRspCustomerCapital(struct DFITCCapitalInfoRtnField * pCapitalInfoRtn, struct DFITCErrorRtnField * pErrorInfo, bool bIsLast);

	/**
	* 交易所合约查询响应:当用户发出合约查询指令后，前置返回响应时该方法会被调用。
	* @param pInstrumentData:返回合约信息结构的地址。
	* @param pErrorInfo:错误信息结构，如果持仓查询发生错误，则返回错误信息。
	* @param bIsLast:表明是否是最后一条响应信息（0 -否   1 -是）。
	*/
	virtual void OnRspQryExchangeInstrument(struct DFITCExchangeInstrumentRtnField * pInstrumentData, struct DFITCErrorRtnField * pErrorInfo, bool bIsLast);

	/**
	* 套利合约查询响应:当用户发出套利合约查询指令后，前置返回响应时该方法会被调用。
	* @param pAbiInstrumentData:返回套利合约信息结构的地址。
	* @param pErrorInfo:错误信息结构，如果持仓查询发生错误，则返回错误信息。
	* @param bIsLast:表明是否是最后一条响应信息（0 -否   1 -是）。
	*/
	virtual void OnRspArbitrageInstrument(struct DFITCAbiInstrumentRtnField * pAbiInstrumentData, struct DFITCErrorRtnField * pErrorInfo, bool bIsLast);

	/**
	* 查询指定合约响应:当用户发出指定合约查询指令后，前置返回响应时该方法会被调用。
	* @param pInstrument:返回指定合约信息结构的地址。
	*/
	virtual void OnRspQrySpecifyInstrument(struct DFITCInstrumentRtnField * pInstrument, struct DFITCErrorRtnField * pErrorInfo, bool bIsLast){};

	/**
	* 查询持仓明细响应:当用户发出查询持仓明细后，前置返回响应时该方法会被调用。
	* @param pInstrument:返回持仓明细结构的地址。
	*/
	virtual void OnRspQryPositionDetail(struct DFITCPositionDetailRtnField * pPositionDetailRtn, struct DFITCErrorRtnField * pErrorInfo, bool bIsLast){};

	/**
	* 交易通知响应:用于接收XSPEED柜台手动发送通知，即支持指定客户，也支持系统广播。
	* @param pTradingNoticeInfo: 返回用户事件通知结构的地址。
	*/
	virtual void OnRtnTradingNotice(struct DFITCTradingNoticeInfoField * pTradingNoticeInfo);

	/**
	* 合约交易状态通知响应:用于接收合约在开市情况下的状态。
	* @param pInstrumentStatus: 返回交易合约状态通知结构的地址。
	*/
	virtual void OnRtnInstrumentStatus(struct DFITCInstrumentStatusField * pInstrumentStatus);

	/**
	* 密码修改响应:用于修改资金账户登录密码。
	* @param pResetPassword: 返回密码修改结构的地址。
	*/
	virtual void OnRspResetPassword(struct DFITCResetPwdRspField * pResetPassword, struct DFITCErrorRtnField * pErrorInfo){};

	/**
	* 交易编码查询响应:返回交易编码信息
	* @param pTradeCode: 返回交易编码查询结构的地址。
	*/
	virtual void OnRspQryTradeCode(struct DFITCQryTradeCodeRtnField * pTradeCode, struct DFITCErrorRtnField * pErrorInfo, bool bIsLast){};

	/**
	* 账单确认响应:用于接收客户账单确认状态。
	* @param pBillConfirm: 返回账单确认结构的地址。
	*/
	virtual void OnRspBillConfirm(struct DFITCBillConfirmRspField * pBillConfirm, struct DFITCErrorRtnField * pErrorInfo){};

	/**
	* 查询客户权益计算方式响应:返回客户权益计算的方式
	* @param pEquityComputMode: 返回客户权益计算方式结构的地址。
	*/
	virtual void OnRspEquityComputMode(struct DFITCEquityComputModeRtnField * pEquityComputMode){};

	/**
	* 客户结算账单查询响应:返回账单信息
	* @param pQryBill: 返回客户结算账单查询结构的地址。
	*/
	virtual void OnRspQryBill(struct DFITCQryBillRtnField *pQryBill, struct DFITCErrorRtnField * pErrorInfo, bool bIsLast){};

	/**
	* 厂商ID确认响应:用于接收厂商信息。
	* @param pProductRtnData: 返回厂商ID确认响应结构的地址。
	*/
	virtual void OnRspConfirmProductInfo(struct DFITCProductRtnField * pProductRtnData){};

	/**
	* 交易日确认响应:用于接收交易日信息。
	* @param DFITCTradingDayRtnField: 返回交易日请求确认响应结构的地址。
	*/
	virtual void OnRspTradingDay(struct DFITCTradingDayRtnField * pTradingDayRtnData);
};

