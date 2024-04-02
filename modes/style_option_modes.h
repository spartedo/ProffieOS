#ifndef MODES_STYLE_OPTION_MODES_H
#define MODES_STYLE_OPTION_MODES_H

namespace mode {

template<class SPEC>
class SelectArgSmoothMode : public SPEC::SmoothMode {
public:
  int get() override { return GetIntArg(menu_current_blade, menu_current_arg); }
  int set(int x) {
    value_ = x;
    if (!getSL<SPEC>()->busy()) {
      getSL<SPEC>()->SayWhole(x * 100 / 32768);
      getSL<SPEC>()->SayPercent();
    }
  }

  void select() override {
    SPEC::SmoothMode::select();
    SetIntArg(menu_current_blade, menu_current_arg, value_);
    popMode();
  }

private:  
  int value_ = 0;
};

template<class SPEC>
class SelectArgTime : public SPEC::SmoothMode {
public:
  int get() override { return GetIntArg(menu_current_blade, menu_current_arg); }
  float t(int x) { return powf(x / 32768.0f, 2.0) * 30.0; }
  int set(int x) {
    value_ = x;
    if (!getSL<SPEC>()->busy()) {
      getSL<SPEC>()->SayNumber(t(x), SAY_DECIMAL);
      getSL<SPEC>()->SaySeconds();
    }
  }

  void select() override {
    SPEC::SmoothMode::select();
    SetIntArg(menu_current_blade, menu_current_arg, (int)(t(value_) * 1000));
    popMode();
  }

private:  
  int value_ = 0;
};

template<class SPEC>
class SelectArgNumber : public SPEC::MenuBase {
public:
  void activate(bool onreturn) override {
    SPEC::MenuBase::activate(onreturn);
    if (!onreturn) {
      int max = GetMaxStyleArg();
      if (max < 0) max = 32768;
      max_ = max;
      this->pos_ = GetIntArg(menu_current_blade, menu_current_arg);
      // TODO: What if pos_ > max_ ?
    }
  }
  void say() override {
    getSL<SPEC>()->SayWhole(this->pos_);
  }
  uint16_t size() override { return max_; }

  void select() override {
    SPEC::SmoothMode::select();
    SetIntArg(menu_current_blade, menu_current_arg, this->pos_);
    popMode();
  }
private:
  uint16_t max_;
};

template<class SPEC>
class SelectArgMode : public SPEC::MenuBase {
public:
  void activate(bool onreturn) override {
    // TODO: Set pos_ to something reasonable?
    arginfo_ = style_parser.GetArgInfo(GetStyle(menu_current_blade));
    SPEC::MenuBase::activate(onreturn);
  }
  int size() override { return arginfo_.used(); }
  void say() override {
    getSL<SPEC>()->SayArgument(getCurrentArgument());
  }

  void select() override {
    menu_current_arg = getCurrentArgument();
    if (arginfo_.iscolor(menu_current_arg)) {
      pushMode<SPEC::SelectArgColor>();
    } else {
      int max_arg = GetMaxStyleArg();
      if (max_arg == 32768 || max_arg == 32767) {
	pushMode<SPEC::SelectArgSmooth>();
      } else if (max_arg <= 0 && isTimeArg(menu_current_arg)) {
	pushMode<SPEC::SelectArgTime>();
      } else {
	pushMode<SPEC::SelectArgNumber>();
      }
    }
  }

protected:
  int getCurrentArgument() { return arginfo_.nth(this->pos_); }
  ArgInfo arginfo_;
};


int menu_selected_blade;

// Select this style for copying.
template<class SPEC>
struct SelectStyleEntry : public  MenuEntry {
  void say(int entry) override {
    getSL<SPEC>->SaySelectStyle();
  }
  void select(int entry) override {
    menu_selected_preset = GetPresetPosition();
    menu_selected_blade = menu_current_blade;
    getSL<SPEC>->SaySelect();
  }
};

template<class SPEC>
struct ApplyColorsFromSelectedStyleEntry : public  MenuEntry {
  void say(int entry) override {
    getSL<SPEC>->SayApplyColorsFromSelectedStyle();
  }
  void select(int entry) override {
    if (menu_selected_preset == -1) {
      getSL<SPEC>->SayNoStyleSelected();
      return;
    }
    CurrentPreset preset;
    preset.Load(menu_selected_preset);
    const char* FROM = preset.GetStyle(menu_selected_blade);
    const char* TO = GetStyle(menu_selected_blade);
    SetStyle(menu_selected_blade, style_parser.CopyColorArguments(FROM, TO));
    getSL<SPEC>->SaySelect();
  }
};

template<class SPEC>
struct ApplyColorsToAllBladesEntry : public  MenuEntry {
  void say(int entry) override {
    getSL<SPEC>->SayApplyColorsToAllBlades();
  }
  void select(int entry) override {
    getSL<SPEC>->SaySelect();
    const char* FROM = GetStyle(menu_selected_blade);
    for (int b = 1; b <= NUM_BLADES; b++) {
      if (b == menu_selected_blade) continue;
      SetStyle(b, style_parser.CopyColorArguments(FROM, GetStyle(b)));
    }
  }
};

template<class SPEC>
struct ApplyStyleArumentsFromSelectedStyleEntry : public  MenuEntry {
  void say(int entry) override {
    getSL<SPEC>->SayApplyStyleOptionsFromSelectedStyle();
  }
  void select(int entry) override {
    if (menu_selected_preset == -1) {
      getSL<SPEC>->SayNoStyleSelected();
      return;
    }
    CurrentPreset preset;
    preset.Load(menu_selected_preset);
    const char* FROM = preset.GetStyle(menu_selected_blade);
    const char* TO = GetStyle(menu_selected_blade);
    SetStyle(menu_selected_blade, style_parser.CopyArguments(FROM, TO));
    getSL<SPEC>->SaySelect();
  }
};


template<class SPEC>
struct ResetColorsEntry : public  MenuEntry {
  void say(int entry) override {
    getSL<SPEC>->SayResetColors();
  }
  void select(int entry) override {
    if (menu_selected_preset == -1) {
      getSL<SPEC>->SayNoStyleSelected();
      return;
    }
    CurrentPreset preset;
    preset.Load(menu_selected_preset);
    const char* TO = GetStyle(menu_selected_blade);
    SetStyle(menu_selected_blade, style_parser.CopyColorArguments("builtin 0 0", TO));
    getSL<SPEC>->SaySelect();
  }
};

template<class SPEC>
struct ResetStyleArumentsEntry : public MenuEntry {
  void say(int entry) override {
    getSL<SPEC>->SayApplyStyleOptionsFromSelectedStyle();
  }
  void select(int entry) override {
    if (menu_selected_preset == -1) {
      getSL<SPEC>->SayNoStyleSelected();
      return;
    }
    CurrentPreset preset;
    preset.Load(menu_selected_preset);
    const char* TO = GetStyle(menu_selected_blade);
    SetStyle(menu_selected_blade, style_parser.CopyArguments("builtin 0 0", TO));
    getSL<SPEC>->SaySelect();
  }
};

template<class SPEC>
struct SelectStyleMenu : public SPEC::MenuBase {
  uint16_t size() override {
    return NUM_BLADES * current_config->num_presets;
  }
  int blade() { return this->pos_ / NUM_BLADES; }
  int preset() { return this->pos_ % NUM_BLADES; }
  void activate(bool onreturn) override {
    SPEC::MenuBase::Activate();
    int preset;
    int style;
    style_parser.GetBuiltinPos(GetStyle(menu_current_blade), &preset, &style);
    this->pos_ = preset * NUM_BLADES + style;
  }
  
  void say(int entry) override {
    getSL<SPEC>->SayBlade();
    getSL<SPEC>->SayWhole(blade());
    getSL<SPEC>->SayPreset();
    getSL<SPEC>->SayWhole(preset());
  }

  void select(int entry) override {
    LSPtr<char> builtin = CurrentPreset::mk_builtin_str(preset(), blade());
    SetStyle(menu_selected_blade, style_parser.CopyArguments(builtin.get(), GetStyle(menu_selected_blade)));
    getSL<SPEC>->SaySelect();
    popMode();
  }
};

template<class SPEC>
using EditStyleMenu = typename SPEC::template MenuListMode<
  SubMenuEntry<typename SPEC::EditStyleOptions, typename SPEC::SoundLibrary::tEditStyleOptions>,
  typename SPEC::SelectStyleEntry,
  typename SPEC::ApplyColorsFromSelectedStyleEntry,
  typename SPEC::ApplyStyleArgumentsFromSelectionEntry,
  typename SPEC::ApplyColorsToAllBladesEntry,
  typename SPEC::ResetColorsEntry,
  typename SPEC::ResetToDefaultEntry,
  SubMenuEntry<typename SPEC::SelectStyleMenu, typename SPEC::SoundLibrary::tEditStyle>>;

}   // namespace mode

#endif
