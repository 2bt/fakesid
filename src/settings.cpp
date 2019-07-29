#include "settings.hpp"
#include "foo.hpp"

namespace {

Settings m_settings;

} // namespace

Settings& settings() { return m_settings; }

void load_settings() {
    m_settings.play_in_background  = android::load_pref("playInBackground", m_settings.play_in_background);
    m_settings.track_row_highlight = android::load_pref("trackRowHighlight", m_settings.track_row_highlight);
}

void save_settings() {
    android::store_pref("playInBackground", m_settings.play_in_background);
    android::store_pref("trackRowHighlight", m_settings.track_row_highlight);
    android::store_pref_apply();
}
