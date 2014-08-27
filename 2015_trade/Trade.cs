using System;
using System.Collections.Concurrent;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading;

namespace Trade2015
{
	/// <summary>
	/// 
	/// </summary>
	public class IntEventArgs : EventArgs
	{
		/// <summary>
		/// 错误代码
		/// </summary>
		public int Value = 0;
	}

	/// <summary>
	/// 
	/// </summary>
	public class StringEventArgs : EventArgs
	{
		/// <summary>
		/// 错误代码
		/// </summary>
		public string Value = string.Empty;
	}

	/// <summary>
	/// 
	/// </summary>
	public class ErrorEventArgs : EventArgs
	{
		/// <summary>
		/// 错误代码
		/// </summary>
		public int ErrorID = 0;
		/// <summary>
		/// 错误说明
		/// </summary>
		public string ErrorMsg = string.Empty;
	}

	/// <summary>
	/// 
	/// </summary>
	public class StatusEventArgs : EventArgs
	{
		/// <summary>
		/// 交易所/品种/合约
		/// </summary>
		public string Exchange = string.Empty;
		/// <summary>
		/// 交易状态
		/// </summary>
		public ExchangeStatusType Status = ExchangeStatusType.Trading;
	}

	/// <summary>
	/// 报单状态改变响应
	/// </summary>
	public class OrderArgs : EventArgs
	{
		/// <summary>
		/// 报单
		/// </summary>
		public OrderField Value;
	}
	/// <summary>
	/// 报单成交响应
	/// </summary>
	public class TradeArgs : EventArgs
	{
		/// <summary>
		/// 报单
		/// </summary>
		public TradeField Value;
	}

	public class Trade
	{
		private Proxy _proxy;
		public Trade(string pProxyFile)
		{
			_proxy = new Proxy(pProxyFile);
			_proxy.OnFrontConnected += _import_OnFrontConnected;
			_proxy.OnRspUserLogin += _import_OnRspUserLogin;
			_proxy.OnRspUserLogout += _import_OnRspUserLogout;
			_proxy.OnRspQryInstrument += _import_OnRspQryInstrument;
			_proxy.OnRspQryOrder += _import_OnRspQryOrder;
			_proxy.OnRspQryPositiont += _import_OnRspQryPositiont;
			_proxy.OnRspQryTrade += _import_OnRspQryTrade;
			_proxy.OnRspQryTradingAccount += _import_OnRspQryTradingAccount;
			_proxy.OnRtnCancel += _import_OnRtnCancel;
			_proxy.OnRtnError += _import_OnRtnError;
			_proxy.OnRtnExchangeStatus += _import_OnRtnExchangeStatus;
			_proxy.OnRtnNotice += _import_OnRtnNotice;
			_proxy.OnRtnOrder += _import_OnRtnOrder;
			_proxy.OnRtnTrade += _import_OnRtnTrade;
		}

		/// <summary>
		/// 交易所状态
		/// </summary>
		public ConcurrentDictionary<string, ExchangeStatusType> DicExcStatus = new ConcurrentDictionary<string, ExchangeStatusType>();

		/// <summary>
		/// 交易所时间
		/// </summary>
		protected ConcurrentDictionary<string, TimeSpan> DicExcLoginTime = new ConcurrentDictionary<string, TimeSpan>();


		/// <summary>
		/// 合约信息
		/// </summary>
		public ConcurrentDictionary<string, InstrumentField> DicInstrumentField = new ConcurrentDictionary<string, InstrumentField>();

		/// <summary>
		/// 报单. CTP:session|front|orderef
		/// </summary>
		public ConcurrentDictionary<int, OrderField> DicOrderField = new ConcurrentDictionary<int, OrderField>();

		/// <summary>
		/// 成交. CTP:TradeID+Direct(区分自成交)
		/// </summary>
		public ConcurrentDictionary<string, TradeField> DicTradeField = new ConcurrentDictionary<string, TradeField>();

		/// <summary>
		/// 持仓
		/// </summary>
		public ConcurrentDictionary<string, PositionField> DicPositionField = new ConcurrentDictionary<string, PositionField>();

		/// <summary>
		/// 资金权益
		/// </summary>
		public TradingAccount TradingAccount = new TradingAccount();

		#region 注册响应
		public delegate void FrontConnected(object sender, EventArgs args);

		private FrontConnected _OnFrontConnected;

		public event FrontConnected OnFrontConnected
		{
			add
			{
				_OnFrontConnected += value;
			}
			remove
			{
				_OnFrontConnected -= value;
			}
		}

		public delegate void RspUserLogin(object sender, IntEventArgs args);

		private RspUserLogin _OnRspUserLogin;

		public event RspUserLogin OnRspUserLogin
		{
			add
			{
				_OnRspUserLogin += value;
			}
			remove
			{
				_OnRspUserLogin -= value;
			}
		}

		public delegate void RspUserLogout(object sender, IntEventArgs args);

		private RspUserLogout _OnRspUserLogout;

		public event RspUserLogout OnRspUserLogout
		{
			add
			{
				_OnRspUserLogout += value;
			}
			remove
			{
				_OnRspUserLogout -= value;
			}
		}

		public delegate void RtnError(object sender, ErrorEventArgs args);

		private RtnError _OnRtnError;

		public event RtnError OnRtnError
		{
			add
			{
				_OnRtnError += value;
			}
			remove
			{
				_OnRtnError -= value;
			}
		}

		public delegate void RtnNotice(object sender, StringEventArgs args);

		private RtnNotice _OnRtnNotice;

		public event RtnNotice OnRtnNotice
		{
			add
			{
				_OnRtnNotice += value;
			}
			remove
			{
				_OnRtnNotice -= value;
			}
		}

		public delegate void RtnExchangeStatus(object sender, StatusEventArgs args);

		private RtnExchangeStatus _OnRtnExchangeStatus;

		public event RtnExchangeStatus OnRtnExchangeStatus
		{
			add
			{
				_OnRtnExchangeStatus += value;
			}
			remove
			{
				_OnRtnExchangeStatus -= value;
			}
		}

		public delegate void RtnOrder(object sender, OrderArgs args);

		private RtnOrder _OnRtnOrder;

		public event RtnOrder OnRtnOrder
		{
			add
			{
				_OnRtnOrder += value;
			}
			remove
			{
				_OnRtnOrder -= value;
			}
		}

		private RtnOrder _OnRtnCancel;

		public event RtnOrder OnRtnCancel
		{
			add
			{
				_OnRtnCancel += value;
			}
			remove
			{
				_OnRtnCancel -= value;
			}
		}

		public delegate void RtnTrade(object sender, TradeArgs args);

		private RtnTrade _OnRtnTrade;

		public event RtnTrade OnRtnTrade
		{
			add
			{
				_OnRtnTrade += value;
			}
			remove
			{
				_OnRtnTrade -= value;
			}
		}
		#endregion

		#region 属性

		/// <summary>
		/// 服务器名称
		/// </summary>
		public string Server { get; set; }

		/// <summary>
		/// 经纪公司代码
		/// </summary>
		public string Broker { get; set; }

		/// <summary>
		/// 帐号
		/// </summary>
		public string Investor { get; set; }

		/// <summary>
		/// 密码
		/// </summary>
		public string Password { get; set; }

		/// <summary>
		/// 交易日
		/// </summary>
		public string TradingDay { get; protected set; }

		/// <summary>
		/// 登录成功
		/// </summary>
		public bool IsLogin { get; protected set; }
		#endregion

		void _import_OnRtnNotice(string pMsg)
		{
			if (_OnRtnNotice != null)
			{
				_OnRtnNotice(this, new StringEventArgs
				{
					Value = pMsg,
				});
			}
		}

		void _import_OnRtnError(int pErrId, string pMsg)
		{
			if (_OnRtnError != null)
			{
				_OnRtnError(this, new ErrorEventArgs
				{
					ErrorID = pErrId,
					ErrorMsg = pMsg,
				});
			}
		}

		void _import_OnRtnExchangeStatus(string pExchange, ExchangeStatusType pStatus)
		{
			DicExcStatus.AddOrUpdate(pExchange, pStatus, (k, v) => pStatus);
			if (_OnRtnExchangeStatus != null)
			{
				_OnRtnExchangeStatus(this, new StatusEventArgs
				{
					Exchange = pExchange,
					Status = pStatus,
				});
			}
		}


		void _import_OnRtnOrder(OrderField pOrder)
		{
			OrderField f = DicOrderField.GetOrAdd(pOrder.OrderId, new OrderField());
			foreach (var info in pOrder.GetType().GetFields())
			{
				f.GetType().GetField(info.Name).SetValue(f, Convert.ChangeType(info.GetValue(pOrder), f.GetType().GetField(info.Name).FieldType));
			}
			if (_OnRtnOrder != null)
			{
				_OnRtnOrder(this, new OrderArgs
				{
					Value = f,
				});
			}
		}

		private void _import_OnRtnTrade(TradeField pTrade)
		{
			//tradeid增加方向标识(自成交冲突)
			TradeField f = DicTradeField.GetOrAdd(pTrade.TradeID + (int)pTrade.Direction, new TradeField
			{
				TradeID = pTrade.TradeID + (int)pTrade.Direction,
			});
			foreach (var info in pTrade.GetType().GetFields())
			{
				if (info.Name == "TradeID")
				{
					continue;
				}
				f.GetType().GetField(info.Name).SetValue(f, Convert.ChangeType(info.GetValue(pTrade), f.GetType().GetField(info.Name).FieldType));
			}

			PositionField pf;
			//处理持仓
			if (pTrade.Offset == OffsetType.Open)
			{
				pf = this.DicPositionField.GetOrAdd(pTrade.InstrumentID + "_" + pTrade.Direction, new PositionField());
				pf.InstrumentID = pTrade.InstrumentID;
				pf.Direction = pTrade.Direction;
				pf.Hedge = pTrade.Hedge;
				pf.Price = (pf.Price * pf.Position + pTrade.Price * pTrade.Volume) / (pf.Position + pTrade.Volume);
				pf.TdPosition += pTrade.Volume;
				pf.Position += pTrade.Volume;
			}
			else
			{
				pf = this.DicPositionField.GetOrAdd(pTrade.InstrumentID + "_" + (pTrade.Direction == DirectionType.Buy ? "Sell" : "Buy"), new PositionField());
				if (pf.TdPosition > 0)// pTrade.ExchangeID == "SHFE" && pTrade.Offset == OffsetType.CloseToday)
				{
					pf.TdPosition -= Math.Min(pf.TdPosition, pTrade.Volume);
				}
				pf.Position -= pTrade.Volume;
			}
			//有关orderfield中的avgprice和tradetime字段,已在C++层进行处理
			//OrderField of;
			//if (DicOrderField.TryGetValue(f.OrderID, out of))
			//{
			//	int preTrade = of.Volume - of.VolumeLeft;
			//	of.AvgPrice = (of.AvgPrice * preTrade + pTrade.Price * pTrade.Volume) / (preTrade + pTrade.Volume);
			//	of.TradeTime = pTrade.TradeTime;
			//	of.VolumeLeft -= pTrade.Volume;
			//	of.Status = of.VolumeLeft == 0 ? OrderStatus.Filled : OrderStatus.Partial;
			//	if (_OnRtnTrade != null)
			//	{
			//		_OnRtnTrade(this, new TradeArgs
			//		{
			//			Value = f,
			//		});
			//	}
			//}
			if (_OnRtnTrade != null)
			{
				_OnRtnTrade(this, new TradeArgs
				{
					Value = f,
				});
			}
		}

		void _import_OnRtnCancel(OrderField pOrder)
		{
			OrderField f = DicOrderField.GetOrAdd(pOrder.OrderId, new OrderField());
			foreach (var info in pOrder.GetType().GetFields())
			{
				f.GetType().GetField(info.Name).SetValue(f, Convert.ChangeType(info.GetValue(pOrder), f.GetType().GetField(info.Name).FieldType));
			}
			f.Status = OrderStatus.Canceled;
			if (_OnRtnCancel != null)
			{
				_OnRtnCancel(this, new OrderArgs
				{
					Value = f,
				});
			}
		}

		void _import_OnRspQryTrade(TradeField pTrade, bool pLast)
		{
			//无数据时,也会返回一条空记录
			if (string.IsNullOrEmpty(pTrade.InstrumentID))
			{
				return;
			}
			TradeField f = DicTradeField.GetOrAdd(pTrade.TradeID + (int)pTrade.Direction, new TradeField
			{
				TradeID = pTrade.TradeID + (int)pTrade.Direction,
			});
			foreach (var info in pTrade.GetType().GetFields())
			{
				if (info.Name == "TradeID")
					continue;
				f.GetType().GetField(info.Name).SetValue(f, Convert.ChangeType(info.GetValue(pTrade), f.GetType().GetField(info.Name).FieldType));
			}
			OrderField of;
			if (DicOrderField.TryGetValue(pTrade.OrderID, out of))
			{
				int preTrade = of.Volume - of.VolumeLeft;
				of.AvgPrice = (of.AvgPrice * preTrade + pTrade.Price * pTrade.Volume) / (preTrade + pTrade.Volume);
				of.TradeTime = pTrade.TradeTime;
				of.VolumeLeft -= pTrade.Volume;
			}
		}

		void _import_OnRspQryOrder(OrderField pField, bool pLast)
		{
			//无数据时,也会返回一条空记录
			if (string.IsNullOrEmpty(pField.InstrumentID))
			{
				return;
			}
			OrderField f = DicOrderField.GetOrAdd(pField.OrderId, new OrderField());
			foreach (var info in pField.GetType().GetFields())
			{
				f.GetType().GetField(info.Name).SetValue(f, Convert.ChangeType(info.GetValue(pField), f.GetType().GetField(info.Name).FieldType));
			}
		}

		void _import_OnRspQryTradingAccount(TradingAccount pAccount)
		{
			foreach (var info in pAccount.GetType().GetFields())
			{
				TradingAccount.GetType().GetField(info.Name).SetValue(TradingAccount, Convert.ChangeType(info.GetValue(pAccount), TradingAccount.GetType().GetField(info.Name).FieldType));
			}
		}

		void _import_OnRspQryPositiont(PositionField pField, bool pLast)
		{
			//无数据时,也会返回一条空记录
			if (string.IsNullOrEmpty(pField.InstrumentID))
			{
				return;
			}
			PositionField f = DicPositionField.GetOrAdd(pField.InstrumentID + "_" + pField.Direction, new PositionField());
			foreach (var info in pField.GetType().GetFields())
			{
				f.GetType().GetField(info.Name).SetValue(f, Convert.ChangeType(info.GetValue(pField), f.GetType().GetField(info.Name).FieldType));
			}
		}

		void _import_OnRspQryInstrument(InstrumentField pInstrument, bool pLast)
		{
			InstrumentField f = DicInstrumentField.GetOrAdd(pInstrument.InstrumentID, new InstrumentField());
			foreach (var info in pInstrument.GetType().GetFields())
			{
				f.GetType().GetField(info.Name).SetValue(f, info.GetValue(pInstrument));
			}
		}

		void _import_OnRspUserLogout(int pReason)
		{
			IsLogin = false;
			if (_OnRspUserLogout != null)
			{
				_OnRspUserLogout(this, new IntEventArgs
				{
					Value = pReason,
				});
			}
		}

		void _import_OnRspUserLogin(int pErrId)
		{
			IsLogin = pErrId == 0;
			if (IsLogin)
			{
				TradingDay = _proxy.GetTradingDay();
			}
			if (_OnRspUserLogin != null)
			{
				_OnRspUserLogin(this, new IntEventArgs
				{
					Value = pErrId,
				});
			}
		}

		void _import_OnFrontConnected()
		{
			if (_OnFrontConnected != null)
			{
				_OnFrontConnected(this, new EventArgs());
			}
		}

		public int ReqConnect()
		{
			return _proxy.ReqConnect(Server);
		}

		public int ReqUserLogin()
		{
			return _proxy.ReqUserLogin(Investor, Password, Broker);
		}

		public void ReqUserLogout()
		{
			_proxy.ReqUserLogout();
		}

		/// <summary>
		/// 
		/// </summary>
		/// <param name="pInstrument"></param>
		/// <param name="pDirection"></param>
		/// <param name="pOffset"></param>
		/// <param name="pPrice"></param>
		/// <param name="pVolume"></param>
		/// <param name="pHedge"></param>
		/// <returns>正确返回0</returns>
		public int ReqOrderInsert(string pInstrument, DirectionType pDirection, OffsetType pOffset, double pPrice, int pVolume, HedgeType pHedge = HedgeType.Speculation, OrderType pType = OrderType.Limit)
		{
			return _proxy.ReqOrderInsert(pInstrument, pDirection, pOffset, pPrice, pVolume, pHedge, pType);
		}

		public int ReqOrderAction(int pOrderId)
		{
			return _proxy.ReqOrderAction(pOrderId);
		}

		public int ReqQryOrder()
		{
			return _proxy.ReqQryOrder();
		}

		public int ReqQryTrade()
		{
			return _proxy.ReqQryTrade();
		}

		public int ReqQryPosition()
		{
			return _proxy.ReqQryPosition();
		}

		public int ReqQryAccount()
		{
			return _proxy.ReqQryAccount();
		}
	}
}
