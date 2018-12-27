#include "helix.h"
#include <math.h>

void helix_pre_calculate ( helix_plan *helix )
{
    switch (helix->plane)
    {
        case XY:
            helix->pld[0] = helix->x[0];
            helix->pld[1] = helix->x[1];
            helix->height = helix->x[2];
            break;
        case YZ:
            helix->pld[0] = helix->x[1];
            helix->pld[1] = helix->x[2];
            helix->height = helix->x[0];
            break;
        case ZX:
            helix->pld[0] = helix->x[2];
            helix->pld[1] = helix->x[0];
            helix->height = helix->x[1];
            break;
    }
    fixed pldl = fsqrt(SQR(helix->pld[0]) + SQR(helix->pld[1]));
    fixed p[2] = { DIV(helix->pld[1], pldl), -DIV(helix->pld[0], pldl) };
    helix->center[0] = helix->pld[0]/2 + MUL(p[0], helix->d);
    helix->center[1] = helix->pld[1]/2 + MUL(p[1], helix->d);
    helix->radius = fsqrt(SQR(pldl)/4 + SQR(helix->d));
    if (helix->cw)
    {
        if (helix->d > 0)
        {
            helix->big_arc = 0;
        }
        else
        {
            helix->big_arc = 1;
        }
    }
    else
    {
        if (helix->d > 0)
        {
            helix->big_arc = 1;
        }
        else
        {
            helix->big_arc = 0;
        }
    }
    
}
