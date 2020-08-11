
#include <stdio.h>

class Render{
    public:
        Render();
        ~Render();
        void draw();

    private:
        void initGL();
        void exitGL();

};