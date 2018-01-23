// 
// NEWorld: GUI.h
// NEWorld: A Free Game with Similar Rules to Minecraft.
// Copyright (C) 2015-2018 NEWorld Team
// 
// NEWorld is free software: you can redistribute it and/or modify it 
// under the terms of the GNU Lesser General Public License as published
// by the Free Software Foundation, either version 3 of the License, or 
// (at your option) any later version.
// 
// NEWorld is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY 
// or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General 
// Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with NEWorld.  If not, see <http://www.gnu.org/licenses/>.
// 

#pragma once
#include "Definitions.h"
#include "I18N.h"

extern int getMouseButton();
extern int getMouseScroll();

inline std::string BoolYesNo(bool b) {
    return b ? I18N::get("NEWorld.yes") : I18N::get("NEWorld.no");
}

inline std::string BoolEnabled(bool b) {
    return b ? I18N::get("NEWorld.enabled") : I18N::get("NEWorld.disabled");
}

template <typename T>
std::string strWithVar(const std::string& str, T var) {
    std::stringstream ss;
    ss << str << var;
    return ss.str();
}

template <typename T>
std::string Var2Str(T var) {
    std::stringstream ss;
    ss << var;
    return ss.str();
}

//图形界面系统。。。正宗OOP！！！
namespace GUI {
    extern float linewidth;
    extern float linealpha;
    extern float FgR;
    extern float FgG;
    extern float FgB;
    extern float FgA;
    extern float BgR;
    extern float BgG;
    extern float BgB;
    extern float BgA;

    extern int nScreenWidth;
    extern int nScreenHeight;
    extern unsigned int transitionList;
    extern unsigned int lastdisplaylist;
    extern double transitionTimer;
    extern bool transitionForward;

    void clearTransition();
    void screenBlur();
    void drawBackground();
    void initStretch();
    void endStretch();

    class Form;

    struct Margin {
        
    };

    class Controls {
    public:
        //控件基类，只要是控件都得继承这个
        virtual ~Controls() { }

        int id, xmin, ymin, xmax, ymax;
        Form* parent;

        virtual void update() { } //莫非这个就是传说中的虚函数？
        virtual void render() { } //貌似是的！
        virtual void destroy() { }
        
        void updatepos();
        void resize(int xi_r, int xa_r, int yi_r, int ya_r, double xi_b, double xa_b, double yi_b, double ya_b);
    private:
        int mXminR, mYminR, mXmaxR, mYmaxR;
        double _xmin_b, _ymin_b, _xmax_b, _ymax_b;
    };

    class Label : public Controls {
    public:
        //标签
        std::string text;
        bool mouseon, focused;

        Label() : mouseon(false), focused(false) { };
        Label(const std::string& t,
              int xi_r, int xa_r, int yi_r, int ya_r, double xi_b, double xa_b, double yi_b, double ya_b);
        void update() override;
        void render() override;
    };

    class Button : public Controls {
    public:
        //按钮
        std::string text;
        bool mouseon, focused, pressed, clicked, enabled;

        Button() : mouseon(false), focused(false), pressed(false), clicked(false), enabled(false) { };
        Button(const std::string& t,
               int xi_r, int xa_r, int yi_r, int ya_r, double xi_b, double xa_b, double yi_b, double ya_b);
        void update() override;
        void render() override;
    };

    class Trackbar : public Controls {
    public:
        //该控件的中文名我不造
        std::string text;
        int barwidth;
        int barpos;
        bool mouseon, focused, pressed, enabled;

        Trackbar() : mouseon(false), focused(false), pressed(false), enabled(false) { };
        Trackbar(const std::string& t, int w, int s,
                 int xi_r, int xa_r, int yi_r, int ya_r, double xi_b, double xa_b, double yi_b, double ya_b);
        void update() override;
        void render() override;
    };

    class Textbox : public Controls {
    public:
        //文本框
        std::string text;
        bool mouseon, focused, pressed, enabled;

        Textbox() : mouseon(false), focused(false), pressed(false), enabled(false) { };
        Textbox(const std::string& t,
                int xi_r, int xa_r, int yi_r, int ya_r, double xi_b, double xa_b, double yi_b, double ya_b);
        void update() override;
        void render() override;
    };

    class Vscroll : public Controls {
    public:
        //垂直滚动条
        int barheight, barpos;
        bool mouseon, focused, pressed, enabled;
        bool defaultv, msup, msdown, psup, psdown;

        Vscroll() : mouseon(false), focused(false), pressed(false), enabled(false) { };
        Vscroll(int h, int s,
                int xi_r, int xa_r, int yi_r, int ya_r, double xi_b, double xa_b, double yi_b, double ya_b);
        void update() override;
        void render() override;
    };

    class Imagebox : public Controls {
    public:
        //图片框
        float txmin, txmax, tymin, tymax;
        TextureID imageid;

        Imagebox() : imageid(0) { };
        Imagebox(float _txmin, float _txmax, float _tymin, float _tymax, TextureID iid,
                 int xi_r, int xa_r, int yi_r, int ya_r, double xi_b, double xa_b, double yi_b, double ya_b);
        void update() override;
        void render() override;
    };

    typedef void (*UIVoidF)();

    // 窗体 / 容器
    class Form {
    public:
        std::vector<Controls*> children;
        bool tabp{}, shiftp{}, enterp{}, enterpl{};
        bool upkp{}, downkp{}, upkpl{}, downkpl{}, leftkp{}, rightkp{}, leftkpl{}, rightkpl{}, backspacep{}, backspacepl{}, updated{};
        int maxid{}, currentid{}, focusid{}, mx{}, my{}, mw{}, mb{}, mxl{}, myl{}, mwl{}, mbl{};
        unsigned int displaylist{};
        bool MouseOnTextbox{};
        void Init();
        void registerControl(Controls* c);
        void registerControls(std::initializer_list<Controls*> controls);
        void update();
        void render();
        Controls* getControlByID(int cid);
        void cleanup();

        virtual void onLoad() { }

        virtual void onUpdate() { }

        UIVoidF Background;

        virtual void onRender() { }

        virtual void onLeave() { }

        Form();
        void singleloop();
        virtual ~Form();
    };

    void appStart();
    void pushPage(Form* View);
    void popPage();
    void BackToMain();
    void ClearStack();
    Form* GetMain();
}
