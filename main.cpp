#include <iostream>
#include <string>
#include <vector>
#include <stdexcept>
#include <algorithm>
#include <utility>
#include <limits>
#include <iomanip>
#include <cstdlib>
#include <ctime>

using namespace std;

class Date {
    int day, month, year;
    bool isLeapYear(int y) const {
        return (y % 4 == 0 && y % 100 != 0) || (y % 400 == 0);
    }
    int getDaysInMonth(int m, int y) const {
        switch(m) {
            case 1: case 3: case 5: case 7: case 8: case 10: case 12: return 31;
            case 4: case 6: case 9: case 11: return 30;
            case 2: return isLeapYear(y) ? 29 : 28;
            default: throw invalid_argument("Некорректный месяц!");
        }
    }
    long toDays() const {
        long days = day;
        for (int i = 1; i < month; i++) days += getDaysInMonth(i, year);
        for (int i = 1900; i < year; i++) days += isLeapYear(i) ? 366 : 365;
        return days;
    }
public:
    Date(int d = 1, int m = 1, int y = 2024) {
        if (y < 0) throw invalid_argument("Некорректный год!");
        if (m < 1 || m > 12) throw invalid_argument("Месяц должен быть от 1 до 12!");
        if (d < 1 || d > getDaysInMonth(m, y)) throw invalid_argument("Некорректный день!");
        day = d; month = m; year = y;
    }
    Date operator++() {
        day++;
        if (day > getDaysInMonth(month, year)) {
            day = 1;
            if (month == 12) { month = 1; year++; }
            else month++;
        }
        return *this;
    }
    Date operator++(int) {
        Date old = *this;
        ++(*this);
        return old;
    }
    Date addMonths(int months) const {
        Date result = *this;
        for (int i = 0; i < months; i++) {
            result.month++;
            if (result.month > 12) {
                result.month = 1;
                result.year++;
            }
            int maxDay = result.getDaysInMonth(result.month, result.year);
            if (result.day > maxDay) result.day = maxDay;
        }
        return result;
    }
    long operator-(const Date& other) const {
        return toDays() - other.toDays();
    }
    bool operator<(const Date& other) const {
        if (year != other.year) return year < other.year;
        if (month != other.month) return month < other.month;
        return day < other.day;
    }
    bool operator==(const Date& other) const {
        return day == other.day && month == other.month && year == other.year;
    }
    friend ostream& operator<<(ostream& os, const Date& d) {
        if (d.day < 10) os << "0";
        os << d.day << ".";
        if (d.month < 10) os << "0";
        os << d.month << ".";
        os << d.year;
        return os;
    }
};

class Person {
protected:
    int id;
    string name;
    int age;
public:
    Person(int i, string n, int a) : id(i), name(n), age(a) {
        if (i <= 0) throw invalid_argument("Некорректное id!");
        if (age <= 14) throw invalid_argument("Возраст должен быть больше 14 лет!");
    }
    virtual string getInfo() const = 0;
    virtual ~Person() {}
    string getName() const { return name; }
    int getAge() const { return age; }
    int getId() const { return id; }
};

class Player;

class Agent : public Person {
    double commission;
public:
    Agent(int i, string n, int a, double c) : Person(i, n, a), commission(c) {}
    string getInfo() const override {
        return "Агент: " + name + ", возраст: " + to_string(age) + ", комиссия: " + to_string(commission) + "%";
    }
    void negotiateContract(Player* p);
    double getCommission() const { return commission; }
};

class Player : public Person {
    string position;
    double value;
    Agent* agent;
    static int totalPlayers;
public:
    Player(int i, string n, int a, string p, double v, Agent* ag = nullptr)
        : Person(i, n, a), position(p), value(v), agent(ag) {
        totalPlayers++;
    }
    Player(const Player& other)
        : Person(other.id, other.name, other.age), position(other.position),
          value(other.value), agent(other.agent) {
        totalPlayers++;
    }
    Player(Player&& other) noexcept
        : Person(other.id, other.name, other.age), position(std::move(other.position)),
          value(other.value), agent(other.agent) {
        totalPlayers++;
    }
    ~Player() { totalPlayers--; }
    string getInfo() const override {
        string info = name + " | " + to_string(age) + " лет | " + position + " | " + to_string(value) + " млн €";
        if (age < 24) info += " | 📈 перспективный";
        else if (age > 30) info += " | 📉 возрастной";
        return info;
    }
    void updateMarketValue(double v) {
        if (v < 0) throw invalid_argument("Стоимость не может быть отрицательной!");
        value = v;
    }
    Player operator-() {
        value *= 0.9;
        return *this;
    }
    double getValue() const { return value; }
    string getPosition() const { return position; }
    Agent* getAgent() const { return agent; }
    void setAgent(Agent* a) { agent = a; }
    static int getTotalPlayers() { return totalPlayers; }
};

int Player::totalPlayers = 0;

void Agent::negotiateContract(Player* p) {
    cout << "Агент " << name << " ведёт переговоры по контракту для " << p->getName() << endl;
}

bool operator>(const Player& a, const Player& b) {
    return a.getValue() > b.getValue();
}

class Club {
    int clubId;
    string name;
    double budget;
    vector<Player*> squad;
    static int totalClubs;
public:
    Club(int i, string n, double d) : clubId(i), name(n), budget(d) {
        totalClubs++;
    }
    ~Club() { totalClubs--; }
    void addPlayer(Player* p) {
        squad.push_back(p);
        cout << "   ✅ " << p->getName() << " присоединился к " << name << endl;
    }
    void removePlayer(Player* p) {
        auto it = find(squad.begin(), squad.end(), p);
        if (it != squad.end()) squad.erase(it);
    }
    bool canAfford(double amount) const { return amount <= budget; }
    void spend(double amount) {
        if (amount > budget) throw invalid_argument("Недостаточно средств!");
        budget -= amount;
        cout << "   💸 " << name << " потратил " << amount << " млн €. Остаток: " << budget << " млн €" << endl;
    }
    void receive(double amount) {
        budget += amount;
        cout << "   💰 " << name << " получил " << amount << " млн €. Бюджет: " << budget << " млн €" << endl;
    }
    string getName() const { return name; }
    double getBudget() const { return budget; }
    vector<Player*>& getSquad() { return squad; }
    void showSquad() const {
        if (squad.empty()) {
            cout << "\n   📭 Состав пуст. Купите игроков!\n";
            return;
        }
        cout << "\n   📋 СОСТАВ " << name << ":\n";
        for (auto p : squad) cout << "       " << p->getInfo() << endl;
    }
    static int getTotalClubs() { return totalClubs; }
};

int Club::totalClubs = 0;

class LoanInfo {
public:
    Player* player;
    Date startDate;
    Date endDate;
    Club* fromClub;
    int durationMonths;
    LoanInfo(Player* p, Date start, int months, Club* from)
        : player(p), startDate(start), fromClub(from), durationMonths(months) {
        endDate = start.addMonths(months);
    }
    bool isExpired(Date current) const { return endDate < current; }
    string getDurationString() const { return to_string(durationMonths) + " мес."; }
    bool isPlayerOnLoan(Player* p) const { return player == p; }
};

class TransferWindow {
    Date startDate;
    Date endDate;
public:
    TransferWindow(Date s, Date e) : startDate(s), endDate(e) {}
    bool isOpen(Date d) const {
        return !(d < startDate) && !(endDate < d);
    }
    void showStatus(Date d) const {
        if (isOpen(d)) cout << "   🔓 Трансферное окно ОТКРЫТО (до " << endDate << ")\n";
        else           cout << "   🔒 Трансферное окно ЗАКРЫТО\n";
    }
};

// класс для ввода/вывода (не связан никак с бизнес-логикой)
class UI {
public:
    template<typename T>
    static T safeInput(const string& prompt, T minVal, T maxVal) {
        T value;
        while (true) {
            cout << prompt;
            cin >> value;
            if (cin.fail()) {
                cin.clear();
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                cout << "   ❌ Ошибка! Введите число.\n";
            } else if (value < minVal || value > maxVal) {
                cout << "   ❌ Ошибка! Введите число от " << minVal << " до " << maxVal << ".\n";
            } else {
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                return value;
            }
        }
    }
    static bool safeConfirm(const string& prompt) {
        char answer;
        cout << prompt;
        cin >> answer;
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        return (answer == 'y' || answer == 'Y');
    }
    static void showWelcome() {
        cout << "\n   ╔════════════════════════════════════════════════════════════════════╗\n";
        cout << "   ║                  ДОБРО ПОЖАЛОВАТЬ В ИГРУ!                          ║\n";
        cout << "   ║                                                                    ║\n";
        cout << "   ║   Вы - спортивный директор футбольного клуба.                      ║\n";
        cout << "   ║   Ваша задача - заработать деньги на трансферах и усилить состав.  ║\n";
        cout << "   ║                                                                    ║\n";
        cout << "   ║   💡 СТРАТЕГИЯ ДЛЯ УСПЕХА:                                         ║\n";
        cout << "   ║      • Покупайте молодых игроков (до 24 лет) - они дорожают        ║\n";
        cout << "   ║      • Продавайте игроков, когда их цена выросла                   ║\n";
        cout << "   ║      • Получайте ежедневный доход от спонсоров                     ║\n";
        cout << "   ║      • Следите за предложениями от других клубов                   ║\n";
        cout << "   ║      • Используйте аренду для временного усиления                  ║\n";
        cout << "   ║      • Игроки в аренде НЕ МОГУТ быть проданы                       ║\n";
        cout << "   ╚════════════════════════════════════════════════════════════════════╝\n";
    }

    static void showMenu(const Date& date, const TransferWindow* window,
                         const Club* userClub, const vector<LoanInfo>& activeLoans) {
        cout << "\n═══════════════════════════════════════════════════════════════════════════════════════════════════════════════════\n\n";
        cout << "   📅 Дата: " << date << endl;
        window->showStatus(date);
        cout << "   💰 " << userClub->getName() << ", бюджет: " << userClub->getBudget() << " млн €\n";

        if (!activeLoans.empty()) {
            cout << "\n   📋 АКТИВНЫЕ АРЕНДЫ:\n";
            for (size_t i = 0; i < activeLoans.size(); i++) {
                cout << "      " << i+1 << ". " << activeLoans[i].player->getName()
                     << " (до " << activeLoans[i].endDate << ")\n";
            }
        }

        cout << "\n   ┌────────────────────────────────────────────────────────────────┐\n";
        cout << "   │ 1. 📋 Показать состав                                          │\n";
        cout << "   │ 2. 💰 Купить игрока                                            │\n";
        cout << "   │ 3. 🤝 Взять игрока в аренду                                    │\n";
        cout << "   │ 4. 💸 Продать игрока                                           │\n";
        cout << "   │ 5. ⏩ Следующий день                                           │\n";
        cout << "   │ 6. 📊 Статистика                                               │\n";
        cout << "   │ 7. 🚪 Выход                                                    │\n";
        cout << "   └────────────────────────────────────────────────────────────────┘\n";
    }

    static void showAvailablePlayers(const vector<Player*>& freeAgents) {
        if (freeAgents.empty()) {
            cout << "\n   ❌ Нет доступных игроков!\n";
            return;
        }
        cout << "\n   🌟 ДОСТУПНЫЕ ИГРОКИ:\n";
        int idx = 1;
        for (auto p : freeAgents) cout << "     " << idx++ << ") " << p->getInfo() << endl;
    }

    static void showStats(int daysPlayed, const Club* userClub,
                          double totalEarned, double totalSpent,
                          const vector<Player*>& ownPlayers,
                          const vector<LoanInfo>& activeLoans) {
        double netProfit = totalEarned - totalSpent;
        cout << "\n\n   📅 Дней в игре: " << daysPlayed << endl;
        cout << "   💰 Текущий бюджет: " << userClub->getBudget() << " млн €\n";
        cout << "   📈 Всего заработано: " << totalEarned << " млн €\n";
        cout << "   📉 Всего потрачено: " << totalSpent << " млн €\n";
        if (netProfit >= 0) cout << "   ✅ Чистая прибыль: +" << netProfit << " млн €" << endl;
        else                cout << "   ❌ Чистый убыток: " << netProfit << " млн €" << endl;
        double squadValue = 0;
        for (auto p : ownPlayers) squadValue += p->getValue();
        cout << "   💎 Стоимость состава (собственность): " << squadValue << " млн €\n";
        if (!activeLoans.empty()) {
            cout << "\n   📋 Игроки в аренде:\n";
            for (auto& loan : activeLoans) {
                cout << "      • " << loan.player->getName() << " (до " << loan.endDate << ")" << endl;
            }
            cout << "      ⚠️ Игроки в аренде не могут быть проданы.\n";
        }
    }

    static int selectLoanDuration() {
        cout << "\n   📅 ВЫБЕРИТЕ СРОК АРЕНДЫ:\n";
        cout << "      1. 1 месяц   (10% от стоимости)\n";
        cout << "      2. 3 месяца  (30% от стоимости)\n";
        cout << "      3. 6 месяцев (50% от стоимости)\n";
        cout << "      4. 12 месяцев (70% от стоимости)\n";
        int choice = safeInput<int>("      Выберите (1-4): ", 1, 4);
        switch(choice) {
            case 1: return 1;
            case 2: return 3;
            case 3: return 6;
            case 4: return 12;
            default: return 6;
        }
    }
};

// случайные предложения, колебания цен
class Market {
public:
    // случайные колебания рыночной стоимости всех игроков
    static void randomMarketChange(vector<Player*>& freeAgents,
                                   const vector<Player*>& ownPlayers) {
        for (auto p : freeAgents) {
            double change = (rand() % 21 - 10) / 100.0;
            double newValue = p->getValue() * (1 + change);
            if (newValue > 0.1) p->updateMarketValue(newValue);
        }
        for (auto p : ownPlayers) {
            double change = (rand() % 21 - 10) / 100.0;
            double newValue = p->getValue() * (1 + change);
            if (newValue > 0.1) {
                double oldValue = p->getValue();
                p->updateMarketValue(newValue);
                if (change > 0.05)
                    cout << "   📈 " << p->getName() << " подорожал на " << (newValue - oldValue) << " млн €" << endl;
                else if (change < -0.05)
                    cout << "   📉 " << p->getName() << " подешевел на " << (oldValue - newValue) << " млн €" << endl;
            }
        }
    }

    // ежедневный доход от спонсоров
    static void addDailyIncome(Club* userClub, double& totalEarned) {
        const double dailyIncome = 0.5;
        userClub->receive(dailyIncome);
        totalEarned += dailyIncome;
        cout << "   📈 Ежедневный доход (спонсоры): +" << dailyIncome << " млн €" << endl;
    }

    // вычисление стоимости аренды
    static double calculateLoanFee(double playerValue, int months) {
        switch(months) {
            case 12: return playerValue * 0.85;
            case 6:  return playerValue * 0.5;
            case 3:  return playerValue * 0.3;
            case 1:  return playerValue * 0.1;
            default: return playerValue * 0.5;
        }
    }
};

// логика трансферных операций
class TransferManager {
    Club* userClub;
    vector<Club*>& clubs;
    vector<Player*>& freeAgents;
    vector<LoanInfo>& activeLoans;
    double& totalSpent;
    double& totalEarned;

public:
    TransferManager(Club* uc, vector<Club*>& cl,
                    vector<Player*>& fa, vector<LoanInfo>& al,
                    double& spent, double& earned)
        : userClub(uc), clubs(cl), freeAgents(fa), activeLoans(al),
          totalSpent(spent), totalEarned(earned) {}

    bool isPlayerOnLoan(Player* p) const {
        for (auto& loan : activeLoans)
            if (loan.player == p) return true;
        return false;
    }

    vector<Player*> getOwnPlayers() const {
        vector<Player*> ownPlayers;
        for (auto p : userClub->getSquad())
            if (!isPlayerOnLoan(p)) ownPlayers.push_back(p);
        return ownPlayers;
    }

    void buyPlayer() {
        if (freeAgents.empty()) {
            cout << "\n   ❌ Нет доступных игроков!\n";
            return;
        }
        UI::showAvailablePlayers(freeAgents);
        int choice = UI::safeInput<int>("\n   Введите номер игрока: ", 1, (int)freeAgents.size());
        Player* p = freeAgents[choice - 1];
        double price = p->getValue();
        cout << "\n   📊 ИНФОРМАЦИЯ О ПОКУПКЕ:\n";
        cout << "      Игрок: " << p->getName() << endl;
        cout << "      Возраст: " << p->getAge() << " лет" << endl;
        cout << "      Цена: " << price << " млн €" << endl;
        if (!userClub->canAfford(price)) {
            cout << "\n   ❌ Недостаточно средств! Не хватает "
                 << (price - userClub->getBudget()) << " млн €\n";
            return;
        }
        if (!UI::safeConfirm("\n   Подтвердить покупку? (y/n): ")) {
            cout << "   ❌ Покупка отменена\n";
            return;
        }
        userClub->addPlayer(p);
        userClub->spend(price);
        totalSpent += price;
        freeAgents.erase(freeAgents.begin() + choice - 1);
        cout << "\n   ✅ ПОКУПКА ЗАВЕРШЕНА!" << endl;
        cout << "      💡 Совет: Следите за рыночной стоимостью. Через несколько дней игрок может подорожать!\n";
    }

    void sellPlayer() {
        vector<Player*> ownPlayers = getOwnPlayers();
        if (ownPlayers.empty()) {
            cout << "\n   ❌ Нет собственных игроков для продажи!\n";
            if (!activeLoans.empty())
                cout << "      (Игроки в аренде не могут быть проданы. Дождитесь окончания аренды.)\n";
            return;
        }
        cout << "\n   📋 ВАШИ ИГРОКИ (собственность):\n";
        for (size_t i = 0; i < ownPlayers.size(); i++)
            cout << "    " << i+1 << ") " << ownPlayers[i]->getInfo() << endl;
        int choice = UI::safeInput<int>("\n   Введите номер игрока: ", 1, (int)ownPlayers.size());
        Player* p = ownPlayers[choice - 1];
        if (isPlayerOnLoan(p)) {
            cout << "\n   ❌ Этот игрок сейчас в аренде и не может быть продан!\n";
            return;
        }
        double marketValue = p->getValue();
        double minPrice = marketValue * 0.7;
        double maxPrice = marketValue * 1.3;
        cout << "\n   📊 АНАЛИЗ РЫНКА:\n";
        printf("      Рыночная стоимость: %.2f млн €\n", marketValue);
        printf("      Рекомендуемая цена: %.2f - %.2f млн €\n", minPrice, maxPrice);
        double askPrice = UI::safeInput<double>("      Ваша цена (млн €): ", minPrice, maxPrice);
        int successChance;
        if (askPrice <= marketValue)             successChance = 85;
        else if (askPrice <= marketValue * 1.1)  successChance = 65;
        else if (askPrice <= marketValue * 1.2)  successChance = 45;
        else                                     successChance = 25;

        cout << "\n   🔍 Ищем покупателя..." << endl;

        if (rand() % 100 < successChance) {
            userClub->receive(askPrice);
            userClub->removePlayer(p);
            freeAgents.push_back(p);
            totalEarned += askPrice;
            double profit = askPrice - marketValue;
            if (profit > 0)      cout << "\n   ✅ ПРОДАНО! Прибыль: +" << profit << " млн €" << endl;
            else if (profit < 0) cout << "\n   ✅ ПРОДАНО! Убыток: " << profit << " млн €" << endl;
            else                 cout << "\n   ✅ ПРОДАНО! Без прибыли и убытка." << endl;
        } else {
            cout << "\n   ❌ Покупатель не согласился на цену. Попробуйте снизить цену." << endl;
        }
    }

    void loanPlayer(const Date& currentDate) {
        if (freeAgents.empty()) {
            cout << "\n   ❌ Нет доступных игроков!\n";
            return;
        }
        UI::showAvailablePlayers(freeAgents);
        int choice = UI::safeInput<int>("\n   Введите номер игрока для аренды: ", 1, (int)freeAgents.size());
        Player* p = freeAgents[choice - 1];
        int months = UI::selectLoanDuration();
        double loanFee = Market::calculateLoanFee(p->getValue(), months);
        Date endDate = currentDate.addMonths(months);
        cout << "\n   📋 ИНФОРМАЦИЯ ОБ АРЕНДЕ:\n";
        cout << "      Игрок: " << p->getName() << endl;
        printf("      Стоимость игрока: %.2f млн €\n", p->getValue());
        cout << "      Срок аренды: " << months << " месяц(ев)\n";
        printf("      Стоимость аренды: %.2f млн €\n", loanFee);
        cout << "      Дата окончания: " << endDate << endl;
        if (!userClub->canAfford(loanFee)) {
            printf("\n   ❌ Недостаточно средств! Нужно: %.2f млн €, доступно: %.2f млн €\n",
                   loanFee, userClub->getBudget());
            return;
        }
        if (!UI::safeConfirm("\n   Подтвердить аренду? (y/n): ")) {
            cout << "   ❌ Аренда отменена\n";
            return;
        }
        userClub->addPlayer(p);
        userClub->spend(loanFee);
        totalSpent += loanFee;
        freeAgents.erase(freeAgents.begin() + choice - 1);
        activeLoans.push_back(LoanInfo(p, currentDate, months, nullptr));
        cout << "\n   ✅ АРЕНДА ОФОРМЛЕНА!" << endl;
        cout << "      🎮 " << p->getName() << " будет играть за " << userClub->getName()
             << " до " << endDate << endl;
        cout << "      ⚠️ Игрок не может быть продан, пока находится в аренде." << endl;
    }

    void processExpiredLoans(const Date& currentDate) {
        vector<LoanInfo> remainingLoans;
        for (auto& loan : activeLoans) {
            if (loan.isExpired(currentDate)) {
                cout << "\n   ⚠️ АРЕНДА ЗАКОНЧИЛАСЬ: " << loan.player->getName()
                     << " покидает " << userClub->getName() << endl;
                userClub->removePlayer(loan.player);
                if (loan.fromClub) {
                    loan.fromClub->addPlayer(loan.player);
                    cout << "   ✅ " << loan.player->getName() << " вернулся в " << loan.fromClub->getName() << endl;
                } else {
                    freeAgents.push_back(loan.player);
                    cout << "   ✅ " << loan.player->getName() << " стал свободным агентом" << endl;
                }
            } else {
                remainingLoans.push_back(loan);
            }
        }
        activeLoans = remainingLoans;
    }

    void randomTransferOffer() {
        vector<Player*> ownPlayers = getOwnPlayers();
        if (ownPlayers.empty()) return;
        if (rand() % 100 < 15) {
            Player* p = ownPlayers[rand() % ownPlayers.size()];
            double multiplier = 0.9 + (rand() % 41) / 100.0;
            double offerPrice = p->getValue() * multiplier;
            Club* buyer = clubs[rand() % clubs.size()];
            if (buyer == userClub) buyer = clubs[1];
            cout << "\n   📞 ПОСТУПИЛО ПРЕДЛОЖЕНИЕ!" << endl;
            cout << "      " << buyer->getName() << " предлагает " << offerPrice << " млн € за " << p->getName() << endl;
            if (multiplier > 1.05)
                cout << "      ✅ Это на " << (offerPrice - p->getValue()) << " млн € ВЫШЕ рыночной стоимости!" << endl;
            else if (multiplier < 0.95)
                cout << "      ⚠️ Это на " << (p->getValue() - offerPrice) << " млн € НИЖЕ рыночной стоимости." << endl;
            if (UI::safeConfirm("      Принять предложение? (y/n): ")) {
                if (!isPlayerOnLoan(p)) {
                    userClub->receive(offerPrice);
                    userClub->removePlayer(p);
                    freeAgents.push_back(p);
                    totalEarned += offerPrice;
                    cout << "   ✅ " << p->getName() << " продан за " << offerPrice << " млн €!" << endl;
                } else {
                    cout << "   ❌ Сделка отменена: игрок находится в аренде!" << endl;
                }
            }
        }
    }
};

class Game {
    Club* userClub;
    vector<Club*>  clubs;
    vector<Agent*> agents;
    vector<Player*> freeAgents;
    vector<LoanInfo> activeLoans;
    TransferWindow* window;
    Date currentDate;
    int daysPlayed;
    double totalSpent;
    double totalEarned;

    TransferManager* transferManager;

    vector<Player*> getOwnPlayers() {
        return transferManager->getOwnPlayers();
    }

public:
    Game() : currentDate(1, 6, 2026), daysPlayed(0), totalSpent(0), totalEarned(0) {
        srand(static_cast<unsigned>(time(nullptr)));

        agents.push_back(new Agent(1, "Jorge Mendes",      55, 10));
        agents.push_back(new Agent(2, "Mino Raiola Jr",    40, 12));
        agents.push_back(new Agent(3, "Jonathan Barnett",  60,  8));

        freeAgents.push_back(new Player(101, "Kylian Mbappe",    25, "FW", 180, agents[1]));
        freeAgents.push_back(new Player(102, "Erling Haaland",   23, "FW", 200, agents[1]));
        freeAgents.push_back(new Player(103, "Jamal Musiala",    21, "MF",  85, agents[0]));
        freeAgents.push_back(new Player(104, "Jude Bellingham",  20, "MF", 120, agents[2]));
        freeAgents.push_back(new Player(105, "Pedri",            21, "MF",  70, agents[0]));
        freeAgents.push_back(new Player(106, "Gavi",             19, "MF",  60, agents[1]));
        freeAgents.push_back(new Player(107, "Ronald Araujo",    25, "DF",  55, agents[2]));
        freeAgents.push_back(new Player(108, "Virgil van Dijk",  32, "DF",  45, agents[0]));
        freeAgents.push_back(new Player(109, "Alisson Becker",   31, "GK",  55, agents[1]));

        clubs.push_back(new Club(1, "FC Barcelona",         300));
        clubs.push_back(new Club(2, "Real Madrid",          350));
        clubs.push_back(new Club(3, "Paris Saint-Germain",  500));
        clubs.push_back(new Club(4, "Manchester City",      600));
        clubs.push_back(new Club(5, "FC Liverpool",         400));
        clubs.push_back(new Club(6, "Spartak Moscow",       200));
        clubs.push_back(new Club(7, "Zenit St-Petersburg",  250));

        userClub = clubs[5];
        window   = new TransferWindow(Date(1, 6, 2026), Date(31, 8, 2026));

        transferManager = new TransferManager(
            userClub, clubs, freeAgents, activeLoans, totalSpent, totalEarned
        );
    }

    ~Game() {
        delete transferManager;
        for (auto c : clubs)      delete c;
        for (auto p : freeAgents) delete p;
        for (auto a : agents)     delete a;
        delete window;
    }

    void nextDay() {
        daysPlayed++;
        cout << "\n   📅 НАЧИНАЕТСЯ НОВЫЙ ДЕНЬ - " << ++currentDate << endl;
        Market::addDailyIncome(userClub, totalEarned);
        Market::randomMarketChange(freeAgents, getOwnPlayers());
        transferManager->processExpiredLoans(currentDate);
        transferManager->randomTransferOffer();
        window->showStatus(currentDate);
    }

    void run() {
        UI::showWelcome();
        int choice;
        do {
            UI::showMenu(currentDate, window, userClub, activeLoans);
            choice = UI::safeInput<int>("\n   Выберите действие: ", 1, 7);

            switch (choice) {
                case 1:
                    userClub->showSquad();
                    break;
                case 2:
                    if (!window->isOpen(currentDate)) {
                        cout << "\n   ❌ Трансферное окно закрыто! Нельзя покупать игроков.\n";
                    } else {
                        transferManager->buyPlayer();
                    }
                    break;
                case 3:
                    if (!window->isOpen(currentDate)) {
                        cout << "\n   ❌ Трансферное окно закрыто! Нельзя брать в аренду.\n";
                    } else {
                        transferManager->loanPlayer(currentDate);
                    }
                    break;
                case 4:
                    if (!window->isOpen(currentDate)) {
                        cout << "\n   ❌ Трансферное окно закрыто! Нельзя продавать игроков.\n";
                    } else {
                        transferManager->sellPlayer();
                    }
                    break;
                case 5:
                    nextDay();
                    break;
                case 6:
                    UI::showStats(daysPlayed, userClub, totalEarned, totalSpent,
                                  getOwnPlayers(), activeLoans);
                    break;
                case 7:
                    cout << "\n   🚪 Выход из игры... Спасибо за игру!\n";
                    break;
            }
        } while (choice != 7);
    }
};

int main() {
    try {
        Game game;
        game.run();
    } catch (const exception& e) {
        cerr << "\n   ❌ КРИТИЧЕСКАЯ ОШИБКА: " << e.what() << endl;
        return 1;
    }
    return 0;
}