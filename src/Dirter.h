#ifndef DIRTER_H
#define DIRTER_H

#include "Bang/Bang.h"
#include "BangEditor/BangEditor.h"

using namespace Bang;

class Dirter
{
public:
    Dirter() = delete;
    ~Dirter() = delete;

    static void AddDirt(GameObject *modelGameObject);
};

#endif  // DIRTER_H
