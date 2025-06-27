// This C++ program implements three trading strategies: RSI, MACD, and Bollinger Bands
// Assumes presence of a `Candle` struct with at least a `close` member
#include <iostream>
#include <vector>
#include <cmath>
#include <numeric>

using namespace std;

struct Candle {
    double close;
};

struct TradeResult {
    double success_rate;
    double avg_return;
    int total_trades;
    vector<int> signal_positions;
};

// Helper: Simple Moving Average
double sma(const vector<double>& data, int start, int period) {
    if (start < period - 1) return 0.0;
    double sum = 0.0;
    for (int i = start - period + 1; i <= start; ++i)
        sum += data[i];
    return sum / period;
}

// Helper: Exponential Moving Average
double ema(const vector<double>& data, int start, int period, double prev_ema) {
    double k = 2.0 / (period + 1);
    return data[start] * k + prev_ema * (1 - k);
}

// --- 1. RSI Strategy ---
double calculate_rsi(const vector<double>& closes, int current_index, int period = 14) {
    if (current_index < period) return 50.0;
    double gain = 0.0, loss = 0.0;
    for (int i = current_index - period + 1; i <= current_index; ++i) {
        double change = closes[i] - closes[i - 1];
        if (change > 0) gain += change;
        else loss -= change;
    }
    double rs = loss == 0 ? 0 : gain / loss;
    return 100.0 - (100.0 / (1.0 + rs));
}

TradeResult run_rsi_strategy(const vector<Candle>& candles, double profit_threshold) {
    vector<double> closes;
    for (const auto& c : candles) closes.push_back(c.close);

    vector<int> signals(closes.size(), 0);
    int trades = 0, profitables = 0;
    double total_ret = 0;
    bool holding = false;
    double entry = 0.0;

    for (size_t i = 15; i < closes.size(); ++i) {
        double rsi = calculate_rsi(closes, i);
        if (!holding && rsi < 30) {
            entry = closes[i];
            holding = true;
            signals[i] = 1; // buy
        } else if (holding && rsi > 70) {
            double ret = (closes[i] - entry) / entry;
            total_ret += ret;
            if (ret > profit_threshold) ++profitables;
            ++trades;
            holding = false;
            signals[i] = -1; // sell
        }
    }

    return {trades ? (double)profitables / trades * 100 : 0, trades ? (total_ret / trades) * 100 : 0, trades, signals};
}

// --- 2. MACD Strategy ---
TradeResult run_macd_strategy(const vector<Candle>& candles, double profit_threshold) {
    vector<double> closes;
    for (const auto& c : candles) closes.push_back(c.close);

    vector<double> macd, signal;
    double ema12 = closes[11], ema26 = closes[25];
    for (size_t i = 26; i < closes.size(); ++i) {
        ema12 = ema(closes, i, 12, ema12);
        ema26 = ema(closes, i, 26, ema26);
        macd.push_back(ema12 - ema26);
    }

    signal.push_back(macd[0]);
    for (size_t i = 1; i < macd.size(); ++i)
        signal.push_back(ema(macd, i, 9, signal.back()));

    int trades = 0, profitables = 0;
    double total_ret = 0;
    vector<int> signals(closes.size(), 0);
    bool holding = false;
    double entry = 0.0;

    for (size_t i = 1; i < signal.size(); ++i) {
        size_t idx = i + 26;
        if (!holding && macd[i - 1] < signal[i - 1] && macd[i] > signal[i]) {
            holding = true;
            entry = closes[idx];
            signals[idx] = 1; // buy
        } else if (holding && macd[i - 1] > signal[i - 1] && macd[i] < signal[i]) {
            double ret = (closes[idx] - entry) / entry;
            total_ret += ret;
            if (ret > profit_threshold) ++profitables;
            ++trades;
            holding = false;
            signals[idx] = -1; // sell
        }
    }

    return {trades ? (double)profitables / trades * 100 : 0, trades ? (total_ret / trades) * 100 : 0, trades, signals};
}

// --- 3. Bollinger Bands Strategy ---
TradeResult run_bollinger_strategy(const vector<Candle>& candles, double profit_threshold) {
    vector<double> closes;
    for (const auto& c : candles) closes.push_back(c.close);

    int period = 20;
    vector<int> signals(closes.size(), 0);
    int trades = 0, profitables = 0;
    double total_ret = 0;
    bool holding = false;
    double entry = 0.0;

    for (size_t i = period; i < closes.size(); ++i) {
        double ma = sma(closes, i, period);
        double stddev = 0;
        for (int j = i - period + 1; j <= (int)i; ++j)
            stddev += pow(closes[j] - ma, 2);
        stddev = sqrt(stddev / period);

        double upper = ma + 2 * stddev;
        double lower = ma - 2 * stddev;

        if (!holding && closes[i] < lower) {
            holding = true;
            entry = closes[i];
            signals[i] = 1; // buy
        } else if (holding && closes[i] > upper) {
            double ret = (closes[i] - entry) / entry;
            total_ret += ret;
            if (ret > profit_threshold) ++profitables;
            ++trades;
            holding = false;
            signals[i] = -1; // sell
        }
    }

    return {trades ? (double)profitables / trades * 100 : 0, trades ? (total_ret / trades) * 100 : 0, trades, signals};
}

// Example usage (you can replace this with actual data and a proper main function)
int main() {
    vector<Candle> candles = {{100}, {101}, {102}, {98}, {96}, {94}, {92}, {93}, {95}, {97}, {99}, {101}, {100}, {102}, {103}, {105}, {104}, {106}, {107}, {109}, {110}, {111}, {113}, {112}, {114}, {115}, {117}, {116}};
    double threshold = 0.01; // 1%

    auto rsi_result = run_rsi_strategy(candles, threshold);
    auto macd_result = run_macd_strategy(candles, threshold);
    auto bb_result = run_bollinger_strategy(candles, threshold);

    cout << "RSI Strategy: Trades = " << rsi_result.total_trades << ", Success = " << rsi_result.success_rate << "%\n";
    cout << "MACD Strategy: Trades = " << macd_result.total_trades << ", Success = " << macd_result.success_rate << "%\n";
    cout << "Bollinger Bands Strategy: Trades = " << bb_result.total_trades << ", Success = " << bb_result.success_rate << "%\n";

    return 0;
}
