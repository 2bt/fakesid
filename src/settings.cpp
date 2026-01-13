#include "settings.hpp"

namespace {

Settings m_settings;

enum {
    #define X(n, ...) SETTING_##n,
    SETTINGS(X)
    #undef X
};

} // namespace

Settings& settings() { return m_settings; }

char const* get_setting_name(int i) {
    switch (i) {
    #define X(n, ...) case SETTING_##n: return #n;
    SETTINGS(X)
    #undef X
    default: return nullptr;
    }
}

int get_setting_value(int i) {
    switch (i) {
    #define X(n, ...) case SETTING_##n: return m_settings.n;
    SETTINGS(X)
    #undef X
    default: return 0;
    }
}

void set_setting_value(int i, int v) {
    switch (i) {
    #define X(n, ...) case SETTING_##n: m_settings.n = v; break;
    SETTINGS(X)
    #undef X
    default: break;
    }
}
