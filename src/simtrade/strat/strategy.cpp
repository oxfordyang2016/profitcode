#include <iostream>
#include <string>

#include "simtrade/strat/strategy.h"

Strategy::Strategy(std::string main_ticker, std::string hedge_ticker, int maxpos, double tick_size, TimeController tc, int contract_size, bool enable_stdout, bool enable_file)
  : position_ready(false),
    is_started(false),
    order_ref(0),
    current_status(StratStatus::OFF),
    max_pos(maxpos),
    e_s(enable_stdout),
    e_f(enable_file),
    poscapital(0.0),
    min_price(tick_size),
    price_control(10.0*min_price),
    avgcost_main(0.0),
    avgcost_hedge(0.0),
    edurance(1.0*min_price),
    m_tc(tc),
    m_contract_size(contract_size) {
  if (e_f) {
    order_file = fopen("order.txt", "w");
    exchange_file = fopen("exchange.txt", "w");
  }
  pthread_mutex_init(&order_ref_mutex, NULL);
  pthread_mutex_init(&add_size_mutex, NULL);
  pthread_mutex_init(&mod_mutex, NULL);
  snprintf(main_shot.ticker, sizeof(main_shot.ticker), "%s", main_ticker.c_str());
  snprintf(hedge_shot.ticker, sizeof(hedge_shot.ticker), "%s", hedge_ticker.c_str());
  sender = new Sender("order");
  RequestQryPos();
}

Strategy::~Strategy() {
  delete sender;
  if (e_f) {
    fclose(order_file);
    fclose(exchange_file);
  }
}

double Strategy::OrderPrice(std::string contract, OrderSide::Enum side, bool control_price) {
  if (contract == hedge_shot.ticker) {
    return (side == OrderSide::Buy)?hedge_shot.asks[0]:hedge_shot.bids[0];
  }
  bool is_close = false;
  if ((position_map[main_shot.ticker] > 0 && side == OrderSide::Sell) || (position_map[main_shot.ticker] < 0 && side == OrderSide::Buy)) {
    is_close = true;
  }
  if (is_close && IsHedged()) {
    double balance_price = CalBalancePrice();
    fprintf(order_file, "close report: np is %d, hedgep is %d, avgcost hedge and main are %lf %lf, hedge ask bid is %lf %lf, main ask bid is %lf %lf, balanceprice is %lf\n", position_map[main_shot.ticker], position_map[hedge_shot.ticker], avgcost_hedge, avgcost_main, hedge_shot.asks[0], hedge_shot.bids[0], main_shot.asks[0], main_shot.bids[0], balance_price);
    if (side == OrderSide::Buy) {
      if (balance_price <= main_shot.bids[0]) {
        fprintf(order_file, "balance report: pricecut buy: %lf->%lf\n", main_shot.bids[0], balance_price);
        return balance_price - min_price;
      } else if (main_shot.bids[0] < balance_price && balance_price <= main_shot.asks[0]) {
        return main_shot.bids[0];
      } else {
        return main_shot.asks[0];
      }
    } else {
      if (balance_price >= main_shot.asks[0]) {
        fprintf(order_file, "balance report: pricecut sell: %lf->%lf\n", main_shot.bids[0], balance_price);
        return balance_price + min_price;
      } else if (main_shot.bids[0] <= balance_price && balance_price < main_shot.asks[0]) {
        return main_shot.asks[0];
      } else {
        return main_shot.bids[0];
      }
    }
  }

  if (is_close && !IsHedged()) {
    fprintf(order_file, "close report: np is %d, hedgep is %d, avgcost hedge and main are %lf %lf, hedge ask bid is %lf %lf, main ask bid is %lf %lf\n", position_map[main_shot.ticker], position_map[hedge_shot.ticker], avgcost_hedge, avgcost_main, hedge_shot.asks[0], hedge_shot.bids[0], main_shot.asks[0], main_shot.bids[0]);
    return (side == OrderSide::Buy)?main_shot.bids[0]-price_control:main_shot.asks[0]+price_control;
  }

  // is open and position not zero, so it's a add position operation, to avoid add to max, add control
  if (control_price) {
    return (side == OrderSide::Buy)?main_shot.bids[0]-price_control:main_shot.asks[0]+price_control;
  }

  return (side == OrderSide::Buy)?main_shot.bids[0]:main_shot.asks[0];
}

int Strategy::GenerateUniqueOrderRef() {
  printf("getting unique orderref\n");
  pthread_mutex_lock(&order_ref_mutex);
  order_ref = order_ref + 1;
  pthread_mutex_unlock(&order_ref_mutex);
  printf("got unique orderref\n");
  return order_ref;
}

void Strategy::SimpleHandle(int line) {
  printf("unexpected error in line %d\n", line);
}

void Strategy::NewOrder(std::string contract, OrderSide::Enum side, int size, bool control_price) {
    if (size == 0) {
      return;
    }
    pthread_mutex_lock(&order_ref_mutex);
    Order* order = new Order();
    snprintf(order->contract, sizeof(order->contract), "%s", contract.c_str());
    order->price = OrderPrice(contract, side, control_price);
    order->size = size;
    order->side = side;
    order->order_ref = order_ref++;
    order->action = OrderAction::NewOrder;
    order->status = OrderStatus::SubmitNew;
    if (e_s) {
      order->Show(stdout);
    }
    if (e_f) {
      order->Show(order_file);
    }
    pthread_mutex_unlock(&order_ref_mutex);
    sender->Send(*order);
    if (strcmp(order->contract, main_shot.ticker) == 0) {
      main_order_map[order->order_ref] = order;
    } else if (strcmp(order->contract, hedge_shot.ticker) == 0) {
      hedge_order_map[order->order_ref] = order;
    } else {
      // TODO(nick): handle error
      SimpleHandle(67);
    }
}

void Strategy::ModOrder(Order* o) {
  printf("modorder lock\n");
  pthread_mutex_lock(&mod_mutex);
  o->price = OrderPrice(o->contract, o->side);
  o->status = OrderStatus::Modifying;
  o->action = OrderAction::ModOrder;
  pthread_mutex_unlock(&mod_mutex);
  printf("release modorder lock\n");
  if (e_s) {
    o->Show(stdout);
  }
  if (e_f) {
    o->Show(order_file);
  }
  sender->Send(*o);
}

void Strategy::Start() {
  /*
  cout << "enter start" << endl;
  if (current_status == StratStatus::OFF) {
    NewOrder(main_shot.ticker, OrderSide::Buy);
    NewOrder(main_shot.ticker, OrderSide::Sell);
  } else {  // handle error
    SimpleHandle(121);
  }
  */
  int pos = position_map[main_shot.ticker];
  if (pos > 0) {
    NewOrder(main_shot.ticker, OrderSide::Sell, pos);
    if (pos < max_pos) {
      NewOrder(main_shot.ticker, OrderSide::Buy);
    }
  } else if (pos < 0) {
    NewOrder(main_shot.ticker, OrderSide::Buy, -pos);
    if (-pos < max_pos) {
      NewOrder(main_shot.ticker, OrderSide::Sell);
    }
  } else {
    NewOrder(main_shot.ticker, OrderSide::Buy);
    NewOrder(main_shot.ticker, OrderSide::Sell);
  }
}

void Strategy::ModerateMainOrders() {
  for (std::tr1::unordered_map<int, Order*>::iterator it = main_order_map.begin(); it != main_order_map.end(); it++) {
    Order* o = it->second;
    if (o->Valid()) {
      double reasonable_price = OrderPrice(main_shot.ticker, o->side);
      // if (o->side == OrderSide::Buy && !DoubleEqual(o->price, OrderPrice(main_shot.ticker, o->side))) {
      if (!DoubleEqual(o->price, reasonable_price) && !PriceChange(o->price, reasonable_price, o->side)) {
        printf("edure price change: from %lf->%lf, side is %s\n", o->price, reasonable_price, OrderSide::ToString(o->side));
      }
      if (o->side == OrderSide::Buy && PriceChange(o->price, reasonable_price, o->side)) {
        printf("modify order %d, price:%lf->%lf\n", o->order_ref, o->price, reasonable_price);
        ModOrder(o);
      // } else if (o->side == OrderSide::Sell && !DoubleEqual(o->price, OrderPrice(main_shot.ticker, o->side))) {
      } else if (o->side == OrderSide::Sell && PriceChange(o->price, reasonable_price, o->side)) {
        printf("modify order %d, price:%lf->%lf\n", o->order_ref, o->price, reasonable_price);
        ModOrder(o);
      } else {
        // TODO(nick): handle error
      }
    }
  }
}

void Strategy::ModerateHedgeOrders() {
  for (std::tr1::unordered_map<int, Order*>::iterator it = hedge_order_map.begin(); it != hedge_order_map.end(); it++) {
    Order* o = it->second;
    if (o->Valid()) {
      if (o->side == OrderSide::Buy && !DoubleEqual(o->price, hedge_shot.asks[0])) {
        fprintf(order_file, "Slip point report:modify buy order %d: %lf->%lf\n", o->order_ref, o->price, hedge_shot.asks[0]);
        ModOrder(o);
      } else if (o->side == OrderSide::Sell && !DoubleEqual(o->price, hedge_shot.bids[0])) {
        fprintf(order_file, "Slip point report:modify sell order %d: %lf->%lf\n", o->order_ref, o->price, hedge_shot.asks[0]);
        ModOrder(o);
      } else {
        // TODO(nick): handle error
      }
    }
  }
}

void Strategy::CloseAllTodayPos() {
  int main_pos = position_map[main_shot.ticker];
  OrderSide::Enum main_side = (main_pos > 0)?OrderSide::Sell:OrderSide::Buy;
  int hedge_pos = position_map[hedge_shot.ticker];
  OrderSide::Enum hedge_side = (hedge_pos > 0)?OrderSide::Sell:OrderSide::Buy;

  pthread_mutex_lock(&order_ref_mutex);
  Order* main_order = new Order();
  snprintf(main_order->contract, sizeof(main_order->contract), "%s", main_shot.ticker);
  main_order->price = (main_side == OrderSide::Buy)?main_shot.asks[0]+5*min_price:main_shot.bids[0]-5*min_price;
  main_order->size = abs(main_pos);
  main_order->side = main_side;
  main_order->order_ref = order_ref++;
  main_order->action = OrderAction::NewOrder;
  main_order->status = OrderStatus::SubmitNew;
  if (e_s) {
    main_order->Show(stdout);
  }
  if (e_f) {
    main_order->Show(order_file);
  }
  pthread_mutex_unlock(&order_ref_mutex);
  sender->Send(*main_order);

  NewOrder(hedge_shot.ticker, hedge_side, abs(hedge_pos));

  printf("close all:position is %d %d avgcost is %lf %lf, market price is %lf %lf\n", main_pos, hedge_pos, avgcost_main, avgcost_hedge, main_order->price, OrderPrice(hedge_shot.ticker, hedge_side));
}

void Strategy::UpdateData(MarketSnapshot shot) {
  if (!m_tc.TimeValid() && is_started) {
    printf("time to sleep!\n");
    ClearValidOrder();
    sleep(3);
    ClearValidOrder();  // make sure no orders
    is_started = false;
    if (m_tc.TimeClose()) {
      CloseAllTodayPos();
    }
    return;
  }
  if (m_tc.TimeValid() && IsReady() && !is_started) {
    printf("time to wake up!\n");
    Start();
    is_started = true;
    return;
  }
  std::string shot_ticker = shot.ticker;
  if (shot_ticker == main_shot.ticker) {
    main_shot = shot;
    ModerateMainOrders();
  } else if (shot_ticker == hedge_shot.ticker) {
    hedge_shot = shot;
    ModerateHedgeOrders();
  } else {
    // SimpleHandle(167);
  }
}

void Strategy::RequestQryPos() {
  position_map.clear();
  Order* o = new Order();
  o->action = OrderAction::QueryPos;
  sender->Send(*o);
}

bool Strategy::IsReady() {
  if (main_shot.is_initialized && hedge_shot.is_initialized && position_ready) {
    return true;
  }
  if (!position_ready) {
    printf("waiting position query finish!\n");
  }
  return false;
}

void Strategy::CancelAllMain() {
  printf("Enter Cancel ALL Main\n");
  pthread_mutex_lock(&mod_mutex);
  for (std::tr1::unordered_map<int, Order*>::iterator it = main_order_map.begin(); it != main_order_map.end(); it++) {
    Order* o = it->second;
    if (o->Valid()) {
      o->action = OrderAction::CancelOrder;
      o->status = OrderStatus::Cancelling;
      o->order_ref = it->first;
      if (e_s) {
        o->Show(stdout);
      }
      if (e_f) {
        o->Show(order_file);
      }
      sender->Send(*o);
    } else if (o->status == OrderStatus::Modifying) {
      o->action = OrderAction::CancelOrder;
      o->status = OrderStatus::Cancelling;
    }
  }
  pthread_mutex_unlock(&mod_mutex);
}

void Strategy::CancelAllHedge() {
  printf("Enter Cancel ALL Hedge\n");
  pthread_mutex_lock(&mod_mutex);
  for (std::tr1::unordered_map<int, Order*>::iterator it = hedge_order_map.begin(); it != hedge_order_map.end(); it++) {
    Order* o = it->second;
    if (o->Valid()) {
      o->action = OrderAction::CancelOrder;
      o->status = OrderStatus::Cancelling;
      o->order_ref = it->first;
      if (e_s) {
        o->Show(stdout);
      }
      if (e_f) {
        o->Show(order_file);
      }
      sender->Send(*o);
    } else if (o->status == OrderStatus::Modifying) {
      o->action = OrderAction::CancelOrder;
      o->status = OrderStatus::Cancelling;
    }
  }
  pthread_mutex_unlock(&mod_mutex);
}

void Strategy::ClearValidOrder() {
  CancelAllMain();
  CancelAllHedge();
}

void Strategy::DelOrder(int ref) {
  std::tr1::unordered_map<int, Order*>::iterator it = main_order_map.find(ref);
  if (it == main_order_map.end()) {
    it = hedge_order_map.find(ref);
    if (it == hedge_order_map.end()) {
      printf("order %d not found\n", ref);
      return;
    }
    hedge_order_map.erase(it);
  } else {
    main_order_map.erase(it);
  }
}

void Strategy::UpdatePos(Order* o, ExchangeInfo info) {
  std::string contract = o->contract;
  int previous_pos = position_map[contract];
  int trade_size = (o->side == OrderSide::Buy)?o->size:-o->size;
  double trade_price = info.trade_price;
  position_map[contract] += trade_size;
  bool is_close = TradeClose(contract, trade_size);
  if (!is_close) {  // only update avgcost when open traded
    UpdateAvgCost(contract, trade_price, trade_size);
  }
  OrderSide::Enum sd;
  OrderSide::Enum reverse_sd;
  if (trade_size > 0) {
    sd = OrderSide::Buy;
    reverse_sd = OrderSide::Sell;
  } else {
    sd = OrderSide::Sell;
    reverse_sd = OrderSide::Buy;
  }

  if (contract == main_shot.ticker) {
    int main_pos = position_map[contract];
    if (!is_close) {  // open traded
      if (main_pos*trade_size == 1) {  // pos 0->1: cancel open order, add close order, add open order
        printf("opentraded and pos=1\n");
        CancelAllMain();  // cancel open
        NewOrder(main_shot.ticker, reverse_sd);
        NewOrder(main_shot.ticker, sd, true);  // add open
      } else {  // pos > 1
        printf("opentraded and pos>1\n");
        if (abs(main_pos) < max_pos) {  // add close, add open
          AddCloseOrderSize(reverse_sd);
          NewOrder(main_shot.ticker, sd, true);  // add open
        } else if (abs(main_pos) == max_pos) {  // add close only
          AddCloseOrderSize(reverse_sd);
        } else {
          SimpleHandle(240);
        }
      }
    } else {  // close traded
      if (abs(previous_pos) == max_pos) {
        NewOrder(main_shot.ticker, reverse_sd);  // make up one open
      }
      if (main_pos == 0) {
        NewOrder(main_shot.ticker, sd);  // add open
        return;
      }
    }
  } else if (contract == hedge_shot.ticker) {
  } else {
    SimpleHandle(251);
  }
}

void Strategy::UpdateExchangeInfo(ExchangeInfo info) {
  printf("enter UpdateExchangeInfo\n");
  if (e_s) {
    info.Show(stdout);
  }
  if (e_f) {
    info.Show(exchange_file);
  }
  InfoType::Enum t = info.type;

  if (t == InfoType::Position) {
    if (position_ready) {  // ignore positioninfo after ready
      return;
    }
    if ((info.trade_price < 0.00001 || abs(info.trade_size) == 0) && strcmp(info.contract, "positionend") != 0) {
      return;
    }
    if (strcmp(info.contract, main_shot.ticker) == 0) {
      if (position_map[info.contract] + info.trade_size == 0) {
        avgcost_main = 0.0;
      } else {
        avgcost_main = (info.trade_price/m_contract_size*info.trade_size + avgcost_main*position_map[info.contract])/(position_map[info.contract] + info.trade_size);
      }
    } else if (strcmp(info.contract, hedge_shot.ticker) == 0) {
      if (position_map[info.contract] + info.trade_size == 0) {
        avgcost_hedge = 0.0;
      } else {
        avgcost_hedge = (info.trade_price/m_contract_size*info.trade_size + avgcost_hedge*position_map[info.contract])/(position_map[info.contract] + info.trade_size);
      }
    } else if (strcmp(info.contract, "positionend") == 0) {
      printf("position recv finished: %s:%d@%lf %s:%d@%lf\n", main_shot.ticker, position_map[main_shot.ticker], avgcost_main, hedge_shot.ticker, position_map[hedge_shot.ticker], avgcost_hedge);
      position_ready = true;
      return;
    } else {
      printf("recv unknown contract %s\n", info.contract);
      return;
    }
    position_map[info.contract] += info.trade_size;
    return;
  }
  std::tr1::unordered_map<int, Order*>::iterator it = main_order_map.find(info.order_ref);
  if (it == main_order_map.end()) {  // not main
    it = hedge_order_map.find(info.order_ref);
    if (it == hedge_order_map.end()) {
      printf("unknown orderref!%d\n", info.order_ref);
      return;
    }
  }
  Order* order = it->second;

  switch (t) {
    case InfoType::Acc:
    {
      if (order->status == OrderStatus::SubmitNew) {
        order->status = OrderStatus::New;
      } else {
        // TODO(nick): ignore other state?
        return;
      }
    }
      // filter the mess order of info arrived
      break;
    case InfoType::Rej:
    {
      order->status = OrderStatus::Rejected;
      DelOrder(info.order_ref);
    }
      break;
    case InfoType::Cancelled:
    {
      order->status = OrderStatus::Cancelled;
      DelOrder(info.order_ref);
      if (order->action == OrderAction::ModOrder) {
        NewOrder(order->contract, order->side, order->size);
      }
    }
      break;
    case InfoType::CancelRej:
    {
      if (order->status == OrderStatus::Filled) {
        printf("cancelrej bc filled!%d\n", order->order_ref);
        return;
      }
      order->status = OrderStatus::CancelRej;
      printf("cancel rej for order %d\n", info.order_ref);
      // TODO(nick):
      // case: cancel filled: ignore
      // case: not permitted in this time, wait to cancel
      // other reason: make up for the cancel failed
    }
      break;
    case InfoType::Filled:
    {
      order->traded_size += info.trade_size;
      if (order->size == order->traded_size) {
        order->status = OrderStatus::Filled;
        DelOrder(info.order_ref);
      } else {
        order->status = OrderStatus::Pfilled;
      }
      if (strcmp(order->contract, main_shot.ticker) == 0) {
        NewOrder(hedge_shot.ticker, (order->side == OrderSide::Buy)?OrderSide::Sell : OrderSide::Buy, info.trade_size);
      } else if (strcmp(order->contract, hedge_shot.ticker) == 0) {
      } else {
        // TODO(nick): handle error
        SimpleHandle(322);
      }
      UpdatePos(order, info);
    }
      break;
    case InfoType::Pfilled:
      // TODO(nick): need to realize
      break;
    default:
      // TODO(nick): handle unknown info
      SimpleHandle(331);
      break;
  }
}
