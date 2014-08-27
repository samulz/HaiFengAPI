using System;
using System.Collections.Concurrent;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;
using System.Collections.Concurrent;

namespace Trade2015
{

	// exchange[8], xxxtime[16], id[32], msg[128]

	#region enum
	public enum OrderType
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
		FOK,
	};

	/// <summary>
	/// 买卖方向
	/// </summary>
	public enum DirectionType
	{
		/// <summary>
		/// 买
		/// </summary>
		Buy,

		/// <summary>
		/// 卖
		/// </summary>
		Sell
	}

	/// <summary>
	/// 开平
	/// </summary>
	public enum OffsetType
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
		//Excute,
	}
	public enum OrderStatus
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
	}

	/// <summary>
	/// 交易所状态
	/// </summary>
	public enum ExchangeStatusType
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
	}

	/// <summary>
	/// 投机套保标志
	/// </summary>
	public enum HedgeType
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
	}
	#endregion enum

	#region structs
	/// <summary>
	/// 合约信息
	/// </summary>
	[StructLayout(LayoutKind.Sequential)]
	public class InstrumentField
	{
		/// <summary>
		/// 合约代码
		/// </summary>
		[MarshalAs(UnmanagedType.ByValTStr, SizeConst = 32)]
		public string InstrumentID;

		/// <summary>
		/// 产品代码
		/// </summary>
		[MarshalAs(UnmanagedType.ByValTStr, SizeConst = 32)]
		public string ProductID;

		/// <summary>
		/// 交易所代码
		/// </summary>
		[MarshalAs(UnmanagedType.ByValTStr, SizeConst = 8)]
		public string ExchangeID;

		/// <summary>
		/// 合约数量乘数
		/// </summary>
		public int VolumeMultiple;

		/// <summary>
		/// 最小变动价位
		/// </summary>
		public double PriceTick;
	}

	/// <summary>
	/// 投资者持仓明细
	/// </summary>
	[StructLayout(LayoutKind.Sequential)]
	public class PositionField
	{
		/// <summary>
		/// 合约代码
		/// </summary>
		[MarshalAs(UnmanagedType.ByValTStr, SizeConst = 32)]
		public string InstrumentID;

		/// <summary>
		/// 买卖
		/// </summary>
		public DirectionType Direction;

		/// <summary>
		/// 持仓均价
		/// </summary>
		public double Price;

		/// <summary>
		/// 持仓总量
		/// </summary>
		public int Position;

		/// <summary>
		/// 昨仓
		/// </summary>
		public int YdPosition;

		/// <summary>
		/// 持仓总量
		/// </summary>
		public int TdPosition;

		/// <summary>
		/// 占用保证金
		/// </summary>
		//public double Margin; //无此项便不再用查询

		/// <summary>
		/// 投机套保标志
		/// </summary>
		public HedgeType Hedge;
	}

	/// <summary>
	/// 帐户权益
	/// </summary>
	[StructLayout(LayoutKind.Sequential)]
	public class TradingAccount
	{
		/// <summary>
		/// 上次结算准备金
		/// </summary>
		public double PreBalance;

		/// <summary>
		/// 持仓盈亏
		/// </summary>
		public double PositionProfit;

		/// <summary>
		/// 平仓盈亏
		/// </summary>
		public double CloseProfit;

		/// <summary>
		/// 手续费
		/// </summary>
		public double Commission;

		/// <summary>
		/// 当前保证金总额
		/// </summary>
		public double CurrMargin;

		/// <summary>
		/// 冻结的资金
		/// </summary>
		public double FrozenCash;

		/// <summary>
		/// 可用资金
		/// </summary>
		public double Available;

		/// <summary>
		/// 动态权益
		/// </summary>
		public double Fund;
	}

	/// <summary>
	/// 报单
	/// </summary>
	[StructLayout(LayoutKind.Sequential)]
	public class OrderField
	{
		/// <summary>
		/// 报单标识
		/// </summary>
		public int OrderId;

		/// <summary>
		/// 合约
		/// </summary>
		[MarshalAs(UnmanagedType.ByValTStr, SizeConst = 32)]
		public string InstrumentID;

		/// <summary>
		/// 买卖
		/// </summary>
		public DirectionType Direction;

		/// <summary>
		/// 开平
		/// </summary>
		public OffsetType Offset;

		/// <summary>
		/// 报价
		/// </summary>
		public double LimitPrice;

		/// <summary>
		/// 成交均价
		/// </summary>
		public double AvgPrice;

		/// <summary>
		/// 委托时间(交易所)
		/// </summary>
		[MarshalAs(UnmanagedType.ByValTStr, SizeConst = 16)]
		public string InsertTime;

		/// <summary>
		/// 最后成交时间
		/// </summary>
		[MarshalAs(UnmanagedType.ByValTStr, SizeConst = 16)]
		public string TradeTime;

		/// <summary>
		/// 本次成交量,trade更新
		/// </summary>
		public int TradeVolume;

		/// <summary>
		/// 报单数量
		/// </summary>
		public int Volume;

		/// <summary>
		/// 未成交,trade更新
		/// </summary>
		public int VolumeLeft;

		/// <summary>
		/// 投保
		/// </summary>
		public HedgeType Hedge;

		/// <summary>
		/// 是否被撤单
		/// </summary>
		public OrderStatus Status;

		/// <summary>
		/// 是否自身委托
		/// </summary>
		public bool IsLocal;
	}

	/// <summary>
	/// 成交
	/// </summary>
	[StructLayout(LayoutKind.Sequential)]
	public class TradeField
	{
		/// <summary>
		/// 成交编号
		/// </summary>
		[MarshalAs(UnmanagedType.ByValTStr, SizeConst = 32)]
		public string TradeID;

		/// <summary>
		/// 合约代码
		/// </summary>
		[MarshalAs(UnmanagedType.ByValTStr, SizeConst = 32)]
		public string InstrumentID;

		/// <summary>
		/// 交易所代码
		/// </summary>
		[MarshalAs(UnmanagedType.ByValTStr, SizeConst = 8)]
		public string ExchangeID;

		/// <summary>
		/// 买卖方向
		/// </summary>
		public DirectionType Direction;

		/// <summary>
		/// 开平标志
		/// </summary>
		public OffsetType Offset;

		/// <summary>
		/// 投机套保标志
		/// </summary>
		public HedgeType Hedge;

		/// <summary>
		/// 价格
		/// </summary>
		public double Price;

		/// <summary>
		/// 数量
		/// </summary>
		public int Volume;

		/// <summary>
		/// 成交时间
		/// </summary>
		[MarshalAs(UnmanagedType.ByValTStr, SizeConst = 16)]
		public string TradeTime;

		/// <summary>
		/// 交易日
		/// </summary>
		[MarshalAs(UnmanagedType.ByValTStr, SizeConst = 16)]
		public string TradingDay;

		/// <summary>
		/// 对应的委托标识
		/// </summary>
		public int OrderID;
	}
	#endregion structs


	public class Proxy
	{
		/// <summary>
		/// 导入C++dll文件
		/// </summary>
		/// <param name="pFile">文件名(包含完整路径)</param>
		public Proxy(string pFile)
		{
			LoadDll(pFile);
		}

		~Proxy()
		{
			FreeLibrary(_handle);
		}

		/// <summary>
		///     原型是 :HMODULE LoadLibrary(LPCTSTR lpFileName);
		/// </summary>
		/// <param name="lpFileName"> DLL 文件名 </param>
		/// <returns> 函数库模块的句柄 </returns>
		[DllImport("kernel32.dll")]
		private static extern IntPtr LoadLibrary(string lpFileName);

		/// <summary>
		///     原型是 : FARPROC GetProcAddress(HMODULE hModule, LPCWSTR lpProcName);
		/// </summary>
		/// <param name="hModule"> 包含需调用函数的函数库模块的句柄 </param>
		/// <param name="lpProcName"> 调用函数的名称 </param>
		/// <returns> 函数指针 </returns>
		[DllImport("kernel32.dll")]
		private static extern IntPtr GetProcAddress(IntPtr hModule, string lpProcName);

		/// <summary>
		///     原型是 : BOOL FreeLibrary(HMODULE hModule);
		/// </summary>
		/// <param name="hModule"> 需释放的函数库模块的句柄 </param>
		/// <returns> 是否已释放指定的 Dll </returns>
		[DllImport("kernel32", EntryPoint = "FreeLibrary", SetLastError = true)]
		protected static extern bool FreeLibrary(IntPtr hModule);

		/// <summary>
		/// 
		/// </summary>
		/// <param name="pHModule"></param>
		/// <param name="lpProcName"></param>
		/// <param name="t"></param>
		/// <returns></returns>
		/// <exception cref="Exception"></exception>
		private static Delegate Invoke(IntPtr pHModule, string lpProcName, Type t)
		{
			// 若函数库模块的句柄为空，则抛出异常 
			if (pHModule == IntPtr.Zero)
			{
				throw (new Exception(" 函数库模块的句柄为空 , 请确保已进行 LoadDll 操作 !"));
			}
			// 取得函数指针 
			IntPtr farProc = GetProcAddress(pHModule, lpProcName);
			// 若函数指针，则抛出异常 
			if (farProc == IntPtr.Zero)
			{
				throw (new Exception(" 没有找到 :" + lpProcName + " 这个函数的入口点 "));
			}
			return Marshal.GetDelegateForFunctionPointer(farProc, t);
		}


		public IntPtr _handle;

		private delegate int DefCreateApi();

		private delegate int DefReqConnect(string pFront);

		private delegate int DefReqUserLogin(string pInvestor, string pPwd, string pBroker);

		private delegate void DefReqUserLogout();

		private delegate string DefGetTradingDay();

		private delegate int DefReqOrderInsert(string pInstrument, DirectionType pDirection, OffsetType pOffset, double pPrice, int pVolume, HedgeType pHedge, OrderType pType);

		private delegate int DefReqOrderAction(int pOrderId);

		#region 注册响应

		private delegate void Reg(IntPtr pPtr);

		public delegate void FrontConnected();

		private FrontConnected _OnFrontConnected;

		public event FrontConnected OnFrontConnected
		{
			add
			{
				_OnFrontConnected += value;
				var reg = (Invoke(this._handle, "RegOnFrontConnected", typeof(Reg)) as Reg);
				reg((Marshal.GetFunctionPointerForDelegate(_OnFrontConnected)));
			}
			remove
			{
				_OnFrontConnected -= value;
				(Invoke(this._handle, "RegOnFrontConnected", typeof(Reg)) as Reg)(Marshal.GetFunctionPointerForDelegate(_OnFrontConnected));
			}
		}

		public delegate void RspUserLogin(int pErrId);

		private RspUserLogin _OnRspUserLogin;

		public event RspUserLogin OnRspUserLogin
		{
			add
			{
				_OnRspUserLogin += value;
				(Invoke(this._handle, "RegOnRspUserLogin", typeof(Reg)) as Reg)(Marshal.GetFunctionPointerForDelegate(_OnRspUserLogin));
			}
			remove
			{
				_OnRspUserLogin -= value;
				(Invoke(this._handle, "RegOnRspUserLogin", typeof(Reg)) as Reg)(Marshal.GetFunctionPointerForDelegate(_OnRspUserLogin));
			}
		}

		public delegate void RspUserLogout(int pReason);

		private RspUserLogout _OnRspUserLogout;

		public event RspUserLogout OnRspUserLogout
		{
			add
			{
				_OnRspUserLogout += value;
				(Invoke(this._handle, "RegOnRspUserLogout", typeof(Reg)) as Reg)(Marshal.GetFunctionPointerForDelegate(_OnRspUserLogout));
			}
			remove
			{
				_OnRspUserLogout -= value;
				(Invoke(this._handle, "RegOnRspUserLogout", typeof(Reg)) as Reg)(Marshal.GetFunctionPointerForDelegate(_OnRspUserLogout));
			}
		}

		public delegate void RtnError(int pErrId, string pMsg);

		private RtnError _OnRtnError;

		public event RtnError OnRtnError
		{
			add
			{
				_OnRtnError += value;
				(Invoke(this._handle, "RegOnRtnError", typeof(Reg)) as Reg)(Marshal.GetFunctionPointerForDelegate(_OnRtnError));
			}
			remove
			{
				_OnRtnError -= value;
				(Invoke(this._handle, "RegOnRtnError", typeof(Reg)) as Reg)(Marshal.GetFunctionPointerForDelegate(_OnRtnError));
			}
		}

		public delegate void RtnNotice(string pMsg);

		private RtnNotice _OnRtnNotice;

		public event RtnNotice OnRtnNotice
		{
			add
			{
				_OnRtnNotice += value;
				(Invoke(this._handle, "RegOnRtnNotice", typeof(Reg)) as Reg)(Marshal.GetFunctionPointerForDelegate(_OnRtnNotice));
			}
			remove
			{
				_OnRtnNotice -= value;
				(Invoke(this._handle, "RegOnRtnNotice", typeof(Reg)) as Reg)(Marshal.GetFunctionPointerForDelegate(_OnRtnNotice));
			}
		}

		public delegate void RtnExchangeStatus(string pExchange, ExchangeStatusType pStatus);

		private RtnExchangeStatus _OnRtnExchangeStatus;

		public event RtnExchangeStatus OnRtnExchangeStatus
		{
			add
			{
				_OnRtnExchangeStatus += value;
				(Invoke(this._handle, "RegOnRtnExchangeStatus", typeof(Reg)) as Reg)(Marshal.GetFunctionPointerForDelegate(_OnRtnExchangeStatus));
			}
			remove
			{
				_OnRtnExchangeStatus -= value;
				(Invoke(this._handle, "RegOnRtnExchangeStatus", typeof(Reg)) as Reg)(Marshal.GetFunctionPointerForDelegate(_OnRtnExchangeStatus));
			}
		}

		public delegate void RspQryInstrument(InstrumentField pInstrument, bool pLast);

		private RspQryInstrument _OnRspQryInstrument;

		public event RspQryInstrument OnRspQryInstrument
		{
			add
			{
				_OnRspQryInstrument += value;
				(Invoke(this._handle, "RegOnRspQryInstrument", typeof(Reg)) as Reg)(Marshal.GetFunctionPointerForDelegate(_OnRspQryInstrument));
			}
			remove
			{
				_OnRspQryInstrument -= value;
				(Invoke(this._handle, "RegOnRspQryInstrument", typeof(Reg)) as Reg)(Marshal.GetFunctionPointerForDelegate(_OnRspQryInstrument));
			}
		}

		public delegate void RspQryOrder(OrderField pField, bool pLast);

		private RspQryOrder _OnRspQryOrder;

		public event RspQryOrder OnRspQryOrder
		{
			add
			{
				_OnRspQryOrder += value;
				(Invoke(this._handle, "RegOnRspQryOrder", typeof(Reg)) as Reg)(Marshal.GetFunctionPointerForDelegate(_OnRspQryOrder));
			}
			remove
			{
				_OnRspQryOrder -= value;
				(Invoke(this._handle, "RegOnRspQryOrder", typeof(Reg)) as Reg)(Marshal.GetFunctionPointerForDelegate(_OnRspQryOrder));
			}
		}

		public delegate void RspQryTrade(TradeField pField, bool pLast);

		private RspQryTrade _OnRspQryTrade;

		public event RspQryTrade OnRspQryTrade
		{
			add
			{
				_OnRspQryTrade += value;
				(Invoke(this._handle, "RegOnRspQryTrade", typeof(Reg)) as Reg)(Marshal.GetFunctionPointerForDelegate(_OnRspQryTrade));
			}
			remove
			{
				_OnRspQryTrade -= value;
				(Invoke(this._handle, "RegOnRspQryTrade", typeof(Reg)) as Reg)(Marshal.GetFunctionPointerForDelegate(_OnRspQryTrade));
			}
		}

		public delegate void RspQryPosition(PositionField pPosition, bool pLast);

		private RspQryPosition _OnRspQryPositiont;

		public event RspQryPosition OnRspQryPositiont
		{
			add
			{
				_OnRspQryPositiont += value;
				(Invoke(this._handle, "RegOnRspQryPosition", typeof(Reg)) as Reg)(Marshal.GetFunctionPointerForDelegate(_OnRspQryPositiont));
			}
			remove
			{
				_OnRspQryPositiont -= value;
				(Invoke(this._handle, "RegOnRspQryPosition", typeof(Reg)) as Reg)(Marshal.GetFunctionPointerForDelegate(_OnRspQryPositiont));
			}
		}

		public delegate void RspQryTradingAccount(TradingAccount pAccount);

		private RspQryTradingAccount _OnRspQryTradingAccount;

		public event RspQryTradingAccount OnRspQryTradingAccount
		{
			add
			{
				_OnRspQryTradingAccount += value;
				(Invoke(this._handle, "RegOnRspQryTradingAccount", typeof(Reg)) as Reg)(Marshal.GetFunctionPointerForDelegate(_OnRspQryTradingAccount));
			}
			remove
			{
				_OnRspQryTradingAccount -= value;
				(Invoke(this._handle, "RegOnRspQryTradingAccount", typeof(Reg)) as Reg)(Marshal.GetFunctionPointerForDelegate(_OnRspQryTradingAccount));
			}
		}

		public delegate void RtnOrder(OrderField pOrder);

		private RtnOrder _OnRtnOrder;

		public event RtnOrder OnRtnOrder
		{
			add
			{
				_OnRtnOrder += value;
				(Invoke(this._handle, "RegOnRtnOrder", typeof(Reg)) as Reg)(Marshal.GetFunctionPointerForDelegate(_OnRtnOrder));
			}
			remove
			{
				_OnRtnOrder -= value;
				(Invoke(this._handle, "RegOnRtnOrder", typeof(Reg)) as Reg)(Marshal.GetFunctionPointerForDelegate(_OnRtnOrder));
			}
		}

		private RtnOrder _OnRtnCancel;

		public event RtnOrder OnRtnCancel
		{
			add
			{
				_OnRtnCancel += value;
				(Invoke(this._handle, "RegOnRtnCancel", typeof(Reg)) as Reg)(Marshal.GetFunctionPointerForDelegate(_OnRtnCancel));
			}
			remove
			{
				_OnRtnCancel -= value;
				(Invoke(this._handle, "RegOnRtnCancel", typeof(Reg)) as Reg)(Marshal.GetFunctionPointerForDelegate(_OnRtnCancel));
			}
		}

		public delegate void RtnTrade(TradeField pTrade);

		private RtnTrade _OnRtnTrade;

		public event RtnTrade OnRtnTrade
		{
			add
			{
				_OnRtnTrade += value;
				(Invoke(this._handle, "RegOnRtnTrade", typeof(Reg)) as Reg)(Marshal.GetFunctionPointerForDelegate(_OnRtnTrade));
			}
			remove
			{
				_OnRtnTrade -= value;
				(Invoke(this._handle, "RegOnRtnTrade", typeof(Reg)) as Reg)(Marshal.GetFunctionPointerForDelegate(_OnRtnTrade));
			}
		}
		#endregion


		/// <summary>
		/// 加载C++文件,取得相应函数
		/// </summary>
		/// <param name="pFile">格式:proxyXXXQuote.dll或proxyXXXTrade.dll;XXX-平台名</param>
		/// <exception cref="Exception"></exception>
		protected void LoadDll(string pFile)
		{
			this._handle = LoadLibrary(pFile);// Environment.CurrentDirectory + "\\" + pFile);
			if (this._handle == IntPtr.Zero)
			{
				throw (new Exception(String.Format(" 没有找到 :{0}.", Environment.CurrentDirectory + "\\" + pFile)));
			}
			Directory.CreateDirectory("log");
		}
		

		public int ReqConnect(string pFront)
		{
			((DefCreateApi)Invoke(this._handle, "CreateApi", typeof(DefCreateApi)))();
			return ((DefReqConnect)Invoke(this._handle, "ReqConnect", typeof(DefReqConnect)))(pFront);
		}

		public int ReqUserLogin(string pInvestor, string pPwd, string pBroker)
		{
			return ((DefReqUserLogin)Invoke(this._handle, "ReqUserLogin", typeof(DefReqUserLogin)))(pInvestor, pPwd, pBroker);
		}

		public void ReqUserLogout()
		{
			((DefReqUserLogout)Invoke(this._handle, "ReqUserLogout", typeof(DefReqUserLogout)))();
		}

		public string GetTradingDay()
		{
			return ((DefGetTradingDay)Invoke(this._handle, "GetTradingDay", typeof(DefGetTradingDay)))();
		}

		public int ReqQryOrder()
		{
			return ((DefCreateApi)Invoke(this._handle, "ReqQryOrder", typeof(DefCreateApi)))();
		}

		public int ReqQryTrade()
		{
			return ((DefCreateApi)Invoke(this._handle, "ReqQryTrade", typeof(DefCreateApi)))();
		}

		public int ReqQryPosition()
		{
			return ((DefCreateApi)Invoke(this._handle, "ReqQryPosition", typeof(DefCreateApi)))();
		}

		public int ReqQryAccount()
		{
			return ((DefCreateApi)Invoke(this._handle, "ReqQryAccount", typeof(DefCreateApi)))();
		}

		/// <summary>
		/// 
		/// </summary>
		/// <param name="pInstrument"></param>
		/// <param name="pDirection"></param>
		/// <param name="pOffset"></param>
		/// <param name="pPrice">价格</param>
		/// <param name="pVolume"></param>
		/// <param name="pHedge"></param>
		/// <param name="pType">报单类型</param>
		/// <returns></returns>
		public int ReqOrderInsert(string pInstrument, DirectionType pDirection, OffsetType pOffset, double pPrice, int pVolume, HedgeType pHedge, OrderType pType)
		{
			return ((DefReqOrderInsert)Invoke(this._handle, "ReqOrderInsert", typeof(DefReqOrderInsert)))(pInstrument, pDirection, pOffset, pPrice, pVolume, pHedge, pType);
		}

		public int ReqOrderAction(int pOrderId)
		{
			return ((DefReqOrderAction)Invoke(this._handle, "ReqOrderAction", typeof(DefReqOrderAction)))(pOrderId);
		}
	}

}
