#ifndef MODE_SORTED_LIST_MENUES
#define MODE_SORTED_LIST_MENUES

#include "../common/command_parser.h"

void SetFont(const char* font);
void SetTrack(const char* track);
const char* GetFont();
const char* GetTrack();
void chdir(const StringPiece chdir);

namespace mode {

// Base class for menues that use a command to list all possible options.
template<class SPEC>
class SelectFromListModeBase : public SPEC::MenuBase, public SortedLineHelper<128> {
public:
  SelectFromListModeBase(const char* cmd) : SortedLineHelper<128>(cmd) {}

  uint16_t size() override { return num_lines_; }
  const StringPiece get() { return SortedLineHelper<128>::get(this->pos_); }

  void mode_activate(bool onreturn) override {
    this->pos_ = SortedLineHelper<128>::init();
    SPEC::MenuBase::mode_activate(onreturn);
    this->say();
  }
};

// Sets the font of the current preset.
template<class SPEC>
class SelectFontMode : public SPEC::SelectFromListModeBase {
public:
  SelectFontMode() : SPEC::SelectFromListModeBase("list_fonts") {}
  StringPiece get_current_value() override {
    return current_value_;
  }
  void mode_activate(bool onreturn) override {
    current_value_ = match_pattern(GetFont(), FONT_PATTERN);
    SPEC::SelectFromListModeBase::mode_activate(onreturn);
  }
  void exit() override {
    SPEC::SelectFromListModeBase::exit();
    chdir(GetFont());
  }
  void say() override {
    chdir(this->get());
    hybrid_font.SB_Effect(EFFECT_NEWFONT, 0);
  }
  void select() override {
    SetFont(format_pattern(FONT_PATTERN, this->get()));
    // Say "saved" or something?
  }
private:
  StringPiece current_value_;
};

// Sets the track of the current preset.
template<class SPEC>
class SelectTrackMode : public SPEC::SelectFromListModeBase {
public:
  SelectTrackMode() : SPEC::SelectFromListModeBase("list_tracks") {}
  StringPiece get_current_value() override {
    return GetTrack();
  }
  void fadeout(float len) override {
    if (track_player_) {
      track_player_->set_fade_time(len);
      track_player_->FadeAndStop();
      track_player_.Free();
    }
  }
  void say() override {
    MountSDCard();
    EnableAmplifier();
    track_player_ = GetFreeWavPlayer();
    track_player_->Play(this->get(), 20.0);  // start 20s from the beginning
  }
  void select() override {
    SetTrack(mkstr(this->get()));
    // Say "saved" or something?
  }
};
  
}  // namespace mode

#endif
