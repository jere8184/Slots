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
    int symbol;
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

    void spin(int amount) //positive for forward //negative for backward
    {
        start = amount % values.size();
    }

    std::vector<int> get_current()
    {
        std::vector<int> ret(values.begin() + start, values.end());
        ret.append_range(std::vector<int>(values.begin(),values.begin() + start));
        return ret;
    }

    Reel(std::vector<int> values)
    {
        this->values = values;
    }
};




//template<typename BonusFeature, typename RNG>
class SlotMachine
{
    using column_t = std::vector<int>;
    std::vector<column_t> grid;

    std::vector<Reel> reels;

    using payline_t = std::vector<Square*>;
    std::vector<std::pair<bool, payline_t>> pay_lines;


    std::unordered_map<payline_t*, int> bets;

    int active_pay_lines;

    int max_bet;
    int bet_per_line;
    int total_bet_per_spin;
    //std::vector<BonusFeature> bonus_features;
    int coin_size;
    int fixed_jackpot;
    int free_spins;
    float multiplier;
    float payout_percentage;
    std::unordered_map<payline_t*, int> pay_table;
    int progressive_jackpot;
    //RNG random_number_generator;
    std::unordered_set<int> scatter_symbols;
    std::unordered_set<int> wild_symbols;
    int current_session_coins;

    int spin_number;

public:
    /*SlotMachine(std::vector<Reel> reels)
    {
        for(Reel reel : reels)
        {
            this->reels.push_back(reel);
            grid.push_back(reel.get_current());
        }
    };*/

    void add_reel(Reel reel)
    {
        this->reels.push_back(reel);
        this->grid.push_back(reel.get_current());
    }

    void add_pay_line(std::vector<Square*> pay_line)
    {
        this->pay_lines.push_back({false, pay_line});
    }

    void set_pay_line_activation(int pay_line_index, bool activation)
    {
        this->pay_lines[pay_line_index].first = activation;
    }

    void swith_pay_line_activation(int pay_line_index)
    {
        if(!this->pay_lines[pay_line_index].first)
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

    void bet_on_pay_line(payline_t* pay_line, int coins)
    {
        this->bets[pay_line] = coins;
    }

    void play_bonus_feature(int bonus_feature_index)
    {
        //this->bonus_features[bonus_feature_index].play();
    }

    void set_pay(payline_t* pay_line, int pay)
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

    void set_frame(int real_index)
    {
        this->grid[real_index] = reels[real_index].get_current();
    }

    void spin()
    {
        int i = 0;
        for(Reel& reel : this->reels)
        {
            reel.spin(rand());
            this->set_frame(i++);
        }
        spin_number++;
    }

    void print_grid()
    {
        for(column_t coloumn : this->grid)
        {
            for(int val : coloumn)
            {
                std::cout << "|" << val;
            }

            std::cout << "|" << std::endl;
        }
    }

};


int main()
{
    SlotMachine sm;
    
    
    sm.add_reel(std::vector<int>({1,2,3,4,5}));
    sm.add_reel(std::vector<int>({1,2,3,4,5}));
    sm.add_reel(std::vector<int>({1,2,3,4,5}));
    sm.add_reel(std::vector<int>({1,2,3,4,5}));
    sm.add_reel(std::vector<int>({1,2,3,4,5}));
    sm.spin();


    sm.print_grid();
}