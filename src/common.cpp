#include "common.h"

static std::unordered_set<std::string> table;

void jcc_ice_assert(const char *expr, std::stacktrace s, const char *message) {
    if (message)
        std::cout << "ICE: " << message << ", " << expr << "\n\n";
    else
        std::cout << "ICE: " << expr << "\n\n";
    std::cout << "Stack Trace:\n" << s << '\n';
    std::exit(-1);
}

std::string* Strand::intern(std::string str) {
    auto location = table.find(str);
    if(location == table.end()) {
        auto [it, inserted] = table.insert(str);
        return const_cast<std::string*>(&(*it));
    } else {
        return const_cast<std::string*>(&(*location));
    }
}

std::string* Strand::intern(const char *char_ptr) {
    std::string str(char_ptr);
    auto location = table.find(str);
    if(location == table.end()) {
        auto [it, inserted] = table.insert(str);
        return const_cast<std::string*>(&(*it));
    } else {
        return const_cast<std::string*>(&(*location));
    }
}

Strand::Strand(const char *str): ptr{intern(str)} {

}

Strand::Strand(std::string str): ptr{intern(str)} {

}

Strand& Strand::operator=(const char *str) {
    ptr = intern(str);
    return *this;
}

Strand& Strand::operator=(std::string str) {
    ptr = intern(str);
    return *this;
}


std::ostream &operator<<(std::ostream &out, const Strand &strand) {
    out << strand.value();
    return out;
}
