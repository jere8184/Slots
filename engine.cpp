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
    int offset = 0;

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
        offset = amount % values.size();
    }

    std::vector<int> get_current()
    {
        std::vector<int> ret(values.begin() + offset, values.end());
        ret.append_range(std::vector<int>(values.begin(), values.begin() + offset));
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


class FrontEnd
{

private:
    const float m_reel_width = 100;
    const float m_symbol_height = 100;
    const float m_grid_bottom_boundary = 600;
    const float m_grid_top_boundary = 100;
    sf::RectangleShape m_top_border  = sf::RectangleShape({800, 100});
    sf::RectangleShape m_bottom_border  = sf::RectangleShape({800, 100});

    struct FrontEndReel
    {
        Reel& m_reel;
        bool m_can_spin = true;
        std::vector<sf::Sprite> m_sprites;

        FrontEndReel(Reel& reel, std::vector<sf::Sprite> sprites): m_reel(reel), m_sprites(sprites) {};

        void stop()
        {
            std::cout << (int)m_sprites.front().getPosition().y <<": " << (m_reel.offset * 100) + 100<< std::endl;
            if((int)m_sprites.front().getPosition().y == (m_reel.offset * 100) + 100)
            {
                m_can_spin = false;
            }
        }
    };

public:
    std::vector<sf::Texture> m_symbol_textures;

    std::vector<FrontEndReel> m_reels;


    sf::RenderWindow* m_window;

    FrontEnd(sf::RenderWindow* w): m_window(w)
    {
        this->m_top_border.setPosition(0, 0);
        this->m_top_border.setFillColor({173,255,47});
        this->m_bottom_border.setPosition(0, m_window->getSize().y - 100);
        this->m_bottom_border.setFillColor({173,255,47});
    };
    
    void add_symbol(std::string file_name)
    {
        std::filesystem::path p("resources/symbols/");
        p.append(file_name);
        sf::Texture t;
        
        if(!t.loadFromFile(file_name))
        {
            std::cout << "failed to load: "<< p.string() << std::endl;
            return;
        }
        m_symbol_textures.push_back(t);

        //s->setOrigin(sf::Vector2f(t.getSize()) / 2.f);
        //s->setPosition(s->getOrigin());

    }
    
    
    void add_reel(Reel* reel)
    {
        int i = 0;
        std::vector<sf::Sprite> sprites;
        for(int symbol : reel->values)
        {
            sf::Sprite sprite(m_symbol_textures[symbol]);
            sprite.setPosition({(m_reels.size() * m_reel_width) + 150, m_symbol_height * i++});
            sprites.push_back(sprite);
        }
        m_reels.push_back(FrontEndReel(*reel, sprites));
    }

    void render_reel(int reel_index)
    {
        int i = 0;
        for(auto& sprite : m_reels[reel_index].m_sprites)
        {
            m_window->draw(sprite);
        }
    }

    void spin_reel(std::vector<sf::Sprite>& reel_sprites ,float amount)
    {
        for(sf::Sprite& sprite : reel_sprites)
        {
            if(sprite.getPosition().y > reel_sprites.size() * 100)
            {
                sprite.setPosition(sprite.getPosition().x, 0);
            }
            sprite.move({0,amount});
        }
    }


    void spin(float amount)
    {
        for(auto& reel : this->m_reels)
        {
            if(reel.m_can_spin)
            {                
                reel.stop();
                spin_reel(reel.m_sprites, amount);
            }
        }
    }

    void render_boardrs()
    {
        this->m_window->draw(this->m_top_border);
        this->m_window->draw(this->m_bottom_border);
    }
};




int main()
{
    sf::RenderWindow window(sf::VideoMode(800, 600), "SFML window");
    FrontEnd r(&window);
    r.add_symbol("symbol_7.png");
    r.add_symbol("symbol_bell.png");
    r.add_symbol("symbol_melon.png");
    r.add_symbol("symbol_cherry.png");


    SlotMachine sm;
    FairStandardReel reel_1(std::vector<int>({0, 1, 2, 3, 3, 3, 3}));
    FairStandardReel reel_2(std::vector<int>({1, 2, 3, 0, 1, 2, 1}));
    FairStandardReel reel_3(std::vector<int>({3, 2, 0, 1, 3, 1, 1}));
    FairStandardReel reel_4(std::vector<int>({2, 0, 3, 0, 0, 0, 1}));
    FairStandardReel reel_5(std::vector<int>({1, 2, 0, 1, 0, 2, 1}));


    r.add_reel(&reel_1);
    r.add_reel(&reel_2);
    r.add_reel(&reel_3);
    r.add_reel(&reel_4);
    r.add_reel(&reel_5);

    reel_1.spin(rand());
    reel_2.spin(rand());
    reel_3.spin(rand());
    reel_4.spin(rand());
    reel_5.spin(rand());

    reel_1.offset;

    while (true)
    {
        r.render_reel(0);
        r.render_reel(1);
        r.render_reel(2);
        r.render_reel(3);
        r.render_reel(4);
        r.spin(1);

        r.render_boardrs();
        window.display();
        window.clear({240,230,140});
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