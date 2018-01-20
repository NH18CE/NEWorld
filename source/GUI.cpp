// 
// NEWorld: GUI.cpp
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

#include "GUI.h"
#include "TextRenderer.h"
#include "Frustum.h"
#include "AudioSystem.h"
#include <deque>


//图形界面系统。。。正宗OOP！！！
namespace GUI {
    double stdppi = 96.0f;
    int nScreenWidth, nScreenHeight;
    float linewidth = 1.0f;
    float linealpha = 0.9f;
    float FgR = 0.2f;
    float FgG = 0.2f;
    float FgB = 0.2f;
    float FgA = 0.6f;
    float BgR = 0.2f;
    float BgG = 0.2f;
    float BgB = 0.2f;
    float BgA = 0.3f;
    unsigned int transitionList;
    unsigned int lastdisplaylist;
    double transitionTimer;
    bool transitionForward;

    void clearTransition() {
        if (transitionList != 0) {
            glDeleteLists(transitionList, 1);
            transitionList = 0;
        }
        if (lastdisplaylist != 0) {
            glDeleteLists(lastdisplaylist, 1);
            lastdisplaylist = 0;
        }
    }

    void screenBlur() {
        static int szl = 0, rl = 0;
        static float* mat = nullptr;
        static uint8_t* scr; //屏幕像素缓存

        int w = windowwidth; //Width
        int h = windowheight; //Height
        int r = 2; //范围
        int sz = 1;
        float scale = 2;
        TextureID bgTex;

        while (sz < w || sz < h) sz *= 2;
        if (sz != szl) {
            szl = sz;
            delete[] scr;
            scr = new uint8_t[sz * sz * 3];
        }

        if (rl != r) {
            if (mat != nullptr) delete[] mat;
            int size = r * 2 + 1;
            int size2 = size * size;
            float sum = 0.0f;
            int index = 0;
            mat = new float[size2];
            for (int x = -r; x <= r; x++) {
                for (int y = -r; y <= r; y++) {
                    float val = 1.0f / (float)(abs(x) + abs(y) + 1);
                    mat[index++] = val;
                    sum += val;
                }
            }
            sum = 1.0f / sum;
            for (int i = 0; i < size2; i++) mat[i] *= sum;
        }

        glPixelStorei(GL_PACK_ALIGNMENT, 1);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glReadPixels(0, 0, sz, sz, GL_RGB, GL_UNSIGNED_BYTE, scr);
        //glColorMask(true, true, true, false);

        glGenTextures(1, &bgTex);
        glBindTexture(GL_TEXTURE_2D, bgTex);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, sz, sz, 0, GL_RGB, GL_UNSIGNED_BYTE, scr);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        for (auto x = -r; x <= r; x++) {
            for (auto y = -r; y <= r; y++) {
                const auto d = mat[(x + r) * (r * 2 + 1) + y + r];
                glColor4f(1.0f, 1.0f, 1.0f, d);
                glBegin(GL_QUADS);
                glTexCoord2f(0.0f, static_cast<float>(h) / sz);
                glVertex2f(x * scale, y * scale);
                glTexCoord2f(static_cast<float>(w) / sz, static_cast<float>(h) / sz);
                glVertex2f(w + x * scale, y * scale);
                glTexCoord2f(static_cast<float>(w) / sz, 0.0f);
                glVertex2f(w + x * scale, h + y * scale);
                glTexCoord2f(0.0f, 0.0f);
                glVertex2f(x * scale, h + y * scale);
                glEnd();
            }
        }

        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDeleteTextures(1, &bgTex);
        glPixelStorei(GL_PACK_ALIGNMENT, 4);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
        //glColorMask(true, true, true, true);
        glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    }

    void drawBackground() {
        static Frustum frus;
        static double startTimer = timer();
        double elapsed = timer() - startTimer;
        frus.loadIdentity();
        frus.setPerspective(90.0f, (float)windowwidth / windowheight, 0.1f, 10.0f);

        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glMultMatrixf(frus.getProjMatrix());
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        glRotated(elapsed * 4.0, 0.1, 1.0, 0.1);
        glClearColor(0.0, 0.0, 0.0, 0.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glDepthFunc(GL_LEQUAL);
        glDisable(GL_CULL_FACE);
        glEnable(GL_TEXTURE_2D);
        glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

        //Begin to draw a cube
        glBindTexture(GL_TEXTURE_2D, tex_mainmenu[0]);
        glBegin(GL_QUADS);
        glTexCoord2f(0.0f, 1.0f);
        glVertex3f(-1.0f, 1.0f, -1.0f);
        glTexCoord2f(1.0f, 1.0f);
        glVertex3f(1.0f, 1.0f, -1.0f);
        glTexCoord2f(1.0f, 0.0f);
        glVertex3f(1.0f, -1.0f, -1.0f);
        glTexCoord2f(0.0f, 0.0f);
        glVertex3f(-1.0f, -1.0f, -1.0f);
        glEnd();
        glBindTexture(GL_TEXTURE_2D, tex_mainmenu[1]);
        glBegin(GL_QUADS);
        glTexCoord2f(0.0f, 1.0f);
        glVertex3f(1.0f, 1.0f, -1.0f);
        glTexCoord2f(1.0f, 1.0f);
        glVertex3f(1.0f, 1.0f, 1.0f);
        glTexCoord2f(1.0f, 0.0f);
        glVertex3f(1.0f, -1.0f, 1.0f);
        glTexCoord2f(0.0f, 0.0f);
        glVertex3f(1.0f, -1.0f, -1.0f);
        glEnd();
        glBindTexture(GL_TEXTURE_2D, tex_mainmenu[2]);
        glBegin(GL_QUADS);
        glTexCoord2f(0.0f, 1.0f);
        glVertex3f(1.0f, 1.0f, 1.0f);
        glTexCoord2f(1.0f, 1.0f);
        glVertex3f(-1.0f, 1.0f, 1.0f);
        glTexCoord2f(1.0f, 0.0f);
        glVertex3f(-1.0f, -1.0f, 1.0f);
        glTexCoord2f(0.0f, 0.0f);
        glVertex3f(1.0f, -1.0f, 1.0f);
        glEnd();
        glBindTexture(GL_TEXTURE_2D, tex_mainmenu[3]);
        glBegin(GL_QUADS);
        glTexCoord2f(0.0f, 1.0f);
        glVertex3f(-1.0f, 1.0f, 1.0f);
        glTexCoord2f(1.0f, 1.0f);
        glVertex3f(-1.0f, 1.0f, -1.0f);
        glTexCoord2f(1.0f, 0.0f);
        glVertex3f(-1.0f, -1.0f, -1.0f);
        glTexCoord2f(0.0f, 0.0f);
        glVertex3f(-1.0f, -1.0f, 1.0f);
        glEnd();
        glBindTexture(GL_TEXTURE_2D, tex_mainmenu[4]);
        glBegin(GL_QUADS);
        glTexCoord2f(0.0f, 1.0f);
        glVertex3f(-1.0f, -1.0f, 1.0f);
        glTexCoord2f(1.0f, 1.0f);
        glVertex3f(-1.0f, -1.0f, -1.0f);
        glTexCoord2f(1.0f, 0.0f);
        glVertex3f(1.0f, -1.0f, -1.0f);
        glTexCoord2f(0.0f, 0.0f);
        glVertex3f(1.0f, -1.0f, 1.0f);
        glEnd();
        glBindTexture(GL_TEXTURE_2D, tex_mainmenu[5]);
        glBegin(GL_QUADS);
        glTexCoord2f(0.0f, 1.0f);
        glVertex3f(1.0f, 1.0f, 1.0f);
        glTexCoord2f(1.0f, 1.0f);
        glVertex3f(1.0f, 1.0f, -1.0f);
        glTexCoord2f(1.0f, 0.0f);
        glVertex3f(-1.0f, 1.0f, -1.0f);
        glTexCoord2f(0.0f, 0.0f);
        glVertex3f(-1.0f, 1.0f, 1.0f);
        glEnd();
    }

    //Get the Screen Physical Size and set stretch
    //NEVER CALL THIS FUNCTION BEFORE THE CONTEXT IS CREATED
    void initStretch() {
        ppistretch = true;
        glfwGetMonitorPhysicalSize(glfwGetPrimaryMonitor(), &nScreenWidth,
                                   &nScreenHeight);
        const auto mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
        double ppi = static_cast<double>(mode->width) / (static_cast<double>(nScreenWidth) / 25.4f);
        stretch = ppi / stdppi;
        //Calaulate the scale and resize the window
        windowwidth = static_cast<int>(windowwidth * stretch);
        windowheight = static_cast<int>(windowheight * stretch);
        glfwSetWindowSize(MainWindow, windowwidth, windowheight);
        TextRenderer::resize();
    }

    void endStretch() {
        ppistretch = false;
        windowwidth = static_cast<int>(windowwidth / stretch);
        windowheight = static_cast<int>(windowheight / stretch);
        stretch = 1.0;
        glfwSetWindowSize(MainWindow, windowwidth, windowheight);
        TextRenderer::resize();
    }

    void Controls::updatepos() {
        xmin = static_cast<int>(windowwidth * _xmin_b / stretch) + mXminR;
        ymin = static_cast<int>(windowheight * _ymin_b / stretch) + mYminR;
        xmax = static_cast<int>(windowwidth * _xmax_b / stretch) + mXmaxR;
        ymax = static_cast<int>(windowheight * _ymax_b / stretch) + mYmaxR;
    }

    void Controls::resize(int xi_r, int xa_r, int yi_r, int ya_r, double xi_b, double xa_b, double yi_b, double ya_b) {
        mXminR = xi_r;
        mXmaxR = xa_r;
        mYminR = yi_r;
        mYmaxR = ya_r;
        _xmin_b = xi_b;
        _xmax_b = xa_b;
        _ymin_b = yi_b;
        _ymax_b = ya_b;
    }

    void Label::update() {
        //更新标签状态
        if (parent->mx >= xmin && parent->mx <= xmax && parent->my >= ymin && parent->my <= ymax) //鼠标悬停
            mouseon = true;
        else
            mouseon = false;

        if (parent->mb == 1 && parent->mbl == 0 && mouseon) parent->focusid = id; //焦点在此
        focused = parent->focusid == id; //焦点
    }

    void Label::render() {
        float fcR = FgR;
        float fcG = FgG;
        float fcB = FgB;
        float fcA = FgA;
        if (mouseon) {
            fcR = FgR * 1.2f;
            fcG = FgG * 1.2f;
            fcB = FgB * 1.2f;
            fcA = FgA * 0.8f;
        }
        if (focused) {
            //Focus
            glDisable(GL_TEXTURE_2D);
            glColor4f(FgR * 0.6f, FgG * 0.6f, FgB * 0.6f, linealpha);
            glLineWidth(linewidth);
            //glBegin(GL_POINTS)
            //UIVertex(xmin - 1, ymin)
            //glEnd()
            glBegin(GL_LINE_LOOP);
            UIVertex(xmin, ymin);
            UIVertex(xmin, ymax);
            UIVertex(xmax, ymax);
            UIVertex(xmax, ymin);
            glEnd();
        }
        glEnable(GL_TEXTURE_2D);
        TextRenderer::setFontColor(fcR, fcG, fcB, fcA);
        TextRenderer::renderString((xmin + xmax - TextRenderer::getStrWidth(text)) / 2, (ymin + ymax - 20) / 2, text);
    }

    void Button::update() {
        if (!enabled) {
            mouseon = false, focused = false, pressed = false, clicked = false;
            return;
        }

        //更新按钮状态
        if (parent->mx >= xmin && parent->mx <= xmax && parent->my >= ymin && parent->my <= ymax)
            mouseon = true;
        else
            mouseon = false;

        if (((parent->mb == 1 && mouseon) || parent->enterp) && focused)
            pressed = true;
        else
            pressed = false;

        if (parent->mb == 1 && parent->mbl == 0 && mouseon) parent->focusid = id;
        if (parent->focusid == id) focused = true;
        else focused = false;

        clicked = ((parent->mb == 0 && parent->mbl == 1 && mouseon) || (parent->enterpl && !parent->enterp)) &&
            focused;
        //clicked = lp&&!pressed

        //TODO:chick
        //if (clicked)getAudioSystem().play();
    }

    void Button::render() {
        float fcR = FgR;
        float fcG = FgG;
        float fcB = FgB;
        float fcA = FgA;
        if (mouseon) {
            fcR = FgR * 1.2f;
            fcG = FgG * 1.2f;
            fcB = FgB * 1.2f;
            fcA = FgA * 0.8f;
        }
        if (pressed) {
            fcR = FgR * 0.8f;
            fcG = FgG * 0.8f;
            fcB = FgB * 0.8f;
            fcA = FgA * 1.5f;
        }
        if (!enabled) {
            fcR = FgR * 0.5f;
            fcG = FgG * 0.5f;
            fcB = FgB * 0.5f;
            fcA = FgA * 0.3f;
        }
        glColor4f(fcR, fcG, fcB, fcA);

        glDisable(GL_TEXTURE_2D); //Button
        glBegin(GL_QUADS);
        UIVertex(xmin, ymin);
        UIVertex(xmin, ymax);
        UIVertex(xmax, ymax);
        UIVertex(xmax, ymin);
        glEnd();
        glColor4f(FgR * 0.9f, FgG * 0.9f, FgB * 0.9f, linealpha);

        if (!enabled) glColor4f(0.5f, 0.5f, 0.5f, linealpha);
        glLineWidth(linewidth);
        glBegin(GL_LINE_LOOP);
        UIVertex(xmin, ymin);
        UIVertex(xmin, ymax);
        UIVertex(xmax, ymax);
        UIVertex(xmax, ymin);
        glEnd();

        glBegin(GL_LINE_LOOP);

        if (focused) glColor4f(1.0f, 1.0f, 1.0f, linealpha);
        else glColor4f(0.8f, 0.8f, 0.8f, linealpha);
        UIVertex(xmin + 1, ymin + 1);

        if (focused) glColor4f(1.0f, 1.0f, 1.0f, linealpha);
        else glColor4f(0.4f, 0.4f, 0.4f, linealpha);
        UIVertex(xmin + 1, ymax - 1);

        if (focused) glColor4f(1.0f, 1.0f, 1.0f, linealpha);
        else glColor4f(0.4f, 0.4f, 0.4f, linealpha);
        UIVertex(xmax - 1, ymax - 1);

        if (focused) glColor4f(1.0f, 1.0f, 1.0f, linealpha);
        else glColor4f(0.8f, 0.8f, 0.8f, linealpha);
        UIVertex(xmax - 1, ymin + 1);

        glEnd();

        glEnable(GL_TEXTURE_2D);
        TextRenderer::setFontColor(1.0f, 1.0f, 1.0f, 1.0f);
        if (!enabled) TextRenderer::setFontColor(0.6f, 0.6f, 0.6f, 1.0f);
        TextRenderer::renderString((xmin + xmax - TextRenderer::getStrWidth(text)) / 2, (ymin + ymax - 20) / 2, text);
    }

    void Trackbar::update() {
        if (!enabled) {
            mouseon = false, focused = false, pressed = false;
            return;
        }

        //更新TrackBar（到底该怎么翻译呢？）状态
        if (parent->mx >= xmin && parent->mx <= xmax && parent->my >= ymin && parent->my <= ymax && parent->mb == 1)
            parent->focusid = id;
        if (parent->mx >= xmin + barpos && parent->mx <= xmin + barpos + barwidth && parent->my >= ymin && parent->my <=
            ymax)
            mouseon = true;
        else mouseon = false;
        if (parent->mb == 1 && mouseon && focused)pressed = true;
        else if (parent->mbl == 0) pressed = false;
        if (parent->mb == 1 && parent->mbl == 0 && mouseon)parent->focusid = id;
        focused = parent->focusid == id;
        if (focused && pressed)barpos += parent->mx - parent->mxl;
        if (focused) {
            if (parent->upkp && !parent->upkpl) barpos -= 1;
            if (parent->downkp && !parent->downkpl) barpos += 1;
            if (parent->leftkp) barpos -= 1;
            if (parent->rightkp) barpos += 1;
        }
        if (barpos <= 0) barpos = 0;
        if (barpos >= xmax - xmin - barwidth) barpos = xmax - xmin - barwidth - 1;
    }

    void Trackbar::render() {
        float fcR = FgR;
        float fcG = FgG;
        float fcB = FgB;
        float fcA = FgA;
        float bcR = BgR;
        float bcG = BgG;
        float bcB = BgB;
        float bcA = BgA;
        if (mouseon) {
            fcR = FgR * 1.2f;
            fcG = FgG * 1.2f;
            fcB = FgB * 1.2f;
            fcA = FgA * 0.8f;
        }
        if (pressed) {
            fcR = FgR * 0.8f;
            fcG = FgG * 0.8f;
            fcB = FgB * 0.8f;
            fcA = FgA * 1.5f;
        }
        if (!enabled) {
            fcR = FgR * 0.5f;
            fcG = FgG * 0.5f;
            fcB = FgB * 0.5f;
            fcA = FgA * 0.3f;
        }
        glColor4f(bcR, bcG, bcB, bcA);
        glDisable(GL_TEXTURE_2D);
        glBegin(GL_QUADS);
        UIVertex(xmin, ymin);
        UIVertex(xmax, ymin);
        UIVertex(xmax, ymax);
        UIVertex(xmin, ymax);
        glEnd();
        glDisable(GL_TEXTURE_2D);
        glColor4f(fcR, fcG, fcB, fcA);
        glBegin(GL_QUADS);
        UIVertex(xmin + barpos, ymin);
        UIVertex(xmin + barpos + barwidth, ymin);
        UIVertex(xmin + barpos + barwidth, ymax);
        UIVertex(xmin + barpos, ymax);
        glEnd();
        glColor4f(FgR * 0.9f, FgG * 0.9f, FgB * 0.9f, linealpha);

        if (!enabled) glColor4f(0.5f, 0.5f, 0.5f, linealpha);
        glBegin(GL_LINE_LOOP);
        UIVertex(xmin, ymin);
        UIVertex(xmin, ymax);
        UIVertex(xmax, ymax);
        UIVertex(xmax, ymin);
        glEnd();

        if (focused) glColor4f(1.0f, 1.0f, 1.0f, linealpha);
        else glColor4f(0.8f, 0.8f, 0.8f, linealpha);

        glBegin(GL_LINE_LOOP);

        if (focused)
            glColor4f(1.0f, 1.0f, 1.0f, linealpha);
        else
            glColor4f(0.8f, 0.8f, 0.8f, linealpha);
        UIVertex(xmin + 1, ymin + 1);

        if (focused)
            glColor4f(1.0f, 1.0f, 1.0f, linealpha);
        else
            glColor4f(0.4f, 0.4f, 0.4f, linealpha);
        UIVertex(xmin + 1, ymax - 1);

        if (focused)
            glColor4f(1.0f, 1.0f, 1.0f, linealpha);
        else
            glColor4f(0.4f, 0.4f, 0.4f, linealpha);
        UIVertex(xmax - 1, ymax - 1);

        if (focused)
            glColor4f(1.0f, 1.0f, 1.0f, linealpha);
        else
            glColor4f(0.8f, 0.8f, 0.8f, linealpha);
        UIVertex(xmax - 1, ymin + 1);

        glEnd();
        glEnable(GL_TEXTURE_2D);
        TextRenderer::setFontColor(1.0f, 1.0f, 1.0f, 1.0f);
        if (!enabled) TextRenderer::setFontColor(0.6f, 0.6f, 0.6f, 1.0f);
        TextRenderer::renderString((xmin + xmax - TextRenderer::getStrWidth(text)) / 2, ymin, text);
    }

    void Textbox::update() {
        if (!enabled) {
            mouseon = false, focused = false, pressed = false;
            return;
        }
        static int delt = 0;
        static int ldel = 0;
        if (delt > INT_MAX - 2) delt = 0;
        if (ldel > INT_MAX - 2) delt = 0;
        //更新文本框状态
        if (parent->mx >= xmin && parent->mx <= xmax && parent->my >= ymin && parent->my <= ymax)
            mouseon = true, parent->MouseOnTextbox = true;
        else mouseon = false;

        if (((parent->mb == 1 && mouseon) || parent->enterp) && focused) pressed = true;
        else pressed = false;

        if (parent->mb == 1 && parent->mbl == 0 && mouseon) parent->focusid = id; //焦点在此
        if (parent->focusid == id) focused = true;
        else focused = false; //焦点
        if (focused && !inputstr.empty()) { text += inputstr; }
        delt++;
        if (parent->backspacep && (delt - ldel > 5) && text.length() >= 1) {
            ldel = delt;
            int n = text[text.length() - 1];
            if (n > 0 && n <= 127)
                text = text.substr(0, text.length() - 1);
            else
                text = text.substr(0, text.length() - 2);
        }
    }

    void Textbox::render() {
        float bcR = BgR;
        float bcG = BgG;
        float bcB = BgB;
        float bcA = BgA;
        if (!enabled) {
            bcR = BgR * 0.5f;
            bcG = BgG * 0.5f;
            bcB = BgB * 0.5f;
            bcA = BgA * 0.3f;
        }
        glColor4f(bcR, bcG, bcB, bcA);

        glDisable(GL_TEXTURE_2D);
        glBegin(GL_QUADS);
        UIVertex(xmin, ymin);
        UIVertex(xmin, ymax);
        UIVertex(xmax, ymax);
        UIVertex(xmax, ymin);
        glEnd();
        glColor4f(FgR * 0.9f, FgG * 0.9f, FgB * 0.9f, linealpha);

        if (!enabled) glColor4f(0.5f, 0.5f, 0.5f, linealpha);
        glLineWidth(linewidth);
        glBegin(GL_LINE_LOOP);
        UIVertex(xmin, ymin);
        UIVertex(xmin, ymax);
        UIVertex(xmax, ymax);
        UIVertex(xmax, ymin);
        glEnd();

        glBegin(GL_LINE_LOOP);

        if (focused)
            glColor4f(1.0f, 1.0f, 1.0f, linealpha);
        else
            glColor4f(0.8f, 0.8f, 0.8f, linealpha);
        UIVertex(xmin + 1, ymin + 1);

        if (focused)
            glColor4f(1.0f, 1.0f, 1.0f, linealpha);
        else
            glColor4f(0.4f, 0.4f, 0.4f, linealpha);
        UIVertex(xmin + 1, ymax - 1);

        if (focused)
            glColor4f(1.0f, 1.0f, 1.0f, linealpha);
        else
            glColor4f(0.4f, 0.4f, 0.4f, linealpha);
        UIVertex(xmax - 1, ymax - 1);

        if (focused)
            glColor4f(1.0f, 1.0f, 1.0f, linealpha);
        else
            glColor4f(0.8f, 0.8f, 0.8f, linealpha);
        UIVertex(xmax - 1, ymin + 1);

        glEnd();

        glEnable(GL_TEXTURE_2D);
        TextRenderer::setFontColor(1.0f, 1.0f, 1.0f, 1.0f);
        if (!enabled) TextRenderer::setFontColor(0.6f, 0.6f, 0.6f, 1.0f);
        TextRenderer::renderString(xmin, (ymin + ymax - 20) / 2, text);
    }

    void Vscroll::update() {
        if (!enabled) {
            mouseon = false, focused = false, pressed = false;
            return;
        }
        static double lstime;
        msup = false;
        msdown = false;
        psup = false;
        psdown = false;

        //更新滚动条状态
        //鼠标悬停
        mouseon = (parent->my >= ymin + barpos + 20 && parent->my <= ymin + barpos + barheight + 20 && parent->mx >=
            xmin && parent->mx <= xmax);
        if (parent->mx >= xmin && parent->mx <= xmax && parent->my >= ymin && parent->my <= ymax) {
            if (parent->mb == 1) parent->focusid = id;
            if (parent->my <= ymin + 20) {
                msup = true;
                if (parent->mb == 1 && parent->mbl == 0) barpos -= 10;
                if (parent->mb == 1) psup = true;
            }
            else if (parent->my >= ymax - 20) {
                msdown = true;
                if (parent->mb == 1 && parent->mbl == 0) barpos += 10;
                if (parent->mb == 1) psdown = true;
            }
            else if (timer() - lstime > 0.1 && parent->mb == 1) {
                lstime = timer();
                if (parent->my < ymin + barpos + 20) barpos -= 25;
                if (parent->my > ymin + barpos + barheight + 20) barpos += 25;
            }
        }
        if (parent->mb == 1 && mouseon && focused) {
            //鼠标按住
            pressed = true;
        }
        else { if (parent->mbl == 0) pressed = false; }

        if (parent->mb == 1 && parent->mbl == 0 && mouseon) parent->focusid = id; //焦点在此
        focused = (parent->focusid == id); //焦点
        if (pressed) barpos += parent->my - parent->myl; //拖动
        if (focused) {
            if (parent->upkp) barpos -= 1;
            if (parent->downkp) barpos += 1;
            if (parent->leftkp && !parent->leftkpl)barpos -= 1;
            if (parent->rightkp && !parent->rightkpl) barpos += 1;
        }
        if (defaultv)
            barpos += (parent->mwl - parent->mw) * 15;
        if (barpos < 0) barpos = 0; //让拖动条不越界
        if (barpos >= ymax - ymin - barheight - 40)
            barpos = ymax - ymin - barheight - 40;
    }

    void Vscroll::render() {
        float fcR = FgR;
        float fcG = FgG;
        float fcB = FgB;
        float fcA = FgA;
        float bcR = BgR;
        float bcG = BgG;
        float bcB = BgB;
        float bcA = BgA;
        if (mouseon) {
            fcR = FgR * 1.2f;
            fcG = FgG * 1.2f;
            fcB = FgB * 1.2f;
            fcA = FgA * 0.8f;
        }
        if (pressed) {
            fcR = FgR * 0.8f;
            fcG = FgG * 0.8f;
            fcB = FgB * 0.8f;
            fcA = FgA * 1.5f;
        }
        if (!enabled) {
            fcR = FgR * 0.5f;
            fcG = FgG * 0.5f;
            fcB = FgB * 0.5f;
            fcA = FgA * 0.3f;
        }

        glColor4f(bcR, bcG, bcB, bcA); //Track
        glDisable(GL_TEXTURE_2D);
        glBegin(GL_QUADS);
        UIVertex(xmin, ymin);
        UIVertex(xmax, ymin);
        UIVertex(xmax, ymax);
        UIVertex(xmin, ymax);
        glEnd();
        glDisable(GL_TEXTURE_2D); //Bar
        glColor4f(fcR, fcG, fcB, fcA);
        glBegin(GL_QUADS);
        UIVertex(xmin, ymin + barpos + 20);
        UIVertex(xmin, ymin + barpos + barheight + 20);
        UIVertex(xmax, ymin + barpos + barheight + 20);
        UIVertex(xmax, ymin + barpos + 20);
        glEnd();

        if (msup) {
            glColor4f(1.0f, 1.0f, 1.0f, 0.7f);
            if (psup) glColor4f(FgR, FgG, FgB, 0.9f);
            glBegin(GL_QUADS);
            UIVertex(xmin, ymin);
            UIVertex(xmin, ymin + 20);
            UIVertex(xmax, ymin + 20);
            UIVertex(xmax, ymin);
            glEnd();
        }
        if (msdown) {
            glColor4f(1.0f, 1.0f, 1.0f, 0.7f);
            if (psdown) glColor4f(FgR, FgG, FgB, 0.9f);
            glBegin(GL_QUADS);
            UIVertex(xmin, ymax - 20);
            UIVertex(xmin, ymax);
            UIVertex(xmax, ymax);
            UIVertex(xmax, ymax - 20);
            glEnd();
        }

        glColor4f(FgR * 0.9f, FgG * 0.9f, FgB * 0.9f, linealpha);
        if (!enabled) glColor4f(0.5f, 0.5f, 0.5f, linealpha);
        glBegin(GL_LINE_LOOP);
        UIVertex(xmin, ymin);
        UIVertex(xmin, ymax);
        UIVertex(xmax, ymax);
        UIVertex(xmax, ymin);
        glEnd();

        glBegin(GL_LINE_LOOP);

        if (focused)
            glColor4f(1.0f, 1.0f, 1.0f, linealpha);
        else
            glColor4f(0.8f, 0.8f, 0.8f, linealpha);
        UIVertex(xmin + 1, ymin + 1);

        if (focused)
            glColor4f(1.0f, 1.0f, 1.0f, linealpha);
        else
            glColor4f(0.4f, 0.4f, 0.4f, linealpha);
        UIVertex(xmin + 1, ymax - 1);

        if (focused)
            glColor4f(1.0f, 1.0f, 1.0f, linealpha);
        else
            glColor4f(0.4f, 0.4f, 0.4f, linealpha);
        UIVertex(xmax - 1, ymax - 1);

        if (focused)
            glColor4f(1.0f, 1.0f, 1.0f, linealpha);
        else
            glColor4f(0.8f, 0.8f, 0.8f, linealpha);
        UIVertex(xmax - 1, ymin + 1);

        glEnd();

        glLineWidth(3.0);
        glBegin(GL_LINES);
        glColor4f(FgR, FgG, FgB, 1.0);
        if (psup) glColor4f(1.0f - FgR, 1.0f - FgG, 1.0f - FgB, 1.0f);
        UIVertex((xmin + xmax) / 2, ymin + 8);
        UIVertex((xmin + xmax) / 2 - 4, ymin + 12);
        UIVertex((xmin + xmax) / 2, ymin + 8);
        UIVertex((xmin + xmax) / 2 + 4, ymin + 12);
        glColor4f(FgR, FgG, FgB, 1.0);
        if (psdown) glColor4f(1.0f - FgR, 1.0f - FgG, 1.0f - FgB, 1.0f);
        UIVertex((xmin + xmax) / 2, ymax - 8);
        UIVertex((xmin + xmax) / 2 - 4, ymax - 12);
        UIVertex((xmin + xmax) / 2, ymax - 8);
        UIVertex((xmin + xmax) / 2 + 4, ymax - 12);
        glEnd();
    }

    void Imagebox::update() { }

    void Imagebox::render() {
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, imageid);
        glBegin(GL_QUADS);
        glTexCoord2f(txmin, tymax);
        UIVertex(xmin, ymin);
        glTexCoord2f(txmin, tymin);
        UIVertex(xmin, ymax);
        glTexCoord2f(txmax, tymin);
        UIVertex(xmax, ymax);
        glTexCoord2f(txmax, tymax);
        UIVertex(xmax, ymin);
        glEnd();
    }

    void Form::Init() {
        maxid = 0;
        currentid = 0;
        focusid = -1;
        //Transition forward
        if (transitionList != 0) glDeleteLists(transitionList, 1);
        transitionList = lastdisplaylist;
        transitionForward = true;
        transitionTimer = timer();
    }

    void Form::registerControl(Controls* c) {
        c->id = currentid;
        c->parent = this;
        children.push_back(c);
        currentid++;
        maxid++;
    }

    void Form::registerControls(std::initializer_list<Controls*> controls) {
        for (auto x : controls)
            registerControl(x);
    }

    void Form::update() {
        updated = false;
        bool lMouseOnTextbox = MouseOnTextbox;
        MouseOnTextbox = false;

        if (glfwGetKey(MainWindow, GLFW_KEY_TAB) == GLFW_PRESS) {
            //TAB键切换焦点
            if (glfwGetKey(MainWindow, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS || glfwGetKey(
                MainWindow, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS) {
                //Shift+Tab
                updated = true;
                if (!tabp) focusid--;
                if (focusid == -2) focusid = maxid - 1; //到了最前一个ID
            }
            else {
                updated = true;
                if (!tabp) focusid++;
                if (focusid == maxid + 1) focusid = -1; //到了最后一个ID
            }
            tabp = true;
        }
        if (glfwGetKey(MainWindow, GLFW_KEY_TAB) != GLFW_PRESS) tabp = false;
        if (!(glfwGetKey(MainWindow, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS || glfwGetKey(MainWindow, GLFW_KEY_RIGHT_SHIFT)
            == GLFW_PRESS))
            shiftp = false;

        enterpl = enterp;
        if (glfwGetKey(MainWindow, GLFW_KEY_ENTER) == GLFW_PRESS) {
            updated = true;
            enterp = true;
        }
        if (!(glfwGetKey(MainWindow, GLFW_KEY_ENTER) == GLFW_PRESS)) enterp = false;

        upkpl = upkp; //方向键上
        if (glfwGetKey(MainWindow, GLFW_KEY_UP) == GLFW_PRESS) {
            updated = true;
            upkp = true;
        }
        if (!(glfwGetKey(MainWindow, GLFW_KEY_UP) == GLFW_PRESS)) upkp = false;

        downkpl = downkp; //方向键下
        if (glfwGetKey(MainWindow, GLFW_KEY_DOWN) == GLFW_PRESS) { downkp = true; }
        if (!(glfwGetKey(MainWindow, GLFW_KEY_DOWN) == GLFW_PRESS)) downkp = false;

        leftkpl = leftkp; //方向键左
        if (glfwGetKey(MainWindow, GLFW_KEY_LEFT) == GLFW_PRESS) { leftkp = true; }
        if (!(glfwGetKey(MainWindow, GLFW_KEY_LEFT) == GLFW_PRESS)) leftkp = false;
        rightkpl = rightkp; //方向键右
        if (glfwGetKey(MainWindow, GLFW_KEY_RIGHT) == GLFW_PRESS) { rightkp = true; }
        if (glfwGetKey(MainWindow, GLFW_KEY_RIGHT) != GLFW_PRESS) rightkp = false;

        backspacepl = backspacep;
        if (glfwGetKey(MainWindow, GLFW_KEY_BACKSPACE) == GLFW_PRESS)
            backspacep = true;
        else
            backspacep = false;

        if (mb == 1 && mbl == 0) focusid = -1; //空点击时使焦点清空

        for (size_t i = 0; i != children.size(); i++) {
            children[i]->updatepos();
            children[i]->update(); //更新子控件
        }

        if (!lMouseOnTextbox && MouseOnTextbox) {
            glfwDestroyCursor(MouseCursor);
            MouseCursor = glfwCreateStandardCursor(GLFW_IBEAM_CURSOR);
            glfwSetCursor(MainWindow, MouseCursor);
        }
        if (lMouseOnTextbox && !MouseOnTextbox) {
            glfwDestroyCursor(MouseCursor);
            MouseCursor = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
            glfwSetCursor(MainWindow, MouseCursor);
        }
        onUpdate();

        getAudioSystem().update();
    }

    void Form::render() {
        if (Background) Background();

        double TimeDelta = timer() - transitionTimer;
        float transitionAnim = (float)(1.0 - pow(0.8, TimeDelta * 60.0) + pow(0.8, 0.3 * 60.0) / 0.3 * TimeDelta);

        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glOrtho(0, windowwidth, windowheight, 0, -1.0, 1.0);
        glMatrixMode(GL_MODELVIEW);
        glDepthFunc(GL_ALWAYS);
        glLoadIdentity();
        if (GUIScreenBlur) screenBlur();
        glDisable(GL_TEXTURE_2D);
        glDisable(GL_LINE_SMOOTH);

        if (TimeDelta <= 0.3 && transitionList != 0) {
            if (transitionForward) glTranslatef(-transitionAnim * windowwidth, 0.0f, 0.0f);
            else glTranslatef(transitionAnim * windowwidth, 0.0f, 0.0f);
            glCallList(transitionList);
            glLoadIdentity();
            if (transitionForward) glTranslatef(windowwidth - transitionAnim * windowwidth, 0.0f, 0.0f);
            else glTranslatef(transitionAnim * windowwidth - windowwidth, 0.0f, 0.0f);
        }
        else if (transitionList != 0) {
            glDeleteLists(transitionList, 1);
            transitionList = 0;
        }

        if (displaylist == 0) displaylist = glGenLists(1);
        glNewList(displaylist, GL_COMPILE_AND_EXECUTE);
        for (size_t i = 0; i != children.size(); i++) { children[i]->render(); }
        onRender();
        glEndList();
        lastdisplaylist = displaylist;
    }

    Label::Label(const std::string& t, int xi_r, int xa_r, int yi_r, int ya_r, double xi_b, double xa_b, double yi_b,
                 double ya_b)
        : Label() {
        text = t;
        resize(xi_r, xa_r, yi_r, ya_r, xi_b, xa_b, yi_b, ya_b);
    }

    Button::Button(const std::string& t, int xi_r, int xa_r, int yi_r, int ya_r, double xi_b, double xa_b, double yi_b,
                   double ya_b)
        : Button() {
        text = t;
        enabled = true;
        resize(xi_r, xa_r, yi_r, ya_r, xi_b, xa_b, yi_b, ya_b);
    }

    Trackbar::Trackbar(const std::string& t, int w, int s, int xi_r, int xa_r, int yi_r, int ya_r, double xi_b,
                       double xa_b, double yi_b, double ya_b)
        : Trackbar() {
        text = t;
        enabled = true;
        barwidth = w;
        barpos = s;
        resize(xi_r, xa_r, yi_r, ya_r, xi_b, xa_b, yi_b, ya_b);
    }

    Textbox::Textbox(const std::string& t, int xi_r, int xa_r, int yi_r, int ya_r, double xi_b, double xa_b,
                     double yi_b, double ya_b)
        : Textbox() {
        text = t;
        enabled = true;
        resize(xi_r, xa_r, yi_r, ya_r, xi_b, xa_b, yi_b, ya_b);
    }

    Vscroll::Vscroll(int h, int s, int xi_r, int xa_r, int yi_r, int ya_r, double xi_b, double xa_b, double yi_b,
                     double ya_b)
        : Vscroll() {
        enabled = true;
        barheight = h;
        barpos = s;
        resize(xi_r, xa_r, yi_r, ya_r, xi_b, xa_b, yi_b, ya_b);
    }

    Imagebox::Imagebox(float _txmin, float _txmax, float _tymin, float _tymax, TextureID iid, int xi_r, int xa_r,
                       int yi_r, int ya_r, double xi_b, double xa_b, double yi_b, double ya_b)
        : Imagebox() {
        txmin = _txmin;
        txmax = _txmax;
        tymin = _tymin;
        tymax = _tymax;
        imageid = iid;
        resize(xi_r, xa_r, yi_r, ya_r, xi_b, xa_b, yi_b, ya_b);
    }

    void Form::cleanup() {
        //Transition backward
        if (transitionList != 0) glDeleteLists(transitionList, 1);
        transitionList = displaylist;
        transitionForward = false;
        transitionTimer = timer();
        for (size_t i = 0; i != children.size(); i++) { children[i]->destroy(); }
    }

    Controls* Form::getControlByID(int cid) {
        for (size_t i = 0; i != children.size(); i++) { if (children[i]->id == cid) return children[i]; }
        return nullptr;
    }

    Form::Form() {
        Init();
        Background = &drawBackground;
    }

    void Form::singleloop() {
        double dmx, dmy;
        mxl = mx;
        myl = my;
        mwl = mw;
        mbl = mb;
        mb = getMouseButton();
        mw = getMouseScroll();
        glfwGetCursorPos(MainWindow, &dmx, &dmy);
        mx = static_cast<int>(dmx / stretch), my = static_cast<int>(dmy / stretch);
        update();
        render();
        glFinish();
        glfwSwapBuffers(MainWindow);
        glfwPollEvents();
    }

    struct PageOpRq {
        int Op; //1 is push ,2 is pop ,3 is back to main;
        Form* Page;
    };

    std::deque<Form*> ViewStack;
    std::deque<PageOpRq> ViewOps = {};

    void pushPage(Form* View) {
        ViewOps.push_back({1, View});
    }

    void popPage() {
        ViewOps.push_back({2, nullptr});
    }

    void BackToMain() {
        ViewOps.push_back({3, nullptr});
    }

    void PopView() {
        ViewStack.front()->onLeave();
        delete ViewStack[0];
        ViewStack.pop_front();
    }

    void ClearStack() {
        ViewOps.push_back({4, nullptr});
    }

    void ProcessRequests() {
        //Process the op deque
        if (!ViewOps.empty()) {
            for (auto& i : ViewOps) {
                switch (i.Op) {
                case 1:
                    ViewStack.push_front(i.Page);
                    ViewStack.front()->onLoad();
                    break;
                case 2:
                    PopView();
                    break;
                case 3:
                    while (!ViewStack.empty()) PopView();
                    ViewStack.push_front(GetMain());
                    ViewStack.front()->onLoad();
                    break;
                case 4:
                    while (!ViewStack.empty()) PopView();
                    break;
                default:;
                }
            }
            ViewOps.clear();
        }
    }

    void appStart() {
        glfwSetInputMode(MainWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        glClearColor(0.0, 0.0, 0.0, 1.0);
        glDisable(GL_CULL_FACE);
        ProcessRequests();
        TextRenderer::setFontColor(1.0, 1.0, 1.0, 1.0);
        while (!ViewStack.empty()) {
            (*ViewStack.begin())->singleloop();
            ProcessRequests();
            if (glfwWindowShouldClose(MainWindow)) { while (!ViewStack.empty()) PopView(); }
        }
        AppCleanUp();
    }

    Form::~Form() { cleanup(); }
}
