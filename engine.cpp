#include <vector>
#include <utility>
#include <unordered_set>
#include <unordered_map>
#include <set>
#include <deque>
#include <iostream>
#include <filesystem>
#include <cmath>


#include "SFML/Window.hpp"
#include "SFML/Graphics.hpp"

#pragma comment(lib,"user32.lib")  


struct Square
{
    int x;
    int y;
    std::vector<int> valid_symbol;
};

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
private:
    using column_t = std::vector<int>;
    std::vector<column_t> m_grid;

    std::vector<Reel *> m_reels;

    using payline_t = std::vector<Square>;
    std::vector<std::pair<bool, payline_t>> m_pay_lines;

    std::unordered_map<payline_t *, int> m_bets;

    int m_active_pay_lines;

    int m_max_bet;
    int m_bet_per_line;
    int m_total_bet_per_spin;
    // std::vector<BonusFeature> bonus_features;
    int m_coin_size;
    std::unordered_map<int, int> m_fixed_jackpots;
    int m_free_spins;
    float m_payout_percentage;
    std::unordered_map<payline_t *, int> m_pay_table;
    std::unordered_map<int, int> m_progressive_jackpots;
    // RNG random_number_generator;
    std::unordered_set<int> m_scatter_symbols;
    std::unordered_set<int> m_wild_symbols;
    int m_current_session_coins;

    int m_spin_number = 0;

public:
    SlotMachine() = default;
    SlotMachine(std::vector<Reel *> reels)
    {
        for (Reel *reel : reels)
        {
            this->m_reels.push_back(reel);
            m_grid.push_back(reel->get_current());
        }
    };

    void add_reel(Reel *reel)
    {
        this->m_reels.push_back(reel);
        this->m_grid.push_back(reel->get_current());
    }

    void add_pay_line(std::vector<Square> pay_line, bool is_active = true)
    {
        this->m_pay_lines.push_back({is_active, pay_line});
    }

    void set_pay_line_activation(int pay_line_index, bool activation)
    {
        this->m_pay_lines[pay_line_index].first = activation;
    }

    void swith_pay_line_activation(int pay_line_index)
    {
        if (!this->m_pay_lines[pay_line_index].first)
        {
            this->m_pay_lines[pay_line_index].first = false;
        }
        else
        {
            this->m_pay_lines[pay_line_index].first = true;
        }
    }

    void set_max_bet(int max_bet)
    {
        this->m_max_bet = max_bet;
    }

    void bet_on_pay_line(payline_t *pay_line, int coins)
    {
        this->m_bets[pay_line] = coins;
    }

    void play_bonus_feature(int bonus_feature_index)
    {
        // this->bonus_features[bonus_feature_index].play();
    }

    void set_pay(payline_t *pay_line, int pay)
    {
        this->m_pay_table[pay_line] = pay;
    }
    void add_scatter_symbol(int symbol)
    {
        this->m_scatter_symbols.insert(symbol);
    }
    void add_wild_symbol(int symbol)
    {
        this->m_wild_symbols.insert(symbol);
    }

    void add_fixed_jackpot(int jackpot, int coins)
    {
        this->m_fixed_jackpots.insert({jackpot, coins});
    }

    void adjust_fixed_jackpot(int jackpot, int coins)
    {
        if (this->m_fixed_jackpots.contains(jackpot))
        {
            this->m_fixed_jackpots[jackpot] += coins;
        }
    }

    void multiply_fixed_jackpot(int jackpot, float modifier)
    {
        if (this->m_fixed_jackpots.contains(jackpot))
        {
            this->m_fixed_jackpots[jackpot] *= modifier;
        }
    }

    void add_progressive_jackpot(int jackpot, int coins)
    {
        this->m_progressive_jackpots.insert({jackpot, coins});
    }

    void adjust_progressive_jackpot(int jackpot, int coins)
    {
        if (this->m_progressive_jackpots.contains(jackpot))
        {
            this->m_progressive_jackpots[jackpot] += coins;
        }
    }

    void multiply_progressive_jackpot(int jackpot, float modifier)
    {
        if (this->m_progressive_jackpots.contains(jackpot))
        {
            this->m_progressive_jackpots[jackpot] *= modifier;
        }
    }

    void set_frame(int real_index)
    {
        this->m_grid[real_index] = m_reels[real_index]->get_current();
    }

    bool payline_satisfied(payline_t payline)
    {
        for (Square s : payline)
        {
            bool symbol_found = false;
            for (int symbol : s.valid_symbol)
            {
                if (this->m_grid[s.y][s.x] == symbol)
                    symbol_found = true;
            }
            if (!symbol_found)
                return false;
        }
        return true;
    }

    std::vector<payline_t> find_wins()
    {
        std::vector<payline_t> pay_lines;
        for (const auto &[is_active, pay_line] : this->m_pay_lines)
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
        for (Reel *reel : this->m_reels)
        {
            reel->spin(rand());
            this->set_frame(i++);
        }

        m_spin_number++;
        if (find_wins().empty())
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
        std::cout << "spin count: " << m_spin_number << std::endl;
        for (column_t coloumn : this->m_grid)
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


class Renderer
{

private:
    const float m_reel_width = 100;
    const float m_symbol_height = 100;

public:
    std::unordered_map<int, sf::Sprite*> m_symbol_sprites;
    sf::RenderWindow* window;

    Renderer(sf::RenderWindow* w): window(w){};
    
    void add_symbol(int symbol_val, std::string file_name)
    {
        std::filesystem::path p("resources/symbols/");
        p.append(file_name);
        sf::Texture t;
        
        if(!t.loadFromFile(file_name))
        {
            std::cout << "failed to load: "<< p.string() << std::endl;
            return;
        }

        sf::Sprite* s = new sf::Sprite(t);
        //s->setOrigin(sf::Vector2f(t.getSize()) / 2.f);
        //s->setPosition(s->getOrigin());

        m_symbol_sprites[symbol_val] = s;
    }
    
    
    void render_reel(Reel* reel, int position)
    {
        int i = 0;
        for(int symbol : reel->values)
        {
            if(m_symbol_sprites.contains(symbol))
            {
                sf::Sprite* sprite = m_symbol_sprites[symbol];
                sprite->setPosition({300, 300});
                window->draw(*sprite);
                break;
            }
        }
    }
};




int main()
{
    sf::RenderWindow window(sf::VideoMode(800, 600), "SFML window");
    Renderer r(&window);
    r.add_symbol(1,"symbol_7.png");
    r.add_symbol(2,"symbol_bell.png");
    r.add_symbol(3,"symbol_melon.png");

    SlotMachine sm;
    FairStandardReel reel_1(std::vector<int>({2, 2, 2, 2, 2}));
    FairStandardReel reel_2(std::vector<int>({1, 2, 3, 4, 5}));
    FairStandardReel reel_3(std::vector<int>({1, 2, 3, 4, 5}));
    FairStandardReel reel_4(std::vector<int>({1, 2, 3, 4, 5}));
    FairStandardReel reel_5(std::vector<int>({1, 2, 3, 4, 5}));


    while (true)
    {
        r.render_reel(&reel_1, 0);
        window.display();
    }

    sm.add_reel(&reel_1);
    sm.add_reel(&reel_2);
    sm.add_reel(&reel_3);
    sm.add_reel(&reel_4);
    sm.add_reel(&reel_5);
    sm.add_pay_line({{2, 0, {4, 5}}, {2, 1, {4, 5}}, {2, 2, {4, 5}}, {2, 3, {4, 5}}, {2, 4, {4, 5}}});

    while (!sm.spin())
        sm.print_grid();

    sm.print_grid();
}