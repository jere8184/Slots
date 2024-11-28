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
        std::vector<int> ret(values.end() - offset, values.end());
        ret.append_range(std::vector<int>(values.begin(), values.end() - offset));
        return ret;
    }

    FairStandardReel(std::vector<int> values) : Reel(values) {};
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
        int m_loops = 10;

        FrontEndReel(Reel& reel, std::vector<sf::Sprite> sprites, int loops): m_reel(reel), m_sprites(sprites), m_loops(loops)  {};

        void stop()
        {
            std::cout << (int)m_sprites.front().getPosition().y <<": " << (m_reel.offset * 100) << std::endl;
            if((int)m_sprites.front().getPosition().y == (m_reel.offset * 100))
            {
                if(m_loops)
                    m_loops--;
                else
                {
                    m_can_spin = false;
                }
            }
        }
    };

public:
    std::vector<sf::Texture> m_symbol_textures;
    std::vector<FrontEndReel> m_reels;
    sf::RenderWindow* m_window;

    float m_spin_speed = 1.0;


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
        m_reels.push_back(FrontEndReel(*reel, sprites, rand()%5));
    }

    void render_reel(FrontEndReel& reel)
    {
        int i = 0;
        for(auto& sprite : reel.m_sprites)
        {
            m_window->draw(sprite);
        }
    }

    using payline_t = std::vector<Square>;

    void render_pay_line(payline_t pay_line)
    {
        sf::CircleShape circle(100);
        circle.setFillColor(sf::Color::Transparent);
        circle.setOutlineThickness(20);

        for(Square square : pay_line)
        {
            circle.setPosition(square.x * 100, square.y * 100);
            m_window->draw(circle);
        }
        m_window->display();
    }

    void spin_reel(std::vector<sf::Sprite>& reel_sprites ,float amount)
    {
        for(sf::Sprite& sprite : reel_sprites)
        {
            if(sprite.getPosition().y > reel_sprites.size() * 100)
            {
                sprite.setPosition(sprite.getPosition().x, 0);
            }
            else
            {
                sprite.move({0,amount});
            }
        }
    }


    bool spin(float amount)
    {
        bool ret = false;
        for(auto& reel : this->m_reels)
        {
            if(reel.m_can_spin)
            {    
                ret = true;            
                reel.stop();
                spin_reel(reel.m_sprites, amount);
            }
        }
        return ret;
    }

    void render_boardrs()
    {
        this->m_window->draw(this->m_top_border);
        this->m_window->draw(this->m_bottom_border);
    }


    void display()
    {
        while(this->spin(m_spin_speed))
        {
            for(FrontEndReel reel: m_reels)
            {
                this->render_reel(reel);
            }
            
            this->render_boardrs();
            m_window->display();
            m_window->clear({240,230,140});
        }
        for(FrontEndReel& reel: m_reels)
        {
            reel.m_can_spin = true;
            reel.m_loops = rand() % 10;
        }
        //sf::sleep(sf::milliseconds(3000));
    }
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
    FrontEnd* m_front_end = nullptr;

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

    FrontEnd* get_front_end()
    {
        return m_front_end;
    }

    void set_front_end(FrontEnd* front_end)
    {
        m_front_end = front_end;
    }

    void add_reel(Reel *reel)
    {
        this->m_reels.push_back(reel);
        this->m_grid.push_back(reel->get_current());

        if(this->m_front_end)
        {
            this->m_front_end->add_reel(reel);
        }
    }

    void add_pay_line(std::vector<Square> pay_line, bool is_active = true)
    {
        this->m_pay_lines.push_back({is_active, pay_line});
    }

    void add_generic_pay_line(std::vector<Square> pay_line, bool is_active = true)
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
                if (this->m_grid[s.x][s.y] == symbol)
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
        this->print_grid();


        for (Reel *reel : this->m_reels)
        {
            reel->spin(rand());
            this->set_frame(i++);
        }

        if(this->m_front_end)
        {
            this->m_front_end->display();
            this->print_grid();
        }

        m_spin_number++;
        if (this->find_wins().empty())
        {
            return false;
        }
        else
        {
            
            //this->m_front_end->display();
            //display win liness
            this->print_grid();
            //this->m_front_end->render_pay_line(this->find_wins().front());
            //sf::sleep(sf::milliseconds(10000));
            return true;
        }
    }

    void print_grid() const
    {
        std::cout << "spin count: " << m_spin_number << std::endl;
        for(int y = 0; y < m_grid[0].size(); y++)
        {
            for(int x = 0; x < m_grid.size(); x++)
            {
                std::cout << m_grid[x][y] << "|";
            }
            std::cout << std::endl;
        }
        std::cout << std::endl;
        std::cout << std::endl;
    }
};


int main()
{
    sf::RenderWindow window(sf::VideoMode(800, 600), "SFML window");
    FrontEnd r(&window);
    r.m_spin_speed = 10;
    r.add_symbol("symbol_7.png");
    r.add_symbol("symbol_bell.png");
    r.add_symbol("symbol_melon.png");
    r.add_symbol("symbol_cherry.png");

    SlotMachine sm;
    sm.set_front_end(&r);

    FairStandardReel reel_1(std::vector<int>({0, 1, 2, 3, 3,3,2,2,2,2}));
    FairStandardReel reel_2(std::vector<int>({0, 1, 2, 3, 1}));
    FairStandardReel reel_3(std::vector<int>({0, 1, 2, 3, 3}));
    FairStandardReel reel_4(std::vector<int>({0, 1, 2, 3, 0}));
    FairStandardReel reel_5(std::vector<int>({0, 1, 2, 3, 0}));

    sm.add_reel(&reel_1);
    sm.add_reel(&reel_2);
    sm.add_reel(&reel_3);
    sm.add_reel(&reel_4);
    sm.add_reel(&reel_5);

    sm.add_pay_line({{0, 2, {0,1}}, {1, 2, {0,1}}, {2, 2, {0,1}}, {3, 2, {0,1}}, {4, 2, {0,1}}});
    sm.add_pay_line({{0, 3, {0,1}}, {1, 3, {0,1}}, {2, 3, {0,1}}, {3, 3, {0,1}}, {4, 3, {0,1}}});
    sm.add_pay_line({{0, 4, {0,1}}, {1, 4, {0,1}}, {2, 4, {0,1}}, {3, 4, {0,1}}, {4, 4, {0,1}}});

    while (!sm.spin())

    sm.print_grid();
    std::cin.get();
}