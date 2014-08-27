using System;
using System.Linq;
using System.Threading;
using Quote2015;
using Trade2015;

namespace ConsoleProxy
{
	class Program
	{
		static private string _inst;
		private static double _LastPrice = double.NaN;

		private static int _orderId;

		//输入：q1ctp /t1ctp /q2xspeed /t2speed
		private static void Main(string[] args)
		{
			Console.WriteLine("选择接口:\t1-CTP  2-xSpeed  3-Femas");
			char c = Console.ReadKey(true).KeyChar;

			Trade t;
			Quote q;
		R:
			switch (c)
			{
				case '1': //CTP
					t = new Trade("ctp_trade_proxy.dll")
					{
						Server = "tcp://211.95.40.130:51205",
						Broker = "1017",
					};
					q = new Quote("ctp_quote_proxy.dll")
					{
						Server = "tcp://211.95.40.130:51213",
						Broker = "1017",
					};
					break;
				case '2': //xSpeed
					t = new Trade("xSpeed_trade_proxy.dll")
					{
						Server = "tcp://203.187.171.250:10910",
						Broker = "0001",
					};
					q = new Quote("xSpeed_quote_proxy.dll")
					{
						Server = "tcp://203.187.171.250:10915",
						Broker = "0001",
					};
					break;
				case '3': //femas
					t = new Trade("femas_trade_proxy.dll")
					{
						Server = "tcp://116.228.53.149:6666",
						Broker = "0001",
					};
					q = new Quote("femas_quote_proxy.dll")
					{
						Server = "tcp://116.228.53.149:6888",
						Broker = "0001",
					};
					break;
				default:
					Console.WriteLine("请重新选择");
					goto R;
			}
			Console.WriteLine("请输入帐号:");
			t.Investor = q.Investor = Console.ReadLine();
			Console.WriteLine("请输入密码:");
			t.Password = q.Password = Console.ReadLine();

			q.OnFrontConnected += (sender, e) =>
			{
				Console.WriteLine("OnFrontConnected");
				q.ReqUserLogin();
			};
			q.OnRspUserLogin += (sender, e) => Console.WriteLine("OnRspUserLogin:{0}", e.Value);
			q.OnRspUserLogout += (sender, e) => Console.WriteLine("OnRspUserLogout:{0}", e.Value);
			q.OnRtnError += (sender, e) => Console.WriteLine("OnRtnError:{0}=>{1}", e.ErrorID, e.ErrorMsg);
			//q.OnRtnTick += (sender, e) => Console.WriteLine("OnRtnTick:{0}", e.Tick);


			t.OnFrontConnected += (sender, e) => t.ReqUserLogin();
			t.OnRspUserLogin += (sender, e) =>
			{
				Console.WriteLine("OnRspUserLogin:{0}", e.Value);
				if (e.Value == 0)
					q.ReqConnect();
			};
			t.OnRspUserLogout += (sender, e) => Console.WriteLine("OnRspUserLogout:{0}", e.Value);
			t.OnRtnCancel += (sender, e) => Console.WriteLine("OnRtnCancel:{0}", e.Value.OrderId);
			t.OnRtnError += (sender, e) => Console.WriteLine("OnRtnError:{0}=>{1}", e.ErrorID, e.ErrorMsg);
			t.OnRtnExchangeStatus += (sender, e) => Console.WriteLine("OnRtnExchangeStatus:{0}=>{1}", e.Exchange, e.Status);
			t.OnRtnNotice += (sender, e) => Console.WriteLine("OnRtnNotice:{0}", e.Value);
			t.OnRtnOrder += (sender, e) =>
			{
				Console.WriteLine("OnRtnOrder:{0}", e.Value);
				_orderId = e.Value.OrderId;
			};
			t.OnRtnTrade += (sender, e) => Console.WriteLine("OnRtnTrade:{0}", e.Value.TradeID);

			t.ReqConnect();
			Thread.Sleep(1000);

			if (!t.IsLogin)
				goto R;
			Console.WriteLine(t.DicInstrumentField.Aggregate("\r\n合约", (cur, n) => cur + "\t" + n.Value.InstrumentID));
					
			string inst = string.Empty;
			Console.WriteLine("请输入合约:");
			inst = Console.ReadLine();
			q.ReqSubscribeMarketData(inst);

		Inst:
			Console.WriteLine("q:退出  1-BK  2-SP  3-SK  4-BP  5-撤单");
			Console.WriteLine("a-交易所状态  b-委托  c-成交  d-持仓  e-合约  f-权益");

			DirectionType dire = DirectionType.Buy;
			OffsetType offset = OffsetType.Open;
			c = Console.ReadKey().KeyChar;
			switch (c)
			{
				case '1':
					dire = DirectionType.Buy;
					offset = OffsetType.Open;
					break;
				case '2':
					dire = DirectionType.Sell;
					offset = OffsetType.Close;
					break;
				case '3':
					dire = DirectionType.Sell;
					offset = OffsetType.Open;
					break;
				case '4':
					dire = DirectionType.Buy;
					offset = OffsetType.Close;
					break;
				case '5':
					t.ReqOrderAction(_orderId);
					break;
				case 'a':
					Console.WriteLine(t.DicExcStatus.Aggregate("\r\n交易所状态", (cur, n) => cur + "\r\n" + n.Key + "=>" + n.Value));
					break;
				case 'b':
					Console.ForegroundColor = ConsoleColor.Cyan;
					Console.WriteLine(t.DicOrderField.Aggregate("\r\n委托", (cur, n) => cur + "\r\n"
						+ n.Value.GetType().GetFields().Aggregate(string.Empty, (f, v) => f + string.Format("{0,12}", v.GetValue(n.Value)))));
					break;
				case 'c':
					Console.ForegroundColor = ConsoleColor.DarkCyan;
					Console.WriteLine(t.DicTradeField.Aggregate("\r\n成交", (cur, n) => cur + "\r\n"
						+ n.Value.GetType().GetFields().Aggregate(string.Empty, (f, v) => f + string.Format("{0,12}", v.GetValue(n.Value)))));
					break;
				case 'd': //持仓
					Console.ForegroundColor = ConsoleColor.DarkGreen;
					Console.WriteLine(t.DicPositionField.Aggregate("\r\n持仓", (cur, n) => cur + "\r\n"
						+ n.Value.GetType().GetFields().Aggregate(string.Empty, (f, v) => f + string.Format("{0,12}", v.GetValue(n.Value)))));
					break;
				case 'e':
					Console.WriteLine(t.DicInstrumentField.Aggregate("\r\n合约", (cur, n) => cur + "\r\n"
						+ n.Value.GetType().GetFields().Aggregate(string.Empty, (f, v) => f + string.Format("{0,12}", v.GetValue(n.Value)))));
					break;
				case 'f':
					Console.WriteLine(t.TradingAccount.GetType().GetFields().Aggregate("\r\n权益\t", (cur, n) => cur + ","
						+ n.GetValue(t.TradingAccount).ToString()));
					break;
				case 'q':
					q.ReqUserLogout();
					t.ReqUserLogout();
					Thread.Sleep(2000); //待接口处理后续操作
					Environment.Exit(0);
					break;
			}
			if (c >= '1' && c <= '4')
			{
				Console.WriteLine("请选择委托类型: 1-限价  2-市价  3-FAK  4-FOK");
				OrderType ot = OrderType.Limit;
				switch (Console.ReadKey().KeyChar)
				{
					case '2':
						ot = OrderType.Market;
						break;
					case '3':
						ot = OrderType.FAK;
						break;
					case '4':
						ot = OrderType.FOK;
						break;
				}
				MarketData tick;
				if (q.DicTick.TryGetValue(inst, out tick))
				{
					double price = dire == DirectionType.Buy ? tick.AskPrice : tick.BidPrice;
					_orderId = -1;
					Console.WriteLine(t.ReqOrderInsert(inst, dire, offset, price, 1, pType: ot));
					for (int i = 0; i < 3; ++i)
					{
						Thread.Sleep(200);
						if (-1 != _orderId)
						{
							Console.WriteLine("委托标识:" + _orderId);
							break;
						}
					}
				}
			}
			goto Inst;
		}
	}
}
