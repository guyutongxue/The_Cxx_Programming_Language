#include <iostream>

bool isLeap(int y) {
    return y % 4 == 0 && y % 100 != 0 || y % 400 == 0;
}

int numDaysOfYear(int y) {
    return isLeap(y) ? 366 : 365;
}

int numDaysOfMonth(int y, int m) {
    constexpr int NUM_DAYS[]{0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    return NUM_DAYS[m] + (m == 2 && isLeap(y) ? 1 : 0);
}

class Date {
private:
    int daysFromEpoch() const {
        int cnt = 0;
        for (int y = 1970; y < this->year; ++y) {
            cnt += numDaysOfYear(y);
        }
        for (int m = 1; m < this->month; ++m) {
            cnt += numDaysOfMonth(this->year, m);
        }
        cnt += this->day - 1;
        return cnt;
    }

    void makeFromTimestamp(int timestamp) {
        this->year = 1970;
        this->month = 1;
        this->day = 1;
        while (timestamp >= numDaysOfYear(this->year)) {
            timestamp -= numDaysOfYear(this->year);
            ++this->year;
        }
        while (timestamp >= numDaysOfMonth(this->year, this->month)) {
            timestamp -= numDaysOfMonth(this->year, this->month);
            ++this->month;
        }
        this->day += timestamp;
    }

public:
    int year;
    int month;
    int day;

    int weekDay() const {
        return (4 + daysFromEpoch()) % 7;
    }

    Date(int year, int month = 1, int day = 1) : year{year}, month{month}, day{day} {}

    int operator-(this const Date& lhs, const Date& rhs) {
        return lhs.daysFromEpoch() - rhs.daysFromEpoch();
    }

    Date operator+(int duration) const {
        Date copy{*this};
        copy.makeFromTimestamp(this->daysFromEpoch() + duration);
        return copy;
    }

    Date operator-(int duration) const {
        return *this + -duration;
    }
    
    Date& operator+=(int duration) {
        return *this = *this + duration;
    }

    Date& operator-=(int duration) {
        return *this = *this - duration;
    }
};

Date operator+(int duration, const Date& base) {
    return base + duration;
}

std::ostream& operator<<(std::ostream& os, const Date& rhs) {
    return os << rhs.year << '/' << rhs.month << '/' << rhs.day;
}

int main() {
    std::cout << "----------------" << std::endl;

    // 1. constructor

    std::cout << "1. constructor" << std::endl;

    Date a(2022);
    Date b(2022, 2);
    Date c{2022, 9, 14};

    std::cout << "----------------" << std::endl;

    // 2. Date - Date

    std::cout << "2. Date - Date" << std::endl;

    std::cout << b - a << std::endl;
    std::cout << c - Date{1970, 1, 1} << std::endl;

    std::cout << "----------------" << std::endl;

    // 3. weekDay

    std::cout << "3. weekDay" << std::endl;

    std::cout << c.weekDay() << std::endl;

    std::cout << "----------------" << std::endl;

    // 4. Date + int

    std::cout << "4. Date + int" << std::endl;

    std::cout << Date{1970, 1, 1} + 10000 << std::endl;
    std::cout << Date{1970, 1, 1} + (c - Date{1970, 1, 1}) << std::endl;

    std::cout << "----------------" << std::endl;

    // 5. int + Date

    std::cout << "5. int + Date" << std::endl;

    std::cout << 10000 + Date{1970, 1, 1} << std::endl;

    std::cout << "----------------" << std::endl;

    // 6. Date - int

    std::cout << "6. Date - int" << std::endl;

    std::cout << c - (c - Date{ 1970,1,1 }) << std::endl;

    std::cout << "----------------" << std::endl;

    // 7. Date += int

    std::cout << "7. Date += int" << std::endl;

    a += 100;
    std::cout << a << std::endl;

    std::cout << "----------------" << std::endl;

    // 8*. (Date -= int) -> Date&

    std::cout << "8*. (Date -= int) -> Date&" << std::endl;

    std::cout << (a -= 50) << std::endl;
    std::cout << ((a -= 25) -= 25) << std::endl;
    std::cout << a << std::endl;

    std::cout << "----------------" << std::endl;

    // 9*. const this

    std::cout << "9*. const this" << std::endl;

    const Date d = a;
    std::cout << d + 100 << std::endl;
    std::cout << 100 + d << std::endl;
    std::cout << d - 100 << std::endl;
    std::cout << d - d << std::endl;

    std::cout << "----------------" << std::endl;
}