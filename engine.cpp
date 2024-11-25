#include <vector>
#include <utility>
#include <unordered_set>
#include <unordered_map>
#include <set>
#include <deque>
#include <iostream>

struct Square
{
    int x;
    int y;
    std::vector<int> valid_symbol;
};

/*
class Reel
{
    std::deque<int> values;


    void add_value(int value)
    {
        values.push_back(value);
    }

    void spin_forward()
    {
        int value = values.back();
        values.pop_back();
        values.push_front(value);
    }

    void spin_backward()
    {
        int value = values.front();
        values.pop_front();
        values.push_back(value);
    }


}*/

struct Reel
{
    std::vector<int> values;
    int start = 0;

    virtual void spin(int amount) = 0; // positive for forward //negative for backward

    virtual std::vector<int> get_current() = 0;

    Reel(std::vector<int> values)
    {
        this->values = values;
    }
};

struct FairStandardReel : Reel
{
    void spin(int amount) // positive for forward //negative for backward
    {
        start = amount % values.size();
    }

    std::vector<int> get_current()
    {
        std::vector<int> ret(values.begin() + start, values.end());
        ret.append_range(std::vector<int>(values.begin(), values.begin() + start));
        return ret;
    }

    FairStandardReel(std::vector<int> values) : Reel(values) {};
};

// template<typename BonusFeature, typename RNG>
class SlotMachine
{
    using column_t = std::vector<int>;
    std::vector<column_t> grid;

    std::vector<Reel *> reels;

    using payline_t = std::vector<Square>;
    std::vector<std::pair<bool, payline_t>> pay_lines;

    std::unordered_map<payline_t *, int> bets;

    int active_pay_lines;

    int max_bet;
    int bet_per_line;
    int total_bet_per_spin;
    // std::vector<BonusFeature> bonus_features;
    int coin_size;
    std::unordered_map<int, int> fixed_jackpots;
    int free_spins;
    float multiplier;
    float payout_percentage;
    std::unordered_map<payline_t *, int> pay_table;
    std::unordered_map<int, int> progressive_jackpots;
    // RNG random_number_generator;
    std::unordered_set<int> scatter_symbols;
    std::unordered_set<int> wild_symbols;
    int current_session_coins;

    int spin_number = 0;

public:
    SlotMachine() = default;
    SlotMachine(std::vector<Reel *> reels)
    {
        for (Reel *reel : reels)
        {
            this->reels.push_back(reel);
            grid.push_back(reel->get_current());
        }
    };

    void add_reel(Reel *reel)
    {
        this->reels.push_back(reel);
        this->grid.push_back(reel->get_current());
    }

    void add_pay_line(std::vector<Square> pay_line, bool is_active = true)
    {
        this->pay_lines.push_back({is_active, pay_line});
    }

    void set_pay_line_activation(int pay_line_index, bool activation)
    {
        this->pay_lines[pay_line_index].first = activation;
    }

    void swith_pay_line_activation(int pay_line_index)
    {
        if (!this->pay_lines[pay_line_index].first)
        {
            this->pay_lines[pay_line_index].first = false;
        }
        else
        {
            this->pay_lines[pay_line_index].first = true;
        }
    }

    void set_max_bet(int max_bet)
    {
        this->max_bet = max_bet;
    }

    void bet_on_pay_line(payline_t *pay_line, int coins)
    {
        this->bets[pay_line] = coins;
    }

    void play_bonus_feature(int bonus_feature_index)
    {
        // this->bonus_features[bonus_feature_index].play();
    }

    void set_pay(payline_t *pay_line, int pay)
    {
        this->pay_table[pay_line] = pay;
    }
    void add_scatter_symbol(int symbol)
    {
        this->scatter_symbols.insert(symbol);
    }
    void add_wild_symbol(int symbol)
    {
        this->wild_symbols.insert(symbol);
    }

    void add_fixed_jackpot(int jackpot, int coins)
    {
        this->fixed_jackpots.insert({jackpot, coins});
    }

    void adjust_fixed_jackpot(int jackpot, int coins)
    {
        if (this->fixed_jackpots.contains(jackpot))
        {
            this->fixed_jackpots[jackpot] += coins;
        }
    }

    void multiply_fixed_jackpot(int jackpot, float modifier)
    {
        if (this->fixed_jackpots.contains(jackpot))
        {
            this->fixed_jackpots[jackpot] *= modifier;
        }
    }

    void add_progressive_jackpot(int jackpot, int coins)
    {
        this->progressive_jackpots.insert({jackpot, coins});
    }

    void adjust_progressive_jackpot(int jackpot, int coins)
    {
        if (this->progressive_jackpots.contains(jackpot))
        {
            this->progressive_jackpots[jackpot] += coins;
        }
    }

    void multiply_progressive_jackpot(int jackpot, float modifier)
    {
        if (this->progressive_jackpots.contains(jackpot))
        {
            this->progressive_jackpots[jackpot] *= modifier;
        }
    }

    void set_frame(int real_index)
    {
        this->grid[real_index] = reels[real_index]->get_current();
    }

    bool payline_satisfied(payline_t payline)
    {
        for (Square s : payline)
        {
            bool symbol_found = false;
            for (int symbol : s.valid_symbol)
            {
                if (this->grid[s.y][s.x] == symbol)
                    symbol_found = true;
            }
            if(!symbol_found)
                return false;
        }
        return true;
    }

    std::vector<payline_t> find_wins()
    {
        std::vector<payline_t> pay_lines;
        for (const auto &[is_active, pay_line] : this->pay_lines)
        {
            if (is_active && this->payline_satisfied(pay_line))
            {
                std::cout << "winner winner chicken dinner" << std::endl;
                pay_lines.push_back(pay_line);
            }
        }
        return pay_lines;
    }

    bool spin()
    {
        int i = 0;
        for (Reel *reel : this->reels)
        {
            reel->spin(rand());
            this->set_frame(i++);
        }

        spin_number++;
        if(find_wins().empty())
        {
            return false;
        }
        else
        {
            return true;
        }

    }

    void print_grid() const
    {
        std::cout << "spin count: " << spin_number << std::endl;
        for (column_t coloumn : this->grid)
        {
            for (int val : coloumn)
            {
                std::cout << "|" << val;
            }

            std::cout << "|" << std::endl;
        }
        std::cout << std::endl
                  << std::endl;
    }
};

int main()
{
    SlotMachine sm;
    FairStandardReel reel_1(std::vector<int>({1, 2, 3, 4, 5}));
    FairStandardReel reel_2(std::vector<int>({1, 2, 3, 4, 5}));
    FairStandardReel reel_3(std::vector<int>({1, 2, 3, 4, 5}));
    FairStandardReel reel_4(std::vector<int>({1, 2, 3, 4, 5}));
    FairStandardReel reel_5(std::vector<int>({1, 2, 3, 4, 5}));

    sm.add_reel(&reel_1);
    sm.add_reel(&reel_2);
    sm.add_reel(&reel_3);
    sm.add_reel(&reel_4);
    sm.add_reel(&reel_5);
    sm.add_pay_line({{2, 0, {4, 5}}, {2, 1, {4, 5}}, {2, 2, {4, 5}}, {2, 3, {4, 5}}, {2, 4, {4, 5}}});
    
    while(!sm.spin())
        sm.print_grid();

    sm.print_grid();

}