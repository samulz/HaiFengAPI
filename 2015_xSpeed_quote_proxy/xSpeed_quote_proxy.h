

#include "../2015_include_lib/DFITCMdApi.h"

using namespace DFITCXSPEEDMDAPI;

class CxSpeed_quote_proxy : DFITCMdSpi
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
	CxSpeed_quote_proxy(void);

	// TODO:  在此添加您的方法。
	/**
	* 网络连接正常响应
	*/
	virtual void OnFrontConnected();

	/**
	* 网络连接不正常响应
	*/
	virtual void OnFrontDisconnected(int nReason);

	/**
	* 登陆请求响应:当用户发出登录请求后，前置机返回响应时此方法会被调用，通知用户登录是否成功。
	* @param pRspUserLogin:用户登录信息结构地址。
	* @param pRspInfo:若请求失败，返回错误信息地址，该结构含有错误信息。
	*/
	virtual void OnRspUserLogin(struct DFITCUserLoginInfoRtnField * pRspUserLogin, struct DFITCErrorRtnField * pRspInfo);

	/**
	* 登出请求响应:当用户发出退出请求后，前置机返回响应此方法会被调用，通知用户退出状态。
	* @param pRspUsrLogout:返回用户退出信息结构地址。
	* @param pRspInfo:若请求失败，返回错误信息地址。
	*/
	virtual void OnRspUserLogout(struct DFITCUserLogoutInfoRtnField * pRspUsrLogout, struct DFITCErrorRtnField * pRspInfo) ;

	/*错误应答*/
	virtual void OnRspError(struct DFITCErrorRtnField *pRspInfo) ;

	/**
	* 行情订阅应答:当用户发出行情订阅该方法会被调用。
	* @param pSpecificInstrument:指向合约响应结构，该结构包含合约的相关信息。
	* @param pRspInfo:错误信息，如果发生错误，该结构含有错误信息。
	*/
	virtual void OnRspSubMarketData(struct DFITCSpecificInstrumentField * pSpecificInstrument, struct DFITCErrorRtnField * pRspInfo) {};

	/**
	* 取消订阅行情应答:当用户发出退订请求后该方法会被调用。
	* @param pSpecificInstrument:指向合约响应结构，该结构包含合约的相关信息。
	* @param pRspInfo:错误信息，如果发生错误，该结构含有错误信息。
	*/
	virtual void OnRspUnSubMarketData(struct DFITCSpecificInstrumentField * pSpecificInstrument, struct DFITCErrorRtnField * pRspInfo) {};

	/**
	* 行情消息应答:如果订阅行情成功且有行情返回时，该方法会被调用。
	* @param pMarketDataField:指向行情信息结构的指针，结构体中包含具体的行情信息。
	*/
	virtual void OnMarketData(struct DFITCDepthMarketDataField * pMarketDataField);

	/**
	* 交易日确认响应:用于接收交易日信息。
	* @param DFITCTradingDayRtnField: 返回交易日请求确认响应结构的地址。
	*/
	virtual void OnRspTradingDay(struct DFITCTradingDayRtnField * pTradingDayRtnData);
};

