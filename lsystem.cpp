#include <SDL2/SDL.h>
#include <vector>
#include <unordered_map>
#include <iostream>
#include <math.h>

using namespace std;

struct SDL_2D_Camera {
    SDL_Window *window = NULL;
    SDL_Renderer *renderer = NULL;
    int w = 800;
    int h = 600;
    float aspectRatio = 800.0/600.0;

    float Cx = 0;
    float Cy = -150;
    float zoom = 400.0;
    float dt = 0.005;
    float turbo = 3;

    bool Init(int width, int height) {
        w = width;
        h = height;

        window = SDL_CreateWindow(
            "Window", 
            SDL_WINDOWPOS_UNDEFINED, 
            SDL_WINDOWPOS_UNDEFINED, 
            w, 
            h,
            0
        );
        if(!window) {
            cout << "Window creation failure" << endl;
            return false;
        }
        renderer = SDL_CreateRenderer(
            window,
            -1,
            SDL_RENDERER_ACCELERATED
        );
        if (!renderer) {
            cout << "Rederer creation failure" << endl;
            return false;
        }
        return true;
    }

    void processInput() {
        const Uint8* keyboard = SDL_GetKeyboardState(NULL);
        float turboMult = (1 + (turbo - 1) * keyboard[SDL_SCANCODE_LSHIFT]);
        Cx += turboMult*dt*zoom*(keyboard[SDL_SCANCODE_D] - keyboard[SDL_SCANCODE_A]);    
        Cy += turboMult*dt*zoom*(keyboard[SDL_SCANCODE_S] - keyboard[SDL_SCANCODE_W]);
        zoom *= 1 - turboMult*dt*(keyboard[SDL_SCANCODE_EQUALS] - keyboard[SDL_SCANCODE_MINUS]);
    }
    void clear() {
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
    }
    void present() {
        SDL_RenderPresent(renderer);
    }

    void ScreenToPlane(int Sx, int Sy, float& Px, float& Py) {
        Px = Cx + zoom * (Sx - w/2.0) / (float) h;
        Py = Cy + zoom * (Sy - h/2.0) / (float) h;
    }

    void PlaneToScreen(float Px, float Py, int& Sx, int& Sy) {
        Sx = h*(Px - Cx)/(zoom) + w/2.0;
        Sy = h*(Py - Cy)/(zoom) + h/2.0;
    }

    void DrawLine(float psx, float psy, float pex, float pey) {
        int ssx, ssy, sex, sey;
        PlaneToScreen(psx, psy, ssx, ssy);
        PlaneToScreen(pex, pey, sex, sey);
        SDL_RenderDrawLine(renderer, ssx, ssy, sex, sey);

    }

};


struct Lsystem {
    string seed;
    string state;
    unordered_map<char, string> ruleset;

    Lsystem(string seed, unordered_map<char, string> ruleset) {
        this->seed = seed;
        this->state = seed;
        this->ruleset = ruleset;
    }
    void step() {
        string buffer = "";
        for (size_t i = 0; i < state.length(); i++)
        {
            buffer += ruleset.at(state[i]);
        }
        state = buffer;
    }
    void iter(int numStep) {
        state = seed;
        for (int i = 0; i < numStep; i++)
        {
            this->step();
        }
    }
};


string seed = "X";
unordered_map<char, string> ruleset = {
    {'A', "AA"}, 
    {'B', "B"},
    {'X', "A+[[X]-X]-A[-AX]+X"},
    {'-', "-"},
    {'+', "+"},
    {'[', "["},
    {']', "]"}
};
float theta = M_PI*(25.0/180.0);
void interpeter(string state, SDL_2D_Camera C) {
    struct Position
    {
        float x = 0;
        float y = 0;
        int dir = 0;
        void step() {
            x += sin(theta*dir);
            y += -cos(theta*dir);
        }
    };
    Position pos, nextpos;
    vector<Position> posStack;
    
    for (size_t i = 0; i < state.size(); i++)
    {
        switch (state.at(i))
        {
        case 'A':
        case 'B':
        case 'F':
        case 'G':
            nextpos.step();
            C.DrawLine(pos.x, pos.y, nextpos.x, nextpos.y);
            break;
        case '+':
            nextpos.dir++;
            break;
        
        case '-':
            nextpos.dir--;
            break;
        
        case '[':
            posStack.push_back(nextpos);
            break;

        case ']':
            nextpos = posStack.at(posStack.size() - 1);
            posStack.pop_back();
            break;

        default:
            break;
        }
        pos = nextpos;
    }
    
}


int main(int argc, char **argv)
{
    SDL_Init(SDL_INIT_VIDEO);
    SDL_2D_Camera C;
    C.Init(800,600);

    Lsystem L (seed, ruleset);
    L.iter(7);
    cout << L.state << endl;

    bool quit = SDL_FALSE;
    SDL_Event e;

    while (!quit) 
    {
        while (SDL_PollEvent(&e)) {
            switch(e.type) {
            case SDL_QUIT:
                quit = SDL_TRUE;    
                break;
            case SDL_KEYDOWN:
                if(e.key.keysym.scancode == SDL_SCANCODE_ESCAPE) {
                    quit = SDL_TRUE;
                }
                if (e.key.keysym.scancode == SDL_SCANCODE_R)
                {
                    theta += M_PI*(5.0/180.0);
                }
                if (e.key.keysym.scancode == SDL_SCANCODE_F)
                {
                    theta -= M_PI*(5.0/180.0);
                }

                break;
            }
        }
        C.processInput();

        C.clear();

        SDL_SetRenderDrawColor(C.renderer, 255, 255, 255, 255);
        interpeter(L.state, C);

        C.present();
    }

    SDL_Quit();
    return 0;
}
